/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream_example_gui_win/stdafx.h>

#pragma comment(lib, "winmm.lib")

#include <huestream/huestream.h>

#include <huestream/effect/effects/ExplosionEffect.h>
#include <huestream/effect/effects/SequenceEffect.h>
#include <huestream/effect/animation/animations/RandomAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>

#include <huestream_example_gui_win/EffectPlayer.h>
#include <huestream/effect/animation/data/Channel.h>
#include <huestream/effect/effects/MultiChannelEffect.h>

#include <math.h>
#include <mmsystem.h>
#include <string>
#include <memory>


using std::string;
using std::make_shared;

using huestream::Color;
using huestream::Location;
using huestream::HueStream;
using huestream::ConstantAnimation;
using huestream::Point;
using huestream::ExplosionEffect;
using huestream::SequenceAnimation;
using huestream::TweenAnimation;
using huestream::TweenType;
using huestream::Channel;
using huestream::MultiChannelEffect;
using huestream::AreaEffect;
using huestream::Area;
using huestream::LightSourceEffect;
using huestream::RandomAnimation;
using huestream::SequenceEffect;
using huestream::BrightnessSquareWaveEffect;
using huestream::RGBSineEffect;
using huestream::ColorloopEffect;
using huestream::LightpointMarqueeEffect;
using huestream::FireballsEffect;

EffectPlayer::EffectPlayer(HueStreamPtr huestream) {
    m_hueStream = huestream;
    m_speed = 1.0;
    m_running = false;
    m_speedAnimation = make_shared<huestream::ConstantAnimation>(1.0);
    m_alpha_saw = make_shared<huestream::ConstantAnimation>(1);
    m_alpha_square = make_shared<huestream::ConstantAnimation>(1);
    m_alpha_sine = make_shared<huestream::ConstantAnimation>(1);
    m_alpha_marquee = make_shared<huestream::ConstantAnimation>(1);
    m_alpha_colorloop = make_shared<huestream::ConstantAnimation>(1);
    m_alpha_fireballs = make_shared<huestream::ConstantAnimation>(1);
    m_alpha_police = make_shared<huestream::ConstantAnimation>(1);
}

void EffectPlayer::SetSpeed(double speed) {
    m_cs.Lock();
    m_speed = speed;
    m_cs.Unlock();

    m_hueStream->LockMixer();
    m_speedAnimation->SetValue(speed);
    m_hueStream->UnlockMixer();
}

void EffectPlayer::PlayExplosion() {
    Start();

    static auto s_explosionCount = 0;
    std::string name = "BAM!" + std::to_string(s_explosionCount++);

    m_hueStream->LockMixer();

    auto explosion = std::make_shared<ExplosionEffect>(name, 9);
    auto color = Color(1.0, 0.7, 0.3);
    auto radius = 2.0;
    auto speed = GetSpeed();
    auto expAlpha = 150 / speed;
    auto expRadius = 250 / speed;
    auto duration = 1500 / speed;
    auto location = Location(0.0, 0.0);
    explosion->PrepareEffect(color, location, duration, radius, expAlpha, expRadius);
    m_hueStream->AddEffect(explosion);
    explosion->Enable();

    m_hueStream->UnlockMixer();
    PlaySoundA((LPCSTR) "explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void EffectPlayer::PlayRedWhiteBlue() {
    Start();

    static auto s_rwbCount = 0;
    std::string name = "rwb" + std::to_string(s_rwbCount++);

    m_hueStream->LockMixer();

    auto animation = make_shared<SequenceAnimation>(0);
    auto slowUp = make_shared<TweenAnimation>(0, 1, 2500, TweenType::Linear);
    auto fastDown = make_shared<TweenAnimation>(1, 0.5, 500, TweenType::Linear);
    auto fastUp = make_shared<TweenAnimation>(0.5, 1, 500, TweenType::Linear);
    auto fastDownUpRepeated = make_shared<SequenceAnimation>(3);
    fastDownUpRepeated->Append(fastDown, "FastDown");
    fastDownUpRepeated->Append(fastUp, "FastUp");
    auto slowDown = make_shared<TweenAnimation>(1, 0, 2500, TweenType::Linear);
    animation->Append(slowUp, "SlowUp");
    animation->Append(fastDownUpRepeated, "FastDownUp");
    animation->Append(slowDown, "SlowDown");

    auto zero = make_shared<ConstantAnimation>(0);
    auto one = make_shared<ConstantAnimation>(1);

    auto redChannel = make_shared<Channel>(Location(-(2.0 / 3.0), 0), animation, zero, zero, one);
    auto whiteChannel = make_shared<Channel>(Location(0, 0), animation, animation, animation, one);
    auto blueChannel = make_shared<Channel>(Location((2.0 / 3.0), 0), zero, zero, animation, one);

    auto rwbEffect = std::make_shared<MultiChannelEffect>(name, 0);
    rwbEffect->AddChannel(redChannel);
    rwbEffect->AddChannel(whiteChannel);
    rwbEffect->AddChannel(blueChannel);
    rwbEffect->SetSpeedAnimation(m_speedAnimation);
    m_hueStream->AddEffect(rwbEffect);
    rwbEffect->Enable();

    m_hueStream->UnlockMixer();
}

void EffectPlayer::PlayCombi() {
    Start();

    static auto s_combiCount = 0;
    std::string bcName = "Saw" + std::to_string(s_combiCount++);
    std::string lsName = "RandomFlash" + std::to_string(s_combiCount);

    m_hueStream->LockMixer();

    auto bcEffect = std::make_shared<AreaEffect>(bcName, 0);

    auto b = make_shared<SequenceAnimation>(4);
    b->Append(make_shared<TweenAnimation>(0, 1, 1000, TweenType::Linear), "Up");
    b->Append(make_shared<TweenAnimation>(1, 0, 1000, TweenType::Linear), "Down");

    auto g = make_shared<ConstantAnimation>(0);
    auto r = make_shared<ConstantAnimation>(0);

    bcEffect->SetColorAnimation(r, g, b);
    bcEffect->AddArea(Area::All);
    bcEffect->SetSpeedAnimation(m_speedAnimation);

    auto lsEffect = std::make_shared<LightSourceEffect>(lsName, 1);
    auto length = 8000;
    auto i = make_shared<RandomAnimation>(0.1, 1, 200, 400, TweenType::Linear, length);

    lsEffect->SetColorAnimation(i, i, i);
    lsEffect->SetRadiusAnimation(i);

    auto x = make_shared<RandomAnimation>(-1, 1, 500, 1000, TweenType::Linear, length);
    auto y = make_shared<RandomAnimation>(-1, 1, 500, 1000, TweenType::Linear, length);
    lsEffect->SetPositionAnimation(x, y);
    lsEffect->SetOpacityAnimation(make_shared<ConstantAnimation>(0.8));
    lsEffect->SetSpeedAnimation(m_speedAnimation);

    m_hueStream->AddEffect(bcEffect);
    m_hueStream->AddEffect(lsEffect);
    bcEffect->Enable();
    lsEffect->Enable();

    m_hueStream->UnlockMixer();
}

void EffectPlayer::PlaySequenceRound() {
    Start();
    AfxBeginThread(SequenceRound, (LPVOID)this);
}

void EffectPlayer::PlaySequenceLtoR() {
    Start();
    AfxBeginThread(SequenceLtoR, (LPVOID)this);
}

void EffectPlayer::PlaySequenceFtoB() {
    Start();
    AfxBeginThread(SequenceFtoB, (LPVOID)this);
}

UINT EffectPlayer::SequenceRound(LPVOID pParam) {
    auto *self = static_cast<EffectPlayer *>(pParam);
    auto hueStream = self->m_hueStream;

    static auto s_roundeffectCount = 0;
    std::string name = "RoundSeq" + std::to_string(s_roundeffectCount++);

    hueStream->LockMixer();

    auto effect = std::make_shared<SequenceEffect>(name, 1);
    effect->SetMode(huestream::SequenceModeRound);
    effect->AddColor(Color(1.0, 1.0, 0.0));
    effect->AddColor(Color(0.0, 1.0, 1.0));
    effect->AddColor(Color(1.0, 0.0, 1.0));
    effect->AddColor(Color(1.0, 0.0, 0.0));
    effect->AddColor(Color(0.0, 1.0, 0.0));
    effect->SetBri(10.0);

    hueStream->AddEffect(effect);
    effect->Enable();

    hueStream->UnlockMixer();

    for (int i = 0; i < 30; i++) {
        hueStream->LockMixer();
        effect->Step();
        hueStream->UnlockMixer();

        auto waitTime = static_cast<int>(round(500.0 / self->GetSpeed()));
        std::unique_lock<std::mutex> lk(self->m_cv_mutex);
        self->m_cv.wait_until(lk, std::chrono::system_clock::now() + std::chrono::milliseconds(waitTime));
        if (!self->IsRunning()) {
            break;
        }
    }

    hueStream->LockMixer();
    effect->Disable();
    hueStream->UnlockMixer();

    return 0;
}

UINT EffectPlayer::SequenceLtoR(LPVOID pParam) {
    auto *self = static_cast<EffectPlayer *>(pParam);
    auto hueStream = self->m_hueStream;

    static auto s_leftRightSeqCount = 0;
    std::string name = "LeftRightSeq" + std::to_string(s_leftRightSeqCount++);

    hueStream->LockMixer();

    auto effect = std::make_shared<SequenceEffect>(name, 1);
    effect->SetMode(huestream::SequenceModeLeftRight);
    effect->AddColor(Color(1.0, 1.0, 0.0));
    effect->AddColor(Color(0.0, 1.0, 1.0));
    effect->AddColor(Color(1.0, 0.0, 1.0));
    effect->AddColor(Color(1.0, 0.0, 0.0));
    effect->AddColor(Color(0.0, 1.0, 0.0));
    hueStream->AddEffect(effect);
    effect->Enable();

    hueStream->UnlockMixer();

    for (auto i = 0; i < 30; i++) {
        hueStream->LockMixer();
        effect->Step();
        hueStream->UnlockMixer();

        auto waitTime = static_cast<int>(round(500.0 / self->GetSpeed()));
        std::unique_lock<std::mutex> lk(self->m_cv_mutex);
        self->m_cv.wait_until(lk, std::chrono::system_clock::now() + std::chrono::milliseconds(waitTime));
        if (!self->IsRunning()) {
            break;
        }
    }

    hueStream->LockMixer();
    effect->Disable();
    hueStream->UnlockMixer();

    return 0;
}

UINT EffectPlayer::SequenceFtoB(LPVOID pParam) {
    auto *self = static_cast<EffectPlayer *>(pParam);
    auto hueStream = self->m_hueStream;

    static auto s_frontbackSeqCount = 0;
    std::string name = "frontbackSeq" + std::to_string(s_frontbackSeqCount++);

    hueStream->LockMixer();

    auto effect = std::make_shared<SequenceEffect>(name, 1);
    effect->SetMode(huestream::SequenceModeFrontBack);
    effect->AddColor(Color(1.0, 0.0, 0.0));
    effect->AddColor(Color(0.0, 1.0, 0.0));
    effect->AddColor(Color(0.0, 0.0, 1.0));
    hueStream->AddEffect(effect);
    effect->Enable();

    hueStream->UnlockMixer();

    for (auto i = 0; i < 30; i++) {
        hueStream->LockMixer();
        effect->Step();
        hueStream->UnlockMixer();

        auto waitTime = static_cast<int>(round(500.0 / self->GetSpeed()));
        std::unique_lock<std::mutex> lk(self->m_cv_mutex);
        self->m_cv.wait_until(lk, std::chrono::system_clock::now() + std::chrono::milliseconds(waitTime));
        if (!self->IsRunning()) {
            break;
        }
    }

    hueStream->LockMixer();
    effect->Disable();
    hueStream->UnlockMixer();

    return 0;
}


void EffectPlayer::SetSawEnabled(bool enabled) {
    if (enabled) {
        Start();
        m_hueStream->LockMixer();
        if (m_saw == nullptr) {
            m_saw = std::make_shared<huestream::BrightnessSawEffect>("Saw", 8);
            m_saw->PrepareEffect(m_speedAnimation, m_alpha_saw);
            m_hueStream->AddEffect(m_saw);
        }
        m_saw->Enable();
        m_hueStream->UnlockMixer();
    } else if (m_saw != nullptr) {
        m_hueStream->LockMixer();
        m_saw->Disable();
        m_hueStream->UnlockMixer();
    }
}

void EffectPlayer::SetSquareEnabled(bool enabled) {
    if (enabled) {
        Start();
        m_hueStream->LockMixer();
        if (m_square == nullptr) {
            m_square = std::make_shared<BrightnessSquareWaveEffect>("Square", 7);
            m_square->PrepareEffect(m_speedAnimation, m_alpha_square);
            m_hueStream->AddEffect(m_square);
        }
        m_square->Enable();
        m_hueStream->UnlockMixer();
    } else if (m_square != nullptr) {
        m_hueStream->LockMixer();
        m_square->Disable();
        m_hueStream->UnlockMixer();
    }
}

void EffectPlayer::SetSineEnabled(bool enabled) {
    if (enabled) {
        Start();
        m_hueStream->LockMixer();
        if (m_sine == nullptr) {
            m_sine = std::make_shared<RGBSineEffect>("Sine", 6);
            m_sine->PrepareEffect(m_speedAnimation, m_alpha_sine);
            m_hueStream->AddEffect(m_sine);
        }
        m_sine->Enable();
        m_hueStream->UnlockMixer();
    } else if (m_sine != nullptr) {
        m_hueStream->LockMixer();
        m_sine->Disable();
        m_hueStream->UnlockMixer();
    }
}

void EffectPlayer::SetColorloopEnabled(bool enabled) {
    if (enabled) {
        Start();
        m_hueStream->LockMixer();
        if (m_colorloop == nullptr) {
            m_colorloop = std::make_shared<ColorloopEffect>("Colorloop", 5);
            m_colorloop->PrepareEffect(m_speedAnimation, m_alpha_colorloop);
            m_hueStream->AddEffect(m_colorloop);
        }
        m_colorloop->Enable();
        m_hueStream->UnlockMixer();
    } else if (m_colorloop != nullptr) {
        m_hueStream->LockMixer();
        m_colorloop->Disable();
        m_hueStream->UnlockMixer();
    }
}

void EffectPlayer::SetMarqueeEnabled(bool enabled) {
    if (enabled) {
        Start();
        m_hueStream->LockMixer();
        if (m_marquee == nullptr) {
            m_marquee = std::make_shared<LightpointMarqueeEffect>("Marquee", 4);
            m_marquee->PrepareEffect(m_speedAnimation, m_alpha_marquee);
            m_hueStream->AddEffect(m_marquee);
        }
        m_marquee->Enable();
        m_hueStream->UnlockMixer();
    } else if (m_marquee != nullptr) {
        m_hueStream->LockMixer();
        m_marquee->Disable();
        m_hueStream->UnlockMixer();
    }
}

void EffectPlayer::SetFireballsEnabled(bool enabled) {
    if (enabled) {
        Start();
        m_hueStream->LockMixer();
        if (m_fireballs == nullptr) {
            m_fireballs = std::make_shared<FireballsEffect>("Fireballs", 3);
            m_fireballs->PrepareEffect(m_speedAnimation, m_alpha_fireballs);
            m_hueStream->AddEffect(m_fireballs);
        }
        m_fireballs->Enable();
        m_hueStream->UnlockMixer();
    } else if (m_fireballs != nullptr) {
        m_hueStream->LockMixer();
        m_fireballs->Disable();
        m_hueStream->UnlockMixer();
    }
}

void EffectPlayer::SetPoliceEnabled(bool enabled) {
    if (enabled) {
        Start();
        m_hueStream->LockMixer();
        if (m_police_left == nullptr) {
            m_police_left = std::make_shared<AreaEffect>("PoliceLeft", 2);
            m_police_left->AddArea(Area::LeftHalf);
            auto animationLeft = std::make_shared<SequenceAnimation>(huestream::INF);
            animationLeft->Append(std::make_shared<TweenAnimation>(0.2, 1, 160, TweenType::Linear));
            animationLeft->Append(std::make_shared<TweenAnimation>(1, 1, 160, TweenType::Linear));
            animationLeft->Append(std::make_shared<TweenAnimation>(1, 0.2, 160, TweenType::Linear));
            animationLeft->Append(std::make_shared<TweenAnimation>(0.2, 0.1, 160, TweenType::Linear));
            m_police_left->SetColorAnimation(animationLeft, std::make_shared<ConstantAnimation>(0),
                                             std::make_shared<ConstantAnimation>(0));
            m_police_left->SetOpacityAnimation(m_alpha_fireballs);
            m_police_left->SetSpeedAnimation(m_speedAnimation);
            m_hueStream->AddEffect(m_police_left);
        }
        if (m_police_right == nullptr) {
            m_police_right = std::make_shared<AreaEffect>("PoliceRight", 2);
            m_police_right->AddArea(Area::RightHalf);
            auto animationRight = std::make_shared<SequenceAnimation>(huestream::INF);
            animationRight->Append(std::make_shared<TweenAnimation>(1, 0.2, 160, TweenType::Linear));
            animationRight->Append(std::make_shared<TweenAnimation>(0.2, 0.1, 160, TweenType::Linear));
            animationRight->Append(std::make_shared<TweenAnimation>(0.2, 1, 160, TweenType::Linear));
            animationRight->Append(std::make_shared<TweenAnimation>(1, 1, 160, TweenType::Linear));
            m_police_right->SetColorAnimation(std::make_shared<ConstantAnimation>(0),
                                              std::make_shared<ConstantAnimation>(0), animationRight);
            m_police_left->SetOpacityAnimation(m_alpha_fireballs);
            m_police_right->SetSpeedAnimation(m_speedAnimation);
            m_hueStream->AddEffect(m_police_right);
        }
        m_police_left->Enable();
        m_police_right->Enable();
        m_hueStream->UnlockMixer();
    } else if (m_police_left != nullptr && m_police_right != nullptr) {
        m_hueStream->LockMixer();
        m_police_left->Disable();
        m_police_right->Disable();
        m_hueStream->UnlockMixer();
    }
}


double EffectPlayer::GetSpeed() {
    m_cs.Lock();
    auto speed = m_speed;
    m_cs.Unlock();
    return speed;
}

void EffectPlayer::Start() {
    m_cs.Lock();
    m_running = true;
    m_cs.Unlock();
}

void EffectPlayer::Stop() {
    m_running = false;
    m_cv.notify_all();
    m_marquee = nullptr;
}

bool EffectPlayer::IsRunning() {
    m_cs.Lock();
    auto running = m_running;
    m_cs.Unlock();
    return running;
}
