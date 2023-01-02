/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/Stream.h>
#include <huestream/stream/DtlsConnector.h>
#include <huestream/stream/StreamFactory.h>
#include <huestream/config/Config.h>

#include <memory>
#include <iomanip>
#include <sstream>

using std::make_shared;
using std::thread;

namespace huestream {

    Stream::Stream(StreamSettingsPtr hueSettings, AppSettingsPtr appSettings, TimeManagerPtr timeManager,
                   ConnectorPtr connector) :
            Stream(hueSettings, appSettings, timeManager, connector, std::make_shared<StreamFactory>()) {
    }

    Stream::Stream(StreamSettingsPtr streamSettings, AppSettingsPtr appSettings, TimeManagerPtr timeManager,
                   ConnectorPtr connector, std::shared_ptr<IStreamFactory> factory) :
            _streamSettings(streamSettings),
            _appSettings(appSettings),
            _activeBridgeCopy(),
            _factory(factory),
            _renderThread(),
            _running(false),
            _seqNr(0),
            _timeManager(timeManager),
            _connector(connector),
            _streamCounter(0) {
    }

    Stream::~Stream() {
        Stop();
    }

    void Stream::SetRenderCallback(StreamRenderCallback callback) {
        std::lock_guard<std::mutex> lock(_lock);
        _renderCallback = callback;
    }

    bool Stream::Start(BridgePtr bridge) {
        if (IsStreamingToBridgeAndGroup(bridge)) {
            // We assume that Start has been called before and so we can ignore this start call
            return true;
        }

        if (_running) {
            // If we where running, first stop
            Stop();
        }

        bridge->SetIsBusy(false);
        if (bridge->IsStreaming()) {
            // In this case it seems that there is already an exact same session active on the bridge, like the one we want
            // to start, yet it was not started by this Stream instance. Here we assume that a previous session as abruptly
            // ended and bridge waiting to timeout the previous session. We will force a stop of that session..
            _factory->CreateStreamStarter(bridge)->Stop();

            // TODO(user): This is a temporary workaround until this is fixed in the bridge
            _timeManager->Sleep(500);
        }

        if (!bridge->IsReadyToStream()) {
            // we will not attempt to connect if the bridge is not Streamable
            _activeBridgeCopy = nullptr;
            return false;
        }

        // Ok, now we are ready to connect
        _options = std::make_shared<StreamOptions>();
        _options->colorSpace = _streamSettings->GetStreamingColorSpace();
				_options->useClipV2 = bridge->IsSupportingClipV2();
        UpdateBridgeGroup(bridge);

        return StartStreamingSession(bridge);
    }

    bool Stream::IsStreamingToBridgeAndGroup(BridgePtr bridge) const {
        if (_activeBridgeCopy == nullptr) {
            return false;
        }
        return _running && IsSameBridgeAndGroup(bridge);
    }

    bool Stream::IsSameBridgeAndGroup(BridgePtr bridge) const {
        return bridge->GetId() == _activeBridgeCopy->GetId() &&
               bridge->GetIpAddress() == _activeBridgeCopy->GetIpAddress() &&
               bridge->GetSelectedGroup() == _activeBridgeCopy->GetSelectedGroup();
    }

    bool Stream::StartStreamingSession(BridgePtr bridge) {
        // ready to start the stream session
        auto startSuccessful = _factory->CreateStreamStarter(bridge)->StartStream(_appSettings->GetActivationOverride());
        if (!startSuccessful) {
            return false;
        }
        _activeBridgeCopy = make_shared<Bridge>(*bridge);

        auto connectSuccessful = false;
        auto connectAttemps = 0;
        while (!connectSuccessful && connectAttemps < 5) {
            // now we can connect to the streaming port

            // TODO(user): This is a temporary workaround until this is fixed in the bridge
            _timeManager->Sleep(500);
            _running = true;

            connectSuccessful = _connector->Connect(bridge, (uint16_t) _streamSettings->GetStreamingPort());
            connectAttemps++;
        }
        if (!connectSuccessful) {
            Stop();
            return false;
        }

        _seqNr = 0;
        _streamCounter = 0;
        _timeManager->UpdateTime();
        return true;
    }

    void Stream::UpdateBridgeGroup(BridgePtr bridge) {
        if (_options != nullptr && bridge->IsValidGroupSelected()) {
            _options->group = bridge->GetGroup();
        }
    }

    bool Stream::StartWithRenderThread(BridgePtr bridge) {
        if (IsStreamingToBridgeAndGroup(bridge)) {
            return true;
        }

        if (!Start(bridge)) {
            return false;
        }
        _renderThread = make_shared<thread>(&Stream::RenderThread, this);

        return true;
    }

    void Stream::Stop() {
        if (!_running) {
            return;
        }

        Stop(_activeBridgeCopy);
    }

    void Stream::Stop(BridgePtr bridge) {
        std::lock_guard<std::mutex> lock(_lock);

        if (!_running) {
            return;
        }

        _running = false;

        if (_renderThread != nullptr) {
            _renderThread->join();
            _renderThread.reset();
        }

        _connector->Disconnect();

        if (!IsSameBridgeAndGroup(bridge)) {
            _factory->CreateStreamStarter(_activeBridgeCopy)->Stop();
        }
        if (bridge->IsStreaming()) {
            _factory->CreateStreamStarter(bridge)->Stop();
        }
    }

    bool Stream::IsStreaming() const {
        return _running;
    }

    void Stream::RenderThread() {
        auto sendPeriod(
                (int64_t) ((1.0 / _streamSettings->GetUpdateFrequency()) * 1000));

        while (_running) {
            auto timeBeforeRender = UpdateAndGetCurrentTime();

            RenderSingleFrame();

            auto renderDuration = UpdateAndGetCurrentTime() - timeBeforeRender;

            auto sleepTime = sendPeriod - renderDuration;
            if (sleepTime > 0)
                _timeManager->Sleep(sleepTime);
        }
    }

    void Stream::RenderSingleFrame() {
        std::lock_guard<std::mutex> lock(_lock);

        if (!_running)
            return;

        _streamCounter++;
        _timeManager->UpdateTime();

        if (_renderCallback)
            _renderCallback();

        auto payload = ProtocolSerializer(_options).Serialize(_seqNr++);
        _connector->Send(reinterpret_cast<const char *>(payload.data()), payload.size());
    }

    int32_t Stream::GetStreamCounter() const {
        return _streamCounter;
    }

    int64_t Stream::UpdateAndGetCurrentTime() const {
        _timeManager->UpdateTime();
        return _timeManager->Now();
    }

}  // namespace huestream
