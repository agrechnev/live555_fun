//
// Created by Oleksiy Grechnyev on 8/12/18.
//

#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

#include "ElfSink.h"

//==================================================================================

ElfSink::~ElfSink() {
    delete[] buffer;
    delete[] streamID;
}
//==================================================================================

Boolean ElfSink::continuePlaying() {
    if (!fSource)
        return false;

    fSource->getNextFrame(buffer + shift, BUFFER_SIZE, cbAfterGetFrame, this, cbOnSourceClosure, this);
    return True;
}
//==================================================================================

void ElfSink::cbAfterGetFrame(void *clientData, unsigned frameSize, unsigned numTruncatedBytes,
                              struct timeval presentationTime, unsigned durationInMicroseconds) {
    ((ElfSink *) clientData)->cbAfterGetFrame1(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}
//==================================================================================

void ElfSink::cbAfterGetFrame1(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime,
                               unsigned durationInMicroseconds) {
    using namespace std;
    using namespace cv;

    cout << "cbAfterGetFrame1 : frameSize = " << frameSize;
    cout << " , presentationTime = " << presentationTime.tv_sec << "." << presentationTime.tv_usec << endl;
    cout << "Frame : " << frameCount << endl;

    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0x01;

    // Dump the frame
//    if (frameCount < 20) {
//        ofstream("dump" + to_string(frameCount) + ".bin", ios::binary).write((char *)buffer, frameSize + shift);
//    }
    frameCount++;

    int CAMERA_WIDTH = 800, CAMERA_HEIGHT = 500;

    Mat frameOut; // Out Frames
    Mat frameOutYuv(CAMERA_HEIGHT * 3 / 2, CAMERA_WIDTH, CV_8UC1); // Pre-allocate the YUV frame
    // Decode this data
    if (frameSize > 0) {
        decoder.parse(buffer, frameSize + shift,
                      [&frameOut, &frameOutYuv, CAMERA_WIDTH, CAMERA_HEIGHT]
                              (int ls0, int ls1, int ls2, void *d0, void *d1, void *d2) -> void {
                          // Create a Yuv frame
                          int frWidth = CAMERA_WIDTH;
                          int frHeight = CAMERA_HEIGHT;
                          size_t size0 = (size_t) (frWidth * frHeight / 4);

                          // Copy data line-by-line because of stupid memory alignment in AVFrame
                          for (int i = 0; i < frHeight; ++i) {
                              memcpy(frameOutYuv.data + frWidth * i, d0 + i * ls0, frWidth);
                          }
                          for (int i = 0; i < frHeight / 2; ++i) {
                              memcpy(frameOutYuv.data + 4 * size0 + frWidth * i / 2, d1 + i * ls1,
                                     frWidth/2);
                              memcpy(frameOutYuv.data + 5 * size0 + frWidth * i / 2, d2 + i * ls2,
                                     frWidth/2);
                          }

                          cvtColor(frameOutYuv, frameOut, CV_YUV2BGR_I420); // Convert YUV420p to BGR
                          imshow("frameOut", frameOut);

                          // Will need it here if no other waitKey
                          if (waitKey(1) == 27)
                              exit(0);
                      });
    }

    // Request the next frame
    continuePlaying();
}
//==================================================================================

void ElfSink::cbOnSourceClosure(void *data) {
    ((ElfSink *) data)->cbOnSourceClosure1();
}
//==================================================================================

void ElfSink::cbOnSourceClosure1() {
    std::cout << "cbOnSourceClosure1() \n" << std::endl;

}
//==================================================================================
