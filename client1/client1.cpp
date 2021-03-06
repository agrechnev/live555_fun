//
// Created by Oleksiy Grechnyev on 8/12/18.
//

#include <iostream>

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>

#include "./ElfClientState.h"
#include "./ElfRTSPClient.h"
#include "./ElfSink.h"

using namespace std;


//==================================================================================
// Subsession to string
string mss2str(const MediaSubsession & m){
    return string(m.mediumName()) + "/" + m.codecName();
}

//==================================================================================
// Misc callbacks
//==================================================================================
void cbSSAfterPlay(void * data) {
    MediaSubsession * ss = (MediaSubsession *) data;
    RTSPClient * rtspClient= (RTSPClient *) (ss->miscPtr);
    UsageEnvironment & env = rtspClient->envir();

    env << "cbSSAfterPlay()\n";

    // Shutdown session
    // sink first
    Medium::close(ss->sink);
    ss->sink = nullptr;

    // Check if all subsessions are closed
    MediaSession & session = ss->parentSession();
    MediaSubsessionIterator iter(session);
    while ((ss = iter.next()) != nullptr) {
        if (ss->sink)
            return;
    }

    env << "cbSSAfterPlay() : SHUTOWN \n";
    // TODO : shut down stream
}
//==================================================================================
// Main callbacks
//==================================================================================
void setupNextSubsession(RTSPClient* rtspClient);
//==================================================================================
// Callback after "SETUP"
void cbAfterSetup(RTSPClient* rtspClient, int resultCode, char* resultString){
    UsageEnvironment & env = rtspClient->envir();
    ElfClientState & scs = ((ElfRTSPClient *) rtspClient)->scs;

    MediaSubsession * ss = scs.subsession;
    env << "cbAfterSetup() " << mss2str(*ss).c_str() << "\n";

    if (resultCode)
        throw runtime_error("cbAfterSetup() : FAIL ! resultCode = " + to_string(resultCode));

    delete[] resultString;

    if (string(ss->mediumName()) == "video") {
        scs.subsessionVideo = ss;

        // Get the resolution one way or another
        int frWidth = ss->videoWidth();
        int frHeight = ss->videoHeight();
        if (frWidth * frHeight == 0) {
            frWidth = scs.frameWidth;
            frHeight = scs.frameHeight;
        }
        if (frWidth * frHeight == 0)
            throw runtime_error("cbAfterSetup() : Cannot determine video resolution !");

        env << "cbAfterSetup() : video : RESOLUTION = " << frWidth << "x" << frHeight << "\n";

        // Create the video sink
        ss->sink = ElfSink::createNew(env, *ss, frWidth, frHeight, rtspClient->url());
        if (!ss->sink)
            throw runtime_error("cbAfterSetup() : Cannot create video sink !");
        // Save client ptr in subsession
        ss->miscPtr = rtspClient;


        // Start playing this sink
        ss->sink->startPlaying(*(ss->readSource()), cbSSAfterPlay, ss);
    } else if (string(ss->mediumName()) == "audio") {
        // Create the audio sink if needed
        env << "cbAfterSetup() : audio \n ";
        ss->sink = nullptr;
    }

    // TODO: bye handler


    // Set up the next subsession, if any:
    setupNextSubsession(rtspClient);
}
//==================================================================================
// Callback after "PLAY"
void cbAfterPlay(RTSPClient* rtspClient, int resultCode, char* resultString){
    UsageEnvironment & env = rtspClient->envir();
    ElfClientState & scs = ((ElfRTSPClient *) rtspClient)->scs;

    env << "cbAfterPlay() \n";
    if (resultCode)
        throw runtime_error("cbAfterPlay() : FAIL ! resultCode = " + to_string(resultCode));

    delete[] resultString;
}
//==================================================================================
void setupNextSubsession(RTSPClient* rtspClient) {
    UsageEnvironment & env = rtspClient->envir();
    ElfClientState & scs = ((ElfRTSPClient *) rtspClient)->scs;

    scs.subsession = scs.iter->next();
    MediaSubsession * ss = scs.subsession;
    if (ss) {
        if (!ss->initiate()) {
            env << "setupNextSubsession() : failed to initiate subsession " << mss2str(*ss).c_str() << "\n";
            setupNextSubsession(rtspClient); // Try again
        } else {
            env << "setupNextSubsession() : initiated subsession " << mss2str(*ss).c_str() << "\n";
            if (ss->rtcpIsMuxed()) {
                env << "setupNextSubsession() : client port = " << ss->clientPortNum() << "\n";
            } else {
                int pn = ss->clientPortNum();
                env << "setupNextSubsession() : client port = " << pn << "-" << pn+1 << "\n";
            }
        }
        // "SETUP"
        rtspClient->sendSetupCommand(*ss, cbAfterSetup, False, True);
        return;
    }

    // We finished setting up all sessions, now "PLAY"
    if (scs.session->absStartTime()) {
        // We are using ABS time
        env << "setupNextSubsession() : ABS time ! \n";
        rtspClient->sendPlayCommand(*scs.session, cbAfterPlay, scs.session->absStartTime(), scs.session->absEndTime());
    } else {
        scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
        env << "setupNextSubsession() : NO ABS time ! duration = " << scs.duration << "\n";
        rtspClient->sendPlayCommand(*scs.session, cbAfterPlay);
    }
}
//==================================================================================
// Callback after describe
void cbAfterDescribe(RTSPClient* rtspClient, int resultCode, char* resultString) {
    UsageEnvironment & env = rtspClient->envir();
    ElfClientState & scs = ((ElfRTSPClient *) rtspClient)->scs;
    scs.sdpLines = resultString; // Save this string
    scs.parseFrameResolution(); // Get width , height from SDP lines

    if (resultCode != 0)
        throw runtime_error("cbAfterDescribe() : resultCode = " + to_string(resultCode));


    env << "cbAfterDescribe() : RESOLUTION = " << scs.frameWidth << "x" << scs.frameHeight << "\n";
    env << "cbAfterDescribe() [" << rtspClient->url() << "] resultString = \n" << resultString << "\n";

    // Create session out of SDP strings
    scs.session = MediaSession::createNew(env, resultString);
    delete [] resultString;
    if (!scs.session)
        throw runtime_error("cbAfterDescribe() : Cannot create session !");
    else if (!scs.session->hasSubsessions())
        throw runtime_error("cbAfterDescribe() : Session has no subsessions !");

    scs.iter = new MediaSubsessionIterator(*scs.session);

    setupNextSubsession(rtspClient);
}
//==================================================================================
// Open a URL
void openURL(UsageEnvironment & env, const string & url) {
    // Create an RTSP client
    cout << "Opening stream : " << url << endl;
    RTSPClient * rtspClient = ElfRTSPClient::createNew(env, url.c_str(), 1, "client1");
    if (nullptr == rtspClient)
        throw runtime_error("openURL() : Cannot open RTSPClient !");

    rtspClient->sendDescribeCommand(cbAfterDescribe);
}
//==================================================================================

int main(int argc, char ** argv) {
    char eventFlag = 0;

    cout << "client1" << endl;
    if (argc < 2) {
        cout << "Usage: client1 <streamURL>" << endl;
        return 1;
    }

    // Start live555
    TaskScheduler * scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment * env = BasicUsageEnvironment::createNew(*scheduler);

    // Open the stream
    openURL(*env, argv[1]);

    // Event loop
    scheduler->doEventLoop(&eventFlag);
}
//==================================================================================
