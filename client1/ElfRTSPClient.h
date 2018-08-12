//
// Created by Oleksiy Grechnyev on 8/12/18.
//

#pragma once

#include <RTSPClient.hh>

#include "./ElfClientState.h"

class ElfRTSPClient : public RTSPClient {
public: //================== Methods
    static ElfRTSPClient *
    createNew(UsageEnvironment &env, const char *rtspURL, int verbosityLevel = 0, const char *applicationName = nullptr,
              portNumBits tunnelOverHTTPPortNum = 0) {
        return new ElfRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
    }

protected: //================== Methods
    ElfRTSPClient(UsageEnvironment &env, const char *rtspURL, int verbosityLevel, const char *applicationName,
                  portNumBits tunnelOverHTTPPortNum) :
            RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {}

    ~ElfRTSPClient() override {}

public:  //================== Fields
    ElfClientState scs;
};


