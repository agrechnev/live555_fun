//
// Created by Oleksiy Grechnyev on 8/12/18.
//

#pragma once

#include <OnDemandServerMediaSubsession.hh>


class GoblinMediaSession : public OnDemandServerMediaSubsession {
public: //=========== Methods
    /// Static Factory method (wait, this is not java !)
    static GoblinMediaSession *createNew(UsageEnvironment &env) {
        return new GoblinMediaSession(env);
    }

private: //=========== Methods

    void setDoneFlag() {
        doneFlag = 1;
    }

    static void afterPlayingCB(void *data){
        ((GoblinMediaSession *)data)->afterPlayingCB1();
    }

    static void checkLine(void *data){
        ((GoblinMediaSession *)data)->checkLine1();
    }

    void afterPlayingCB1();

    void checkLine1();

protected: //=========== Overriden Methods

    // Ctor
    GoblinMediaSession(UsageEnvironment &env) :
            OnDemandServerMediaSubsession(env, true) {}

    ~GoblinMediaSession() override {
        delete[] auxSDPLines;
    }

    const char *getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource) override;

    FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate) override;

    RTPSink *createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                              FramedSource *inputSource) override;

private: //=========== Fields

    char *auxSDPLines = nullptr;
    char doneFlag = 0;
    RTPSink *sink = nullptr;
};


