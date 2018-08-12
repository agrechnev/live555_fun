#include <H264VideoRTPSink.hh>
#include "H264LiveServerMediaSession.h"

H264LiveServerMediaSession *H264LiveServerMediaSession::createNew(UsageEnvironment &env, bool reuseFirstSource) {
    return new H264LiveServerMediaSession(env, reuseFirstSource);
}

H264LiveServerMediaSession::H264LiveServerMediaSession(UsageEnvironment &env, bool reuseFirstSource)
        : OnDemandServerMediaSubsession(env, reuseFirstSource),
          auxSDPLines(nullptr),
          doneFlag(0),
          sink(nullptr) {
}

H264LiveServerMediaSession::~H264LiveServerMediaSession() {
    delete[] auxSDPLines;
}

static void afterPlayingDummy(void *clientData) {
    auto *session = (H264LiveServerMediaSession *) clientData;
    session->afterPlayingDummy1();
}

void H264LiveServerMediaSession::afterPlayingDummy1() {
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    setDoneFlag();
}

static void checkForAuxSDPLine(void *clientData) {
    auto *session = (H264LiveServerMediaSession *) clientData;
    session->checkForAuxSDPLine1();
}

void H264LiveServerMediaSession::checkForAuxSDPLine1() {
    if (auxSDPLines != nullptr) {
        setDoneFlag();
        return;
    }

    char const *dasl;
    if (sink != nullptr && (dasl = sink->auxSDPLine()) != nullptr) {
        auxSDPLines = strDup(dasl);
        sink = nullptr;
        setDoneFlag();
        return;
    }

    int uSecsDelay = 100000;
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsDelay, (TaskFunc *) checkForAuxSDPLine, this);
}

char const *H264LiveServerMediaSession::getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource) {
    if (auxSDPLines != nullptr) {
        return auxSDPLines;
    }

    if (sink == nullptr) {
        sink = rtpSink;
        sink->startPlaying(*inputSource, afterPlayingDummy, this);
        checkForAuxSDPLine(this);
    }

    envir().taskScheduler().doEventLoop(&doneFlag);
    return auxSDPLines;
}

FramedSource *H264LiveServerMediaSession::createNewStreamSource(unsigned clientSessionID, unsigned &estBitRate) {
    LiveSourceWithx264 *source = LiveSourceWithx264::createNew(envir());
    estBitRate = source->getEstimatedBitrate();
    return H264VideoStreamDiscreteFramer::createNew(envir(), source);
}

RTPSink *H264LiveServerMediaSession::createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                                      FramedSource *inputSource) {
    OutPacketBuffer::maxSize = 100000;
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

void H264LiveServerMediaSession::setDoneFlag() {
    doneFlag = 1;
}
