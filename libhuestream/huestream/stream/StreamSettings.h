/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_STREAMSETTINGS_H_
#define HUESTREAM_STREAM_STREAMSETTINGS_H_

#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/stream/ProtocolSerializer.h"

#include <memory>

namespace huestream {

    class StreamSettings {
    public:
            StreamSettings();

    PROP_DEFINE(StreamSettings, int, updateFrequency, UpdateFrequency);
    PROP_DEFINE(StreamSettings, ColorSpace, streamingColorSpace, StreamingColorSpace);
    PROP_DEFINE(StreamSettings, int, streamingPort, StreamingPort);
    };

    typedef std::shared_ptr<StreamSettings> StreamSettingsPtr;
}  // namespace huestream

#endif  // HUESTREAM_STREAM_STREAMSETTINGS_H_
