/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <jni.h>
#include <memory>
#include <type_traits>
#include "Converter.h"
#include "JObjectFactory.h"

namespace huesdk_jni_core {

    namespace Detail {
        template<typename Base, typename... Types>
        struct IsBaseOfAny;

        template<typename Base, typename Head, typename... Tail>
        struct IsBaseOfAny<Base, Head, Tail...> {
            static constexpr bool value = std::is_base_of<Base, Head>::value || IsBaseOfAny<Base, Tail...>::value;
        };

        template<typename Base>
        struct IsBaseOfAny<Base> {
            static constexpr bool value = false;
        };

        template<typename Head, typename... Tail>
        struct DerivedAfterBase {
            static constexpr bool value = IsBaseOfAny<Head, Tail...>::value || DerivedAfterBase<Tail...>::value;
        };

        template<typename Head>
        struct DerivedAfterBase<Head> {
            static constexpr bool value = false;
        };

        template<typename Base, typename... Types>
        struct ConvertFromMostDerived;

        template<typename Base, typename Head, typename... Tail>
        struct ConvertFromMostDerived<Base, Head, Tail...> {
            static jobject convert(const std::shared_ptr<Base>& obj) {
                if (const auto derived_obj = std::dynamic_pointer_cast<Head>(obj)) {
                    return from<std::shared_ptr<Head>>(derived_obj).value();
                } else {
                    return ConvertFromMostDerived<Base, Tail...>::convert(obj);
                }
            }
        };

        template<typename Base>
        struct ConvertFromMostDerived<Base> {
            static jobject convert(const std::shared_ptr<Base>& value) {
                return create_jobject(value);
            }
        };


    }  // namespace Detail

    template<typename... Types>
    struct from_inheritance_tree {
        template<typename Base>
        explicit from_inheritance_tree(const std::shared_ptr<Base>& obj) {
            static_assert(!Detail::DerivedAfterBase<Types...>::value, "A base class appears earlier in the types list than one of its derived classes");

            _value = Detail::ConvertFromMostDerived<Base, Types...>::convert(obj);
        }

        jobject value() const { return _value; }
        operator jobject() const { return _value; }  // NOLINT
        jobject _value;
    };

}  // namespace huesdk_jni_core
