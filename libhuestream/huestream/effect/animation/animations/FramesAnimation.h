/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_FRAMESANIMATION_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_FRAMESANIMATION_H_

#include "huestream/effect/animation/animations/base/RepeatableAnimation.h"

#include <string>

namespace huestream {

    /**
     animation which describes a trajectory by equally spaced frames
     often used for large animation sizes so uses optimized storage
     */
    class FramesAnimation : public RepeatableAnimation {
    public:
        static constexpr const char* type = "huestream.FramesAnimation";

        static HUESTREAM_EXPORT std::string AttributeFps;
        double GetFps() const;
        void SetFps(const double &fps);

        PROP_DEFINE(FramesAnimation, std::shared_ptr<std::vector<uint16_t>>, frames, Frames);
        PROP_DEFINE(FramesAnimation, bool, compressionEnabled, CompressionEnabled);

    public:
        FramesAnimation();

        /**
         constructor
         @param fps Frames per second
         */
        explicit FramesAnimation(double fps);

        /**
         constructor
         @param fps Frames per second
         @param size Number of frames to pre-reserve memory for
         */
        FramesAnimation(double fps, unsigned int size);
        
        ~FramesAnimation() override = default;
        
        AnimationPtr Clone() override;

        void UpdateValue(double *value, double positionMs) override;

        double GetLengthMs() const override;

        /**
         append frame
         */
        void Append(const double frame);

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        std::string GetTypeName() const override;

    private:
        void SerializeFramesCompressed(JSONNode *node) const;
        void SerializeFramesRaw(JSONNode *node) const;
        void AddFramesFrom12BitBase64String(const std::string& frameString);
        const std::string Pack12bitBase64(double number) const;

        double _fpms;
        double _frameLength;
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_FRAMESANIMATION_H_
