#pragma once
#include <OnDemandServerMediaSubsession.hh>
#include <H264VideoStreamDiscreteFramer.hh>
#include <Groupsock.hh>
#include "LiveSourceWithx264.h"


class H264LiveServerMediaSession : public OnDemandServerMediaSubsession {

public:
    static H264LiveServerMediaSession *createNew(UsageEnvironment &env, bool reuseFirstSource);

    void checkForAuxSDPLine1();

    void afterPlayingDummy1();

protected:
    H264LiveServerMediaSession(UsageEnvironment &env, bool reuseFirstSource);

    ~H264LiveServerMediaSession() override;

    void setDoneFlag();

protected:
    char const *getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource) override;

    FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate) override;

    RTPSink *createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                              FramedSource *inputSource) override;

private:
    char *auxSDPLines;
    char doneFlag;
    RTPSink *sink;
};
