#pragma once

#include <queue>
#include "H264Encoder.h"
#include <opencv2/opencv.hpp>
#include <UsageEnvironment.hh>

class LiveSourceWithx264 : public FramedSource {

public:
    static LiveSourceWithx264 *createNew(UsageEnvironment &env);

    static EventTriggerId eventTriggerId;

public:

    unsigned int getEstimatedBitrate() const;

protected:
    explicit LiveSourceWithx264(UsageEnvironment &env);

    ~LiveSourceWithx264() override;

private:
    void doGetNextFrame() override;

    static void deliverFrame0(void *clientData);

    void deliverFrame();

    void encodeNewFrame();

private:

    std::queue<AVPacket> packets;
    timeval currentTime;

    cv::VideoCapture videoCaptureDevice;
    cv::Mat rawImage;

    H264Encoder encoder;
};
