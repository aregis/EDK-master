/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_LIGHTSCRIPT_ACTION_H_
#define HUESTREAM_EFFECT_LIGHTSCRIPT_ACTION_H_

#include "huestream/effect/effects/base/AnimationEffect.h"
#include "huestream/common/data/Color.h"
#include "huestream/common/data/Light.h"
#include "huestream/effect/animation/ActionPlayer.h"

#include <memory>
#include <string>

namespace huestream {

/**
 wrapper around an huestream::AnimationEffect which adds a start position and optional end position such that it can be automatically enabled(/disabled) at a those times
 */
class Action : public Effect {
 /**
  set the start position in milliseconds of this action
  */
 PROP_DEFINE(Action, int64_t, startPosition, StartPosition);

 /**
  set the end position in milliseconds of this action, -1 means it uses the end position of the underlying AnimationEffect
  */
 PROP_DEFINE(Action, int64_t, endPosition, EndPosition);

 /**
  set the effect which is enabled at the start position (and optionally disabled at the end position)
  */
 PROP_DEFINE(Action, AnimationEffectPtr, effect, Effect);

 protected:
    std::shared_ptr<ActionPlayer> _actionplayer;
    TimeProviderPtr _timeProvider;

    bool IsBetweenStartAndEnd(int64_t position) const;

    void OnUpdateEffect();

public:
    static constexpr const char* type = "huestream.Action";

    Action();

    /**
     constructor
     @see Effect()
     @param effect Effect which is triggered at the start position
     @param startPosition Position in milliseconds at which the effect is enabled
     @param endPosition Position in milliseconds at which the effect is disabled, -1 means it uses the end position of the effect itself
     */
    Action(std::string name, unsigned int layer, AnimationEffectPtr effect, int64_t startPosition,
           int64_t endPosition = -1);

    /**
     destructor
     */
    virtual ~Action() {}

    /**
     set object which provides the time position
     */
    void SetTimeProvider(TimeProviderPtr timeProvider);

    /**
     get the calculated end position of this action based on the set end position and duration of the underlying effect (whichever ends first)
     */
    int64_t GetCalculatedEndPosition() const;

    Color GetColor(LightPtr light) override;

    void UpdateGroup(GroupPtr group) override;

    void Render() override;

    bool IsEnabled() const override;

    std::string GetTypeName() const override;

    void Serialize(JSONNode *node) const override;

    void Deserialize(JSONNode *node) override;
};

/**
 shared pointer to an huestream::Action object
 */
SMART_POINTER_TYPES_FOR(Action)

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_LIGHTSCRIPT_ACTION_H_
