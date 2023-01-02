/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EXAMPLE_CONSOLE_APP_H_
#define HUESTREAM_EXAMPLE_CONSOLE_APP_H_

#include <huestream/config/Config.h>
#include <huestream/HueStream.h>

#include <string>

using huestream::HueStreamPtr;

typedef enum {
    CONNECTION_MODE_AUTO,
    CONNECTION_MODE_MANUAL_IP,
    CONNECTION_MODE_MANUAL,
    CONNECTION_MODE_RESET,
} ConnectionMode;

typedef struct {
    huestream::StreamingMode streamingMode;
    ConnectionMode mode;
    std::string ip;
    std::string username;
    std::string clientkey;
    bool isValid;
} TerminalCommand;

class App {
public:
    App(const std::string &_appName, const std::string &_deviceName);

    void Main(int argc, char *argv[]);

    std::string _appName;
    std::string _deviceName;
    TerminalCommand _command;

    void ExecuteConnectionModeAuto() const;

    void ExecuteConnectionModeManualIp();

    void ExecuteConnectionModeManual() const;

    void ExecuteConnectionModeReset() const;

    void InitializeHueStream();

    void ParseArguments(int argc, char *const *argv);

    void ConnectToBridge();

    void PressAnyKeyOrQToQuit() const;

    void SelectGroup() const;

    virtual void StartExample() = 0;

    void ParseStreamingMode(const char *argument);

    void PrintTutorialAndExit() const;

    bool ArgumentHasValue(const char *argument, const char *value) const;

    void CheckConnection() const;

 protected:
    HueStreamPtr _huestream;
};

#endif  // HUESTREAM_EXAMPLE_CONSOLE_APP_H_
