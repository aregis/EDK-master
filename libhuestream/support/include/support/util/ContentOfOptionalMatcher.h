/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "gmock/gmock-matchers.h"

#include <type_traits>

namespace gmock_matcher {

// Implements the ContentOfOptional(m) matcher for matching an optional whose
// value matches matcher m. This is basically a copy paste of gmock's version for pointer types
    template<typename InnerMatcher>
    class ContentOfOptionalMatcher {
    public:
        explicit ContentOfOptionalMatcher(const InnerMatcher &matcher) : matcher_(matcher) {}

        template<typename Optional>
        operator testing::Matcher<Optional>() const {
            return testing::MakeMatcher(new Impl<Optional>(matcher_));
        }

    private:
        // The monomorphic implementation that works for a particular optional type.
        template<typename Optional>
        class Impl : public testing::MatcherInterface<Optional> {
        public:
            using NakedOptional = std::remove_cv_t<std::remove_reference_t<Optional>>;
            using ContainedType = typename NakedOptional::value_type;

            explicit Impl(const InnerMatcher &matcher)
                    : matcher_(testing::MatcherCast<const ContainedType &>(matcher)) {}

            virtual void DescribeTo(::std::ostream *os) const {
                *os << "holds a value that ";
                matcher_.DescribeTo(os);
            }

            virtual void DescribeNegationTo(::std::ostream *os) const {
                *os << "does not hold a value that ";
                matcher_.DescribeTo(os);
            }

            virtual bool MatchAndExplain(Optional o,
                                         testing::MatchResultListener *listener) const {
                if (o == boost::none)
                    return false;

                *listener << "which holds ";
                return testing::internal::MatchPrintAndExplain(o.value(), matcher_, listener);
            }

        private:
            const testing::Matcher<const ContainedType &> matcher_;

            GTEST_DISALLOW_ASSIGN_(Impl);
        };

        const InnerMatcher matcher_;

        GTEST_DISALLOW_ASSIGN_(ContentOfOptionalMatcher);
    };

// Creates a matcher that matches an optional that contains
// a value that matches inner_matcher.
    template<typename InnerMatcher>
    inline ContentOfOptionalMatcher<InnerMatcher> ContentOfOptional(
            const InnerMatcher &inner_matcher) {
        return ContentOfOptionalMatcher<InnerMatcher>(inner_matcher);
    }

}  // namespace gmock_matcher
