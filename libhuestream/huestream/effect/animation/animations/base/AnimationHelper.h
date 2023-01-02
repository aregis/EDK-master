/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_ANIMATIONHELPER_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_ANIMATIONHELPER_H_

#include "huestream/effect/animation/animations/base/Animation.h"
#include <memory>

namespace huestream {

    /**
    Helper class to create list of animations
     */
    class AnimationHelper {
    public:
        static AnimationListPtr CreatePtr(AnimationPtr p1);

        static AnimationListPtr CreatePtr(AnimationPtr p1, AnimationPtr p2);

        static AnimationListPtr CreatePtr(AnimationPtr p1, AnimationPtr p2, AnimationPtr p3, AnimationPtr p4);

        static AnimationListPtr CreatePtr(AnimationPtr p1, AnimationPtr p2, AnimationPtr p3, AnimationPtr p4, AnimationPtr p5);

        static AnimationListPtr CreatePtr(AnimationPtr p1, AnimationPtr p2, AnimationPtr p3, AnimationPtr p4, AnimationPtr p5, AnimationPtr p6, AnimationPtr p7);

        static AnimationListPtr CreatePtr(AnimationPtr p1, AnimationPtr p2, AnimationPtr p3, AnimationPtr p4, AnimationPtr p5, AnimationPtr p6, AnimationPtr p7, AnimationPtr p8);
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_ANIMATIONHELPER_H_
