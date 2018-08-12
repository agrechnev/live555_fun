#include <iostream>
#include <string>
#include <memory>

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>

#include "./GoblinMediaSession.h"

using namespace std;

static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms,
                           const string & streamName, const string & inputFileName) {
    char *url = rtspServer->rtspURL(sms);
//    UsageEnvironment &env = rtspServer->envir();
    cerr << "Stream " << streamName << " from file " << inputFileName << "\n";
    cerr << "URL : " << url << "\n";
    delete[] url;
}

int main() {
    // Set up the environment
    TaskScheduler * scheduler =  BasicTaskScheduler::createNew();
    UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);

    // Create the server
    UserAuthenticationDatabase *authDB = nullptr;
    RTSPServer *rtspServer = RTSPServer::createNew(*env, 8554, authDB);

    // Create the Media session
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, "goblin", "goblin_info", "Goblin Session");
    GoblinMediaSession *liveSS = GoblinMediaSession::createNew(*env);

//        sms->addSubsession(MP3AudioFileServerMediaSubsession::createNew(*env, "/home/seymour/Music/Jump.mp3", true, false, nullptr));
    sms->addSubsession(liveSS);

    rtspServer->addServerMediaSession(sms);
    announceStream(rtspServer, sms, "goblin", "no-file");

    // Run the file
    env->taskScheduler().doEventLoop();

    return 0;
}