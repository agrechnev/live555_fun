//
// Created by Oleksiy Grechnyev on 8/12/18.
//

#pragma once

#include <string>

#include <liveMedia.hh>

struct ElfClientState {
    std::string sdpLines;

    MediaSubsessionIterator * iter = nullptr;
    MediaSession * session = nullptr;

    /// Curent subsession
    MediaSubsession * subsession = nullptr;

    /// Video subsession
    MediaSubsession * subsessionVideo = nullptr;

    double duration = 0.;

    int frameWidth = 0;
    int frameHeight = 0;

    /// Parse frame resolution from sdpLines if available
    void parseFrameResolution();
};


