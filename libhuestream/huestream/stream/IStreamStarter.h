/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_ISTREAMSTARTER_H_
#define HUESTREAM_STREAM_ISTREAMSTARTER_H_

#include "huestream/config/AppSettings.h"

#include <memory>

namespace huestream {

    class IStreamStarter {
    public:
        virtual ~IStreamStarter() {}

        virtual bool StartStream(ActivationOverrideLevel overrideLevel) = 0;

        virtual bool Start(bool force) = 0;

        virtual void Stop() = 0;

        virtual bool DeactivateGroup(std::string groupId) = 0;
    };

    typedef std::shared_ptr<IStreamStarter> StreamStarterPtr;
}  // namespace huestream

#endif  // HUESTREAM_STREAM_ISTREAMSTARTER_H_
