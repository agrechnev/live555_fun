#include <FramedSource.hh>
#include "LiveSourceWithx264.h"

LiveSourceWithx264 *LiveSourceWithx264::createNew(UsageEnvironment &env) {
    return new LiveSourceWithx264(env);
}

EventTriggerId LiveSourceWithx264::eventTriggerId = 0;

LiveSourceWithx264::LiveSourceWithx264(UsageEnvironment &env) : FramedSource(env) {
    videoCaptureDevice.open(0);
    encoder.initialize();
    if (eventTriggerId == 0) {
        eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    }
}

LiveSourceWithx264::~LiveSourceWithx264() {
    videoCaptureDevice.release();
    encoder.unInitialize();
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
}

void LiveSourceWithx264::encodeNewFrame() {
    rawImage.data = nullptr;
    while (rawImage.empty()) {
        videoCaptureDevice >> rawImage;
    }

    if (rawImage.empty()) {
        return;
    }

    encoder.encodeFrame(rawImage);

    auto encodedPackets = encoder.getEncodedPackets();
    for (auto packet : encodedPackets) {
        packets.push(packet);
    }
}

void LiveSourceWithx264::deliverFrame0(void *clientData) {
    ((LiveSourceWithx264 *) clientData)->deliverFrame();
}

void LiveSourceWithx264::doGetNextFrame() {
    if (packets.empty()) {
        encodeNewFrame();
        gettimeofday(&currentTime, nullptr);
    }

    deliverFrame();
}

void LiveSourceWithx264::deliverFrame() {
    if (!isCurrentlyAwaitingData()) {
        return;
    }

    if (packets.empty()) {
        doGetNextFrame();
        return;
    }

    auto packet = packets.front();
    packets.pop();

    if (packet.data == nullptr) {
        return;
    }

    if (packet.size > fMaxSize) {
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = packet.size - fMaxSize;
    } else {
        fFrameSize = static_cast<unsigned int>(packet.size);
    }

    fPresentationTime = currentTime;
    memmove(fTo, packet.data, fFrameSize);
    FramedSource::afterGetting(this);
}

unsigned int LiveSourceWithx264::getEstimatedBitrate() const {
    return encoder.getBitrate();
}
