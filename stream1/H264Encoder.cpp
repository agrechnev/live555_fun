#include "H264Encoder.h"
#include <opencv/cv.hpp>

extern "C" {
#include <libavutil/opt.h>
}

#include <iostream>
#include <fstream>

void H264Encoder::initialize() {
    encoder = avcodec_find_encoder(AV_CODEC_ID_H264);

    codecContext = avcodec_alloc_context3(encoder);

    codecContext->bit_rate = getBitrate();
    codecContext->width = 1280;
    codecContext->height = 720;
    codecContext->time_base = (AVRational) {1, 60};
    codecContext->gop_size = 25;
    codecContext->level = 31;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    av_opt_set(codecContext->priv_data, "preset", "medium", 0);
    av_opt_set(codecContext->priv_data, "tune", "zerolatency", 0);
    av_opt_set(codecContext->priv_data, "crf", "17", 0);

    frame = av_frame_alloc();

    frame->format = codecContext->pix_fmt;
    frame->width = codecContext->width;
    frame->height = codecContext->height;

    totalFrameSize = frame->width * frame->height;

    frame->linesize[0] = frame->width;
    frame->linesize[1] = frame->width / 2;
    frame->linesize[2] = frame->width / 2;

    avcodec_open2(codecContext, encoder, nullptr);
}

void H264Encoder::unInitialize() {
    avcodec_close(codecContext);
    av_free(codecContext);
    av_frame_free(&frame);
}

void H264Encoder::encodeFrame(const cv::Mat &image) {
    preprocessImage(image);

    frame->data[0] = encodingImage.data;
    frame->data[1] = encodingImage.data + totalFrameSize;
    frame->data[2] = encodingImage.data + totalFrameSize + totalFrameSize / 4;

    avcodec_send_frame(codecContext, frame);
}

std::vector<AVPacket> H264Encoder::getEncodedPackets() {
    std::vector<AVPacket> packets;

    AVPacket packet;
    int resultCode = 0;

    while (resultCode == 0) {
        av_init_packet(&packet);
        resultCode = avcodec_receive_packet(codecContext, &packet);

        if (resultCode >= 0) {
            uint8_t *data = packet.data;

            std::vector<std::pair<int, size_t>> nalsPositions;
            int startSize = 0;
            int currentItem = -1;

            for (int i = 0; i < packet.size; i++) {
                if (packet.size > i + 3 && data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 1) {
                    startSize = 3;
                } else if (packet.size > i + 4 && data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 0 &&
                           data[i + 3] == 1) {
                    startSize = 4;
                } else {
                    startSize = 0;
                }

                i += startSize;
                if (startSize != 0) {
                    currentItem++;
                    nalsPositions.emplace_back();
                    nalsPositions[currentItem].first = i;
                    nalsPositions[currentItem].second = 0;
                }

                nalsPositions[currentItem].second = nalsPositions[currentItem].second + 1;
            }

            for (auto nalPosition : nalsPositions) {
                AVPacket tmp;
                av_init_packet(&tmp);

                tmp.data = static_cast<uint8_t *>(av_malloc(nalPosition.second));

                memcpy(tmp.data, packet.data + nalPosition.first, nalPosition.second);
                tmp.size = static_cast<int>(nalPosition.second);
                packets.push_back(tmp);
            }

            continue;
        }

        if (resultCode == AVERROR(EAGAIN)) {
            break;
        }


        char errorMessage[512];
        av_make_error_string(errorMessage, sizeof(errorMessage), resultCode);

        std::cout << "Unable to receive packet from encoder: " << errorMessage << std::endl;
    }

    return packets;
}

void H264Encoder::preprocessImage(const cv::Mat &image) {
    cv::resize(image, encodingImage, cv::Size(codecContext->width, codecContext->height));
    cv::cvtColor(encodingImage, encodingImage, cv::COLOR_BGR2YUV_I420);
}

unsigned int H264Encoder::getBitrate() const {
    return 90000;
}
