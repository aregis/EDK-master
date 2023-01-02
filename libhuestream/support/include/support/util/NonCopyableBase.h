/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

namespace support {

    class NonCopyableBase {
    protected:
        NonCopyableBase() = default;
        virtual ~NonCopyableBase() = default;

        NonCopyableBase(const NonCopyableBase &) = delete;
        NonCopyableBase(NonCopyableBase &&) = delete;

        NonCopyableBase &operator=(const NonCopyableBase &) = delete;
        NonCopyableBase &operator=(NonCopyableBase &&) = delete;
    };

}  // namespace support
