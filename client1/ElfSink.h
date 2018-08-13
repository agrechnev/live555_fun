//
// Created by Oleksiy Grechnyev on 8/12/18.
//

#pragma once

#include <string>
#include <vector>

#include <liveMedia.hh>

#include "./VDecoder.h"

class ElfSink : public MediaSink {
public: //================== Parameter
    static constexpr int BUFFER_SIZE = 1000000;
    static constexpr int BUFFER_TOTAL_SIZE = BUFFER_SIZE + 100;
public: //================== Methods

    static ElfSink *createNew(UsageEnvironment &env, MediaSubsession &subsession,
                              int frWidth, int frHeight,  const std::string streamId) {
        return new ElfSink(env, subsession, frWidth, frHeight, streamId);
    }

protected: //================== Methods

    // Ctor
    ElfSink(UsageEnvironment &env, MediaSubsession &subsession,
            int frWidth, int frHeight, const std::string & streamId) :
            MediaSink(env),
            mediaSubsession(subsession),
            frWidth(frWidth),
            frHeight(frHeight),
            decoder(frWidth, frHeight),
            streamID(streamId) {}

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
    // Frame resolution
    int frWidth = 0, frHeight = 0;

//    u_int8_t *buffer = new u_int8_t[BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE + 10];
    std::vector <uint8_t > buffer =  std::vector <uint8_t >(BUFFER_TOTAL_SIZE);

    MediaSubsession &mediaSubsession;
    std::string streamID;
    VDecoder decoder;
    int frameCount = 0;
    int shift{4};
};


