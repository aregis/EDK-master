

#ifndef LIBHUESTREAM_MOCKSTREAMFACTORY_H
#define LIBHUESTREAM_MOCKSTREAMFACTORY_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <functional>
#include "huestream/stream/IStreamFactory.h"

using namespace testing;
using namespace huestream;

class MockStreamFactory : public IStreamFactory {
public:
    MOCK_METHOD1(CreateStreamStarter, StreamStarterPtr(BridgePtr
            bridge));
};

#endif //LIBHUESTREAM_MOCKSTREAMFACTORY_H
