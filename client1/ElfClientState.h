//
// Created by Oleksiy Grechnyev on 8/12/18.
//

#pragma once

#include <liveMedia.hh>

struct ElfClientState {
    MediaSubsessionIterator * iter = nullptr;
    MediaSession * session = nullptr;
    MediaSubsession * subsession = nullptr;
    double duration = 0.;
};


