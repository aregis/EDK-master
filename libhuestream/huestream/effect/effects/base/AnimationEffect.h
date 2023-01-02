/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_BASE_ANIMATIONEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_BASE_ANIMATIONEFFECT_H_

#include "huestream/effect/effects/base/Effect.h"
#include "huestream/effect/animation/Player.h"
#include "huestream/common/time/ITimeProvider.h"

#include <string>

namespace huestream {

    /**
     abstract base class for an effect which uses animations
     @see doc/EDK_concepts.pdf
     */
    class AnimationEffect : public Effect {
    public:
    PROP_DEFINE(AnimationEffect, AnimationPtr, speed, Speed);

    protected:
        PlayerPtr _player;
        PlayerPtr _speedPlayer;
        AnimationListPtr _animations;

        virtual void RenderUpdate() = 0;

        virtual void InitializeAnimations();

        /**
         constructor
         @see Effect()
         */
        AnimationEffect(std::string name, unsigned int layer);

    public:
        /**
         destructor
         */
        virtual ~AnimationEffect() {}

        /**
         animationEffects override this method to implement which properties are animated
         @return list of animations to play
         */
        virtual AnimationListPtr GetAnimations() = 0;

        /**
         convenience method to set the animation for speed
         @param speed Animation for speed
         */
        virtual void SetSpeedAnimation(AnimationPtr speed);

        /**
         inject player instance which plays the animations
         @note mostly internal use
         @param player Reference to player object
         */
        virtual void SetPlayer(PlayerPtr player);

        /**
         get effect duration
         @return length of effect in milliseconds or INF if endless
         */
        virtual double GetLength();

        /**
         check whether this effect is endless
         @return whether this effect is endless
         */
        virtual bool IsEndless();

        void Enable() override;

        void Disable() override;

        void Render() override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;
    };

    SMART_POINTER_TYPES_FOR(AnimationEffect)

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_BASE_ANIMATIONEFFECT_H_
