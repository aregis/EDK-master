#include <stdio.h>
#include <thread>
#include <iostream>
#include <support/logging/Log.h>
#include <huestream/config/Config.h>
#include <huestream/HueStream.h>
#include <huestream/effect/effects/SequenceEffect.h>
#include <sstream>
#include <Windows.h>
#include "testwrapper.h"
#include "testserver.h"
#include "version.h"

using namespace huestream;

constexpr static auto PERSISTENCE_ENCRYPTION_KEY = "encryption_key";

struct CommandLineArguments {
    bool dontMeasure;
    string bridgeIpAddress;
    string userName;
    string clientKey;
    int numEffectRepeats;
    int framesPerSecond;
};

static shared_ptr<HueStream> _huestream;

void InitializeHueStream(int framesPerSecond) {
    auto config = std::make_shared<Config>("huestream_performance_test", "Windows", PersistenceEncryptionKey(PERSISTENCE_ENCRYPTION_KEY));
    config->GetStreamSettings()->SetUpdateFrequency(framesPerSecond);
    //Get HueStream instance to work with ...
    _huestream = std::make_shared<HueStream>(config);

    //Register feedback callback
    _huestream->RegisterFeedbackCallback([](const FeedbackMessage &message) {
        std::cout << "[" << message.GetId() << "] " << message.GetTag() << std::endl;
    });
}

void Connect(string ipAddress, string username, string clientKey) {
    auto bridge = std::make_shared<Bridge>(_huestream->GetConfig()->GetBridgeSettings());
    bridge->SetIpAddress(ipAddress);
    bridge->SetUser(username);
    bridge->SetClientKey(clientKey);
    _huestream->ConnectManualBridgeInfo(bridge);
}

std::shared_ptr<SequenceEffect> CreateEffect() {
    auto _effect = std::make_shared<SequenceEffect>("seq", 1);
    _huestream->AddEffect(_effect);
    _effect->Enable();
    _effect->SetMode(huestream::SequenceModeAll);
    _effect->SetBri(1);
    _effect->AddColor(Color(0.0, 0.0, 0.0));
    _effect->AddColor(Color(1.0, 1.0, 1.0));
    return _effect;
}

void PlayEffect(bool *connectionFlag) {
}

int ParseStringToInt(string s) {
    int value;
    istringstream ss(s);

    if (!(ss >> value))
        cerr << "Invalid number " << s << '\n';
    return value;
}

bool hasOptions(char *arguments[]) {
    return arguments[1][0] == '-';
}

CommandLineArguments ParseArguments(int argc, char *argv[]) {
    if (argc < 6 || argc > 7) {
        throw exception(
                "Program needs 1 option and 5 arguments: [-d] bridgeIp, username, clientKey, numEffectRepeats, framesPerSecond");
    }

    CommandLineArguments arguments = {};
    arguments.dontMeasure = false;

    int argumentIndex = 1;
    if (hasOptions(argv)) {
        string options = argv[argumentIndex++];
        int optionIndex = 1;
        char option = options[optionIndex++];
        while (option != 0) {
            switch (option) {
                case 'd':
                    arguments.dontMeasure = true;
                    break;
                default:
                    break;
            }
            option = options[optionIndex++];
        }
    }

    arguments.bridgeIpAddress = argv[argumentIndex++];
    arguments.userName = argv[argumentIndex++];
    arguments.clientKey = argv[argumentIndex++];
    arguments.numEffectRepeats = ParseStringToInt(argv[argumentIndex++]);
    arguments.framesPerSecond = ParseStringToInt(argv[argumentIndex++]);
    return arguments;
}

void StartServerForPi(hardwaretimer *timer, bool *ptrConnectionFlag) {
    std::thread serverThread(server::testserver::serverinit, timer, ptrConnectionFlag);
    serverThread.detach();
}

int main(int argc, char *argv[]) {
    printf("PerformanceTest version: %s %s %s %s\n\n", VERSION, GIT_HASH, DATETIME, GIT_BRANCH);

    int returnValue = 0;
    setbuf(stdout, nullptr);

    try {
        CommandLineArguments arguments = ParseArguments(argc, argv);
        hardwaretimer *timer_1 = new hardwaretimer();
        bool connectionFlag = false;
        bool *ptrconnectionFlag = &connectionFlag;

        puts("STARTED");
        if (!arguments.dontMeasure) {
            StartServerForPi(timer_1, ptrconnectionFlag);
        }

        InitializeHueStream(arguments.framesPerSecond);
        ::Connect(arguments.bridgeIpAddress, arguments.userName, arguments.clientKey);
        puts("BRIDGE_CONNECTED");
        Sleep(2000);

        timer_1->Getstarttime();
        auto _effect = CreateEffect();
        puts("MEASUREMENT_STARTED");
        for (int i = 0; i < arguments.numEffectRepeats; i++) {
            puts("STEP...");
            _effect->Step();
            timer_1->Getstarttime();
            Sleep(500);
            _effect->Step();
            Sleep(500);
        }
        _huestream->ShutDown();
        puts("MEASUREMENT_DONE");

        //get stream counter
        auto numberOfFrames = _huestream->GetStreamCounter();
        printf("FrameCount: %d\n", numberOfFrames);

        while (true) {
            if (connectionFlag == false) {
                break;
            }
        }
    }
    catch (exception e) {
        printf(e.what());
        returnValue = -1;
    }
    return returnValue;
}
