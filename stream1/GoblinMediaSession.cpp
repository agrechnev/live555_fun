//
// Created by Oleksiy Grechnyev on 8/12/18.
//

#include <iostream>

#include <H264VideoStreamDiscreteFramer.hh>
#include <H264VideoRTPSink.hh>

#include "./LiveSourceWithx264.h"

#include "GoblinMediaSession.h"
//==================================================================================
const char *GoblinMediaSession::getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource) {
    using namespace std;

    cout << "getAuxSDPLine()" << endl;
    if (nullptr != auxSDPLines) {
        cout << "auxSDPLines =" << endl;
        cout << auxSDPLines << endl;
        return auxSDPLines;
    }

    if (nullptr == sink) {
        sink = rtpSink;
        sink->startPlaying(*inputSource, afterPlayingCB, this);
        checkLine(this);
    }

    envir().taskScheduler().doEventLoop(&doneFlag);
    return auxSDPLines;
}
//==================================================================================

void GoblinMediaSession::afterPlayingCB1() {
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    setDoneFlag();
}
//==================================================================================

void GoblinMediaSession::checkLine1() {
    if (auxSDPLines != nullptr) {
        setDoneFlag();
        return;
    }

    // Otherwise we create something
    char const *dasl;
    if (sink != nullptr && (dasl = sink->auxSDPLine()) != nullptr){
        auxSDPLines = strDup(dasl);
        sink = nullptr;
        setDoneFlag();
        return;
    }

    // We wait
    int uSecsDelay =  100000;
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsDelay, (TaskFunc *) checkLine, this);
}
//==================================================================================

FramedSource *GoblinMediaSession::createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate) {
    LiveSourceWithx264 * source = LiveSourceWithx264::createNew(envir());
    estBitrate = source->getEstimatedBitrate();
    return H264VideoStreamDiscreteFramer::createNew(envir(), source);
}
//==================================================================================

RTPSink *GoblinMediaSession::createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                              FramedSource *inputSource) {
   OutPacketBuffer::maxSize = 100000;
   return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
//==================================================================================
