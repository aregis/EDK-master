/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_ICONNECTOR_H_
#define HUESTREAM_STREAM_ICONNECTOR_H_

#include "huestream/common/data/Bridge.h"

#include <string>
#include <memory>

namespace huestream {

    class IConnector {
    public:
        IConnector() {}

        virtual ~IConnector() = default;

        virtual bool Connect(BridgePtr bridge, unsigned short port) = 0;

        virtual void Disconnect() = 0;

        virtual bool Send(const char *buffer, unsigned int bufferSize) = 0;
    };

    typedef std::shared_ptr<IConnector> ConnectorPtr;

}  // namespace huestream

#endif  // HUESTREAM_STREAM_ICONNECTOR_H_
