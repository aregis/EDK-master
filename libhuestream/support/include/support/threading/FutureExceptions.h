/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <stdexcept>

namespace support {
    struct FutureException : public std::logic_error {
        using std::logic_error::logic_error;
        FutureException() : std::logic_error::logic_error{"Unexpected future error"}{}
    };

    struct InvalidFutureException : public FutureException {
        InvalidFutureException() : FutureException{"invalid future!"} {}
    };

    struct BrokenPromiseException : public FutureException {
        BrokenPromiseException() : FutureException{"Promise destroyed without setting a value"}{}
    };

    struct PromiseAlreadySetException : public FutureException {
        PromiseAlreadySetException() : FutureException{"Promise is already set"}{}
    };

    struct FutureCanceled : public FutureException {
        FutureCanceled() : FutureException{"Future was canceled before"}{}
    };

    struct EmptyFutureListException : public FutureException {
        EmptyFutureListException() : FutureException{"Future list is empty"}{}
    };

    struct WaitTimeoutException : public FutureException {
        WaitTimeoutException() : FutureException{"wait timeout exception"}{}
    };
}  // namespace support