/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "support/details/SdkUtilsHelpers.h"

namespace support {

    template<typename To, typename From>
    std::unique_ptr<To> unique_dynamic_cast(std::unique_ptr<From> from) {
        if (dynamic_cast<To*>(from.get()) != nullptr) {
            return std::unique_ptr<To>(dynamic_cast<To*>(from.release()));
        } else {
            return nullptr;
        }
    }
    
    template<typename PointerType>
    void clean_up(PointerType& pointer) {
        delete pointer;
        pointer = nullptr;
    }

    template<typename PointerType>
    void clean_up(std::unique_ptr<PointerType>& pointer) {
        pointer = nullptr;
    }

    template<typename PointerType>
    void clean_up(std::shared_ptr<PointerType>& pointer) {
        pointer = nullptr;
    }

    template<typename PointerType>
    void clean_up_array(PointerType& pointer) {
        delete[] pointer;
        pointer = nullptr;
    }

    /* Covariant return types are not implemented in the same way on all compilers.
    On MSVC this leads to crashes. Therefore 'covariant return type like' functionality is introduced via this wrapper */

    template<typename TargetType, typename SourceType>
    TargetType* clone(const SourceType& p) {
        using PointeeType = std::remove_cv_t<std::remove_reference_t<typename std::pointer_traits<SourceType>::element_type>>;

        static_assert(is_base_of_v<PointeeType, TargetType>, "SourceType is not a base class of TargetType.");

        if (p) {
            auto clone = optional_release(p->clone());
            auto casted_clone = dynamic_cast<TargetType* >(clone);

            if (casted_clone != nullptr) {
                return casted_clone;
            } else {
                clean_up(clone);
                return nullptr;
            }
        }
        return nullptr;
    }

    template<typename SourceType>
    std::remove_cv_t<std::remove_reference_t<SourceType>>* clone(const SourceType* const & p) {
        using TargetType = std::remove_cv_t<std::remove_reference_t<SourceType>>;
        return clone<TargetType, const SourceType*>(p);
    }

    template<typename SourceType>
    std::remove_cv_t<std::remove_reference_t<SourceType>>* clone(const std::unique_ptr<SourceType>& p) {
        using TargetType = std::remove_cv_t<std::remove_reference_t<SourceType>>;
        return clone<TargetType>(p.get());
    }

    template<typename SourceType>
    std::remove_cv_t<std::remove_reference_t<SourceType>>* clone(const std::shared_ptr<SourceType>& p) {
        using TargetType = std::remove_cv_t<std::remove_reference_t<SourceType>>;
        return clone<TargetType>(p.get());
    }
}  // namespace support
