/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_SEQUENCEANIMATION_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_SEQUENCEANIMATION_H_

#include "huestream/effect/animation/animations/base/TriggerableAnimation.h"

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <utility>

namespace huestream {

    typedef std::shared_ptr<Animation> AnimationPtr;
    typedef std::vector<AnimationPtr> AnimationList;
    typedef std::shared_ptr<AnimationList> AnimationListPtr;

    typedef std::map<std::string, int> BookMarkToIndexList;

    /**
     animation which is a sequence of any type of animations 
     */
    class SequenceAnimation : public TriggerableAnimation {
    public:
        static constexpr const char* type = "huestream.SequenceAnimation";
        static HUESTREAM_EXPORT std::string AttributeBookmarks;
    PROP_DEFINE(SequenceAnimation, AnimationListPtr, sequences, Sequences);

    private:
        BookMarkToIndexList bookmarks_;
        size_t startIndex_;
        int currentIndex_;

    public:
        AnimationPtr Clone() override;

        /**
         constructor
         @see repeatableanimation()
         */
        explicit SequenceAnimation(double repeatTimes = 0);

        /**
         destructor
         */
        virtual ~SequenceAnimation() {}

        void UpdateValue(double *value, double positionMs) override;

        void UpdateSequence(int index, double startValueMarker);

        double GetLengthMs() const override;

        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        /**
         append an animation to this sequence of animations
         */
        int Append(AnimationPtr animation, std::string bookmark = "");

        int get_num_sequences() const;

        int get_num_triggers() const;

        std::pair<bool, int> get_trigger_index(std::string bookmark);

        void TriggerOnLevel(const std::string& bookmark) override;

        void Trigger(int index);

        void Trigger(const std::string& bookmark) override;

        bool IsPlaying() const override;

        void Rewind() override;
    };

    SMART_POINTER_TYPES_FOR(SequenceAnimation);

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_SEQUENCEANIMATION_H_
