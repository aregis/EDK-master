#ifndef LIBHUESTREAM_MOCKBRIDGESTORAGEACCESSER_H
#define LIBHUESTREAM_MOCKBRIDGESTORAGEACCESSER_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <functional>

#include <huestream/connect/IBridgeStorageAccessor.h>

using namespace testing;
using namespace huestream;

class MockBridgeStorageAccessor : public IBridgeStorageAccessor {
public:
    BridgesLoadCallbackHandler load_callback;
    BridgesSaveCallbackHandler save_callback;

    MOCK_METHOD1(Load, void (BridgesLoadCallbackHandler(cb)));
    MOCK_METHOD2(Save, void(HueStreamDataPtr
            b, BridgesSaveCallbackHandler(cb)));
};

class MockWrapperBridgeStorageAccessor : public IBridgeStorageAccessor {
public:
    explicit MockWrapperBridgeStorageAccessor(const std::shared_ptr<MockBridgeStorageAccessor>& mock)
      : _mock(mock) {}

    void Load(BridgesLoadCallbackHandler cb) {
        _mock->Load(cb);
    }

    void Save(HueStreamDataPtr bridges, BridgesSaveCallbackHandler cb) {
        _mock->Save(bridges, cb);
    }

private:\
    std::shared_ptr<MockBridgeStorageAccessor> _mock;
};

#endif //LIBHUESTREAM_MOCKBRIDGESTORAGEACCESSER_H
