/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "support/util/Proxy.h"

namespace support {

    class ISubscription {
    public:
        virtual ~ISubscription() = default;

        virtual void enable() = 0;
        virtual void disable() = 0;
    };

    class Subscription : public Proxy<ISubscription> {
    public:
        using Proxy::Proxy;

        void enable() override {
            call(&ISubscription::enable);
        }

        void disable() override {
            call(&ISubscription::disable);
        }
    };

}  // namespace support
