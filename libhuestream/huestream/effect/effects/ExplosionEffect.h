/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_EXPLOSIONEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_EXPLOSIONEFFECT_H_

#include "huestream/effect/animation/animations/ConstantAnimation.h"
#include "huestream/effect/effects/LightSourceEffect.h"
#include "huestream/effect/effects/AreaEffect.h"

#include <string>

namespace huestream {

    /**
     this serves as an example of how you could create a specific effect (explosion) from an effect primitive (lightsource)
     */
    class ExplosionEffect : public LightSourceEffect {
        static constexpr const char* type = "huestream.ExplosionEffect";

    public:
        explicit ExplosionEffect(const std::string &name = "", unsigned int layer = 0);
        virtual ~ExplosionEffect() {}

        /**
         configure how the explosion will look
         @param color Color of the explosion
         @param location Position of the center of the explosion
         @param duration Length in milliseconds of the explosion
         @param radius Peak radius of the explosion
         @param radius_expansion_time Time in milliseconds between start and peak radius of the explosion
         @param intensity_expansion_time Time in milliseconds between start and peak intensity of the explosion
         */
        void PrepareEffect(Color color,
                           Location location,
                           double duration,
                           double radius,
                           double radius_expansion_time,
                           double intensity_expansion_time);

        std::string GetTypeName() const override;

    private:
        Color _color;
        Location _location;
        double _radius;
        double _radius_expansion_time_ms;
        double _intensity_expansion_time_ms;
        double _duration;

        void PrepareColor();
        void PrepareRadius();
        void PreparePosition();
        double Clip(double val, double min, double max);
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_EXPLOSIONEFFECT_H_
