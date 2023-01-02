/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKCONNECTOR_H_
#define LIBHUESTREAM_TEST_MOCKCONNECTOR_H_

#include <gmock/gmock.h>

#include <memory>

#include "huestream/stream/IConnector.h"

namespace huestream {

    class MockConnector : public IConnector {
    public:
        MockConnector() {}

        MOCK_METHOD2(Connect, bool(BridgePtr bridge, unsigned short port));
        MOCK_METHOD0(Disconnect, void());
        MOCK_METHOD2(Send, bool(const char *buffer, unsigned int bufferSize));
    };

    class MockWrapperConnector : public IConnector {
    public:
        explicit MockWrapperConnector(const std::shared_ptr<MockConnector>& mock)
          : _mock(mock) {}

        bool Connect(BridgePtr bridge, unsigned short port) {
            return _mock->Connect(bridge, port);
        }

        void Disconnect() {
            _mock->Disconnect();
        }

        bool Send(const char* buffer, unsigned int bufferSize) {
            return _mock->Send(buffer, bufferSize);
        }

    private:
        std::shared_ptr<MockConnector> _mock;
    };

}// namespace huestream
#endif     // LIBHUESTREAM_TEST_MOCKCONNECTOR_H_
