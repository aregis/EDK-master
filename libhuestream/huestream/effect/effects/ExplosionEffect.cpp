/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/CurveAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/effect/effects/ExplosionEffect.h>

#include <string>
#include <memory>

namespace huestream {

ExplosionEffect::ExplosionEffect(const std::string &name, unsigned int layer):
    LightSourceEffect(name, layer),
    _radius(0),
    _radius_expansion_time_ms(0),
    _intensity_expansion_time_ms(0),
    _duration(0) {
}

void ExplosionEffect::PrepareRadius() {
    auto radiusAnimation = std::make_shared<SequenceAnimation>();
    radiusAnimation->Append(std::make_shared<TweenAnimation>(0, _radius, _radius_expansion_time_ms, TweenType::Linear));
    radiusAnimation->Append(std::make_shared<TweenAnimation>(_radius, 0, _duration - _radius_expansion_time_ms, TweenType::Linear));
    SetRadiusAnimation(radiusAnimation);
}

void ExplosionEffect::PrepareEffect(Color color,
                                    Location location,
                                    double duration,
                                    double radius,
                                    double radius_expansion_time,
                                    double intensity_expansion_time) {
    _location = location;
    _duration = duration;
    _color = color;
    _radius = radius;
    _radius_expansion_time_ms = Clip(radius_expansion_time, 0, duration);
    _intensity_expansion_time_ms = Clip(intensity_expansion_time, 0 , duration);
    PrepareColor();
    PreparePosition();
    PrepareRadius();
}

void ExplosionEffect::PrepareColor() {
    SetFixedColor(_color);
    SetFixedOpacity(_color.GetAlpha());

    auto maxIntensity = _color.GetCurrentBrightness();
    auto intensity = std::make_shared<SequenceAnimation>();
    intensity->Append(std::make_shared<TweenAnimation>(0, maxIntensity, _intensity_expansion_time_ms, TweenType::Linear));
    intensity->Append(std::make_shared<TweenAnimation>(maxIntensity, 0, _duration - _intensity_expansion_time_ms, TweenType::Linear));
    SetI(intensity);
}


void ExplosionEffect::PreparePosition() {
    SetPositionAnimation(std::make_shared<ConstantAnimation>(_location.GetX()),
                         std::make_shared<ConstantAnimation>(_location.GetY()));
}

double ExplosionEffect::Clip(double val, double min, double max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

std::string ExplosionEffect::GetTypeName() const {
    return type;
}

}  // namespace huestream
