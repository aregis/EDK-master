/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_STREAMFACTORY_H_
#define HUESTREAM_STREAM_STREAMFACTORY_H_

#include "huestream/stream/IStreamFactory.h"

namespace huestream {

    class StreamFactory : public IStreamFactory {
    public:
        StreamFactory();

        StreamStarterPtr CreateStreamStarter(BridgePtr bridge) override;
    };

}  // namespace huestream

#endif  // HUESTREAM_STREAM_STREAMFACTORY_H_
