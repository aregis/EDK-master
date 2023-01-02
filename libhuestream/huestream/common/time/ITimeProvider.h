/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_TIME_ITIMEPROVIDER_H_
#define HUESTREAM_COMMON_TIME_ITIMEPROVIDER_H_

#include <memory>

namespace huestream {

    class ITimeProvider {
    public:
        virtual ~ITimeProvider() = default;

        virtual int64_t Now() = 0;
    };

    typedef std::shared_ptr<ITimeProvider> TimeProviderPtr;
}  // namespace huestream

#endif  // HUESTREAM_COMMON_TIME_ITIMEPROVIDER_H_
