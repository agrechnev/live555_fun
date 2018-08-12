//
// Created by Oleksiy Grechnyev on 8/12/18.
//

#pragma once


#include <liveMedia.hh>

#include "./VDecoder.h"

class ElfSink : public MediaSink {
public: //================== Parameter
    static constexpr int BUFFER_SIZE = 100000;
public: //================== Methods

    static ElfSink *createNew(UsageEnvironment &env, MediaSubsession &subsession,
                              char const *streamId = nullptr) {
        return new ElfSink(env, subsession, streamId);
    }

protected: //================== Methods

    // Ctor
    ElfSink(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId) :
            MediaSink(env),
            mediaSubsession(subsession),
            streamID(strDup(streamId)) {}

    // Dtor
    ~ElfSink() override;

    Boolean continuePlaying() override;

private: //================== Methods
    static void cbAfterGetFrame(void *clientData, unsigned frameSize,
                                unsigned numTruncatedBytes, struct timeval presentationTime,
                                unsigned durationInMicroseconds);

    void cbAfterGetFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                          struct timeval presentationTime, unsigned durationInMicroseconds);

    static void cbOnSourceClosure(void *data);

    void cbOnSourceClosure1();

private:  //================== Fields
    u_int8_t *buffer = new u_int8_t[BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE + 10];
    MediaSubsession & mediaSubsession;
    char *streamID;
    VDecoder decoder{800, 500};
    int frameCount = 0;
    int shift{4};
};


