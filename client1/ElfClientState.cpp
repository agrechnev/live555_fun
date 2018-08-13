//
// Created by Oleksiy Grechnyev on 8/13/18.
//

#include <sstream>
#include <iostream>

#include "./ElfRTSPClient.h"

//==================================================================================
void ElfClientState::parseFrameResolution() {
    using namespace std;

    if (sdpLines.empty())
        return;

    // Find a substring between "a=framesize" and EOF
    string marker("a=framesize");
    auto pos = sdpLines.find(marker);
    if (pos == string::npos)
        return;

    istringstream iss(sdpLines.substr(pos + marker.size()));
//    cout << "ElfClientState::parseFrameResolution() : SUBSTR = \n" << sdpLines.substr(pos + marker.size()) << endl;

    char c1, c2;
    int temp;
    iss >> c1 >> temp >> frameWidth >> c2 >> frameHeight;
}
//==================================================================================
