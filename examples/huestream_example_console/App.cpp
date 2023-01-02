/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream_example_console/Terminal.h>
#include <huestream_example_console/App.h>
#include <support/network/NetworkConfiguration.h>

#include <string>
#include <memory>
#include <cstdlib>
#include <iostream>

using std::cout;
using std::endl;
using std::make_shared;
using std::string;

#define CONNECTION_MODE_AUTO_STRING "auto"
#define CONNECTION_MODE_MANUAL_STRING "manual"
#define CONNECTION_MODE_MANUALIP_STRING "manualip"
#define CONNECTION_MODE_RESET_STRING  "clear"

#define STREAMING_MODE_UDP_STRING "udp"
#define STREAMING_MODE_DTLS_STRING "dtls"

constexpr static auto PERSISTENCE_ENCRYPTION_KEY = "encryption_key";

App::App(const string &_appName, const string &_deviceName) : _appName(_appName), _deviceName(_deviceName) {}

void App::Main(int argc, char **argv) {
    ParseArguments(argc, argv);
    InitializeHueStream();
    ConnectToBridge();
    StartExample();
    _huestream->ShutDown();
}

void App::ConnectToBridge() {
    switch (_command.mode) {
        case CONNECTION_MODE_AUTO:
            ExecuteConnectionModeAuto();
            break;
        case CONNECTION_MODE_MANUAL_IP:
            ExecuteConnectionModeManualIp();
            break;
        case CONNECTION_MODE_MANUAL:
            ExecuteConnectionModeManual();
            break;
        case CONNECTION_MODE_RESET:
            ExecuteConnectionModeReset();
            ExecuteConnectionModeAuto();
            break;
    }
}

void App::ParseArguments(int argc, char *const *argv) {
    std::cout << "Entertainment Example: " << _appName << std::endl;
    std::cout << "Build: " << BUILD_INFO << std::endl;

    _command.isValid = true;
    auto currentArg = 1;
    if (argc == 1) {
        _command.mode = CONNECTION_MODE_AUTO;
        _command.streamingMode = huestream::STREAMING_MODE_DTLS;
    } else {
        if (ArgumentHasValue(argv[currentArg], CONNECTION_MODE_AUTO_STRING)) {
            _command.mode = CONNECTION_MODE_AUTO;
            currentArg++;
        } else if (ArgumentHasValue(argv[currentArg], CONNECTION_MODE_RESET_STRING)) {
            _command.mode = CONNECTION_MODE_RESET;
            currentArg++;
        } else if (ArgumentHasValue(argv[currentArg], CONNECTION_MODE_MANUAL_STRING)) {
            _command.mode = CONNECTION_MODE_MANUAL;
            currentArg++;
            if (argc >= currentArg + 3) {
                _command.ip = argv[currentArg++];
                _command.username = argv[currentArg++];
                _command.clientkey = argv[currentArg++];
            } else {
                _command.isValid = false;
            }
        } else if (ArgumentHasValue(argv[currentArg], CONNECTION_MODE_MANUALIP_STRING)) {
            _command.mode = CONNECTION_MODE_MANUAL_IP;
            currentArg++;
            if (argc >= currentArg + 1) {
                _command.ip = argv[currentArg++];
            } else {
                _command.isValid = false;
            }
        }
        if (currentArg == argc - 1) {
            ParseStreamingMode(argv[currentArg]);
        } else {
            _command.isValid = false;
        }
    }
    if (!_command.isValid) {
        PrintTutorialAndExit();
    }
}

bool App::ArgumentHasValue(const char *argument, const char *value) const {
    return string(argument).compare(value) == 0;
}

void App::PrintTutorialAndExit() const {
    cout << "Usage: " << endl;
    cout << _appName << " [bridge-connect-mode] [bridge-connect-mode-args] [streaming-mode]" << endl;
    cout << "    where bridge-connect-mode is:" << endl;
    cout << "      * " << CONNECTION_MODE_AUTO_STRING << endl;
    cout << "        search bridge and/or connect with (last used) bridge" << endl;
    cout << "        (default mode when no mode is given)" << endl;
    cout << "      * " << CONNECTION_MODE_RESET_STRING << endl;
    cout << "        same as " << CONNECTION_MODE_AUTO_STRING << " but first resets stored bridge information"
         << endl;
    cout << "      * " << CONNECTION_MODE_MANUAL_STRING << " <ip> <username> <clientkey>" << endl;
    cout << "        same as " << CONNECTION_MODE_AUTO_STRING << " but uses manual provided ip, username and clientkey"
         << endl;
    cout << "      * " << CONNECTION_MODE_MANUALIP_STRING << " <ip>" << endl;
    cout << "        same as " << CONNECTION_MODE_AUTO_STRING << " but uses manual provided ip"
         << endl;
    cout << "    and streaming-mode is:" << endl;
    cout << "      * " << STREAMING_MODE_UDP_STRING << endl;
    cout << "        unencrypted UDP for using increment 2 bridges or simulator" << endl;
    cout << "      * " << STREAMING_MODE_DTLS_STRING << endl;
    cout << "        encrypted with DTLS for using with increment 3+ and product version bridges" << endl;
    cout << "        (default mode when no mode is given)" << endl;
    exit(1);
}

void App::ParseStreamingMode(const char *argument) {
    if (string(argument).compare(STREAMING_MODE_UDP_STRING) == 0) {
        _command.streamingMode = huestream::STREAMING_MODE_UDP;
    } else if (string(argument).compare(STREAMING_MODE_DTLS_STRING) == 0) {
        _command.streamingMode = huestream::STREAMING_MODE_DTLS;
    } else {
        _command.isValid = false;
    }
}

void App::ExecuteConnectionModeAuto() const {
    std::cout << "Connecting to bridge " << std::endl;
    _huestream->ConnectBridge();
    CheckConnection();
    std::cout << "Connecting to bridge done " << std::endl;
}

void App::ExecuteConnectionModeManual() const {
    std::cout << "Connection with manual bridge information ip=" << _command.ip << " username="
              << _command.username << std::endl;

    auto bridge = std::make_shared<huestream::Bridge>(make_shared<huestream::BridgeSettings>());
    bridge->SetIpAddress(_command.ip);
    bridge->SetUser(_command.username);
    bridge->SetClientKey(_command.clientkey);
    _huestream->ConnectManualBridgeInfo(bridge);
    CheckConnection();

    std::cout << "Connecting with manual bridge information done" << std::endl;
}

void App::CheckConnection() const {
    while (!this->_huestream->IsBridgeStreaming()) {
        auto bridge = this->_huestream->GetLoadedBridge();

        if (bridge->GetStatus() == huestream::BRIDGE_INVALID_GROUP_SELECTED) {
            std::cout << "Please enter groupid you want to select from the available groups" << std::endl;
            SelectGroup();
        } else {
            std::cout << "No streamable bridge configured: " << bridge->GetStatusTag() << std::endl <<
                      "Press any key to connect to search for a new bridge, press q to quit" << endl;
            PressAnyKeyOrQToQuit();
            this->_huestream->ConnectBridge();
        }
    }
}

void App::ExecuteConnectionModeManualIp() {
    std::cout << "Connection with manual bridge ip=" << _command.ip << std::endl;

    _huestream->ConnectBridgeManualIp(_command.ip);

    CheckConnection();

    std::cout << "Connecting with manual bridge ip done" << std::endl;
}

void App::ExecuteConnectionModeReset() const {
    cout << "Resetting bridge information" << endl;

    _huestream->ResetBridgeInfo();

    cout << "Resetting bridge information done" << endl;
}

void App::InitializeHueStream() {
    auto config = make_shared<huestream::Config>(_appName, _deviceName, huestream::PersistenceEncryptionKey(PERSISTENCE_ENCRYPTION_KEY));
    cout << "Initializing with streaming mode ";
    config->SetStreamingMode(_command.streamingMode);
	support::NetworkConfiguration::set_use_http2(_command.streamingMode != huestream::STREAMING_MODE_UDP);
    switch (_command.streamingMode) {
        case huestream::STREAMING_MODE_DTLS:
            cout << STREAMING_MODE_DTLS_STRING << endl;
            break;
        default:
            cout <<STREAMING_MODE_UDP_STRING << endl;
            break;
    }

    _huestream = make_shared<huestream::HueStream>(config);

    //  Register feedback callback
    _huestream->RegisterFeedbackCallback([](const huestream::FeedbackMessage &message) {
        std::cout << "[" << message.GetId() << "] " << message.GetTag() << std::endl;
        if (message.GetId() == huestream::FeedbackMessage::ID_DONE_COMPLETED) {
            std::cout << "bridge-ip: " << message.GetBridge()->GetIpAddress() << std::endl;
            std::cout << "bridge-username: " << message.GetBridge()->GetUser() << std::endl;
            std::cout << "bridge-clientkey: " << message.GetBridge()->GetClientKey() << std::endl;
        }
        if (message.GetMessageType() == huestream::FeedbackMessage::FEEDBACK_TYPE_USER) {
            std::cout << message.GetUserMessage() << std::endl;
        }
    });
}

void App::PressAnyKeyOrQToQuit() const {
    auto key = Terminal::GetChar();
    if (key == 'q') {
        _huestream->Stop();
        exit(0);
    }
}

void App::SelectGroup() const {
    auto groups = _huestream->GetLoadedBridgeGroups();
    for (int i = 0; i < groups->size(); ++i) {
        auto groupId = groups->at(i)->GetId();
        auto name = groups->at(i)->GetName();
        std::cout << "[" << i << "] id:"  << groupId << ", name: " << name << std::endl;
    }
    auto key = Terminal::GetChar();

    if (isdigit(key)) {
        int i = key - '0';
        auto groupId = groups->at(i)->GetId();
        _huestream->SelectGroup(groupId);
        std::cout << "bridge-groupid: " << groupId << std::endl;
    }
}
