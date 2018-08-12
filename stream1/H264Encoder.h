#pragma once

#include <queue>
#include <opencv2/core/mat.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
}

class H264Encoder {

public:
    void initialize();

    void unInitialize();

    void encodeFrame(const cv::Mat &image);

    std::vector<AVPacket> getEncodedPackets();

    unsigned int getBitrate() const;

private:

    void preprocessImage(const cv::Mat &image);

private:
    AVCodec *encoder;
    cv::Mat encodingImage;
    AVCodecContext *codecContext;
    AVFrame *frame;

    int totalFrameSize = 0;
};
