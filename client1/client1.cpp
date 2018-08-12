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

    env << "cbSSAfterPlay(\n)";

    // TODO shutdown session
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

    // Create the sink
    ss->sink = ElfSink::createNew(env, *ss, rtspClient->url());
    if (!ss->sink)
        throw runtime_error("cbAfterSetup() : Cannot create sink !");
    // Save client ptr in subsession
    ss->miscPtr = rtspClient;

    // Set up the next subsession, if any:
    setupNextSubsession(rtspClient);

    // Start playing this sink
    ss->sink->startPlaying(*(ss->readSource()), cbSSAfterPlay, ss);

    // TODO: bye handler
    delete[] resultString;

}
//==================================================================================
// Callback after "PLAY"
void cbAfterPlay(RTSPClient* rtspClient, int resultCode, char* resultString){
    UsageEnvironment & env = rtspClient->envir();
    ElfClientState & scs = ((ElfRTSPClient *) rtspClient)->scs;

    env << "cbAfterPlay() \n";
    if (resultCode)
        throw runtime_error("cbAfterPlay() : FAIL ! resultCode = " + to_string(resultCode));

    // TODO optional timer

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
        rtspClient->sendSetupCommand(*ss, cbAfterSetup, False, False);
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

    if (resultCode != 0)
        throw runtime_error("cbAfterDescribe() : resultCode = " + to_string(resultCode));

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
