/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <type_traits>

namespace support {

    /**
     * Type safe wrapper for flag enums.
     *
     * Requirement: static_cast<underlying>(a) & static_cast<underlying>(b) == 0 for any values a, b of Enum
     */
    template<typename Enum>
    class EnumSet final {
    public:
        using underlying = typename std::underlying_type<Enum>::type;

        constexpr EnumSet() : _values() {}

        constexpr EnumSet(Enum e) : _values(static_cast<underlying>(e)) {}

        EnumSet(std::initializer_list<Enum> values) : _values(0) {
            for (auto v : values) {
                add(v);
            }
        }

        constexpr explicit EnumSet(underlying values) : _values(values) {}

        constexpr bool contains(Enum e) const { return (static_cast<underlying>(e) & _values) != underlying{}; }

        void add(Enum e) { _values |= static_cast<underlying>(e); }

        void flip(Enum e) { _values ^= static_cast<underlying>(e); }

        void remove(Enum e) { _values &= ~static_cast<underlying>(e); }

        constexpr static EnumSet all() {
            return EnumSet(~static_cast<underlying>(0));
        }

        constexpr static EnumSet none() {
            return EnumSet(0);
        }

        constexpr static EnumSet only(Enum e) {
            return EnumSet(e);
        }

        constexpr bool operator&(Enum e) const { return contains(e); }

        friend constexpr bool operator&(Enum e, EnumSet es) {
            return es & e;
        }

        constexpr EnumSet operator|(Enum e) const {
            return EnumSet(_values | static_cast<underlying>(e));
        }

        constexpr EnumSet operator|(EnumSet e) const {
            return EnumSet(_values | static_cast<underlying>(e));
        }

        friend constexpr EnumSet operator|(Enum e, EnumSet es) {
            return es | e;
        }

        constexpr EnumSet operator^(Enum e) const {
            return EnumSet(_values ^ static_cast<underlying>(e));
        }

        explicit constexpr operator underlying () const {
            return _values;
        }

        constexpr bool operator == (const EnumSet& set) const {
            return _values == set._values;
        }

    private:
        underlying _values;
    };

}  //  namespace support
