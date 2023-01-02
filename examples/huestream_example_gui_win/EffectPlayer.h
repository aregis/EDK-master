/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <huestream/HueStream.h>
#include <huestream_example_gui_win/BrightnessSawEffect.h>
#include <huestream_example_gui_win/BrightnessSquareWaveEffect.h>
#include <huestream_example_gui_win/ColorloopEffect.h>
#include <huestream_example_gui_win/LightpointMarqueeEffect.h>
#include <huestream_example_gui_win/RGBSineEffect.h>
#include <huestream_example_gui_win/FireballsEffect.h>

#include <memory>

using huestream::HueStreamPtr;

class EffectPlayer;

typedef shared_ptr<EffectPlayer> EffectPlayerPtr;

class EffectPlayer {
public:
    explicit EffectPlayer(HueStreamPtr huestream);

    void Start();

    void Stop();

    void SetSpeed(double speed);

    void PlayExplosion();

    void PlayRedWhiteBlue();

    void PlayCombi();

    void PlaySequenceRound();

    void PlaySequenceLtoR();

    void PlaySequenceFtoB();

    void SetSawEnabled(bool enabled);

    void SetSquareEnabled(bool enabled);

    void SetSineEnabled(bool enabled);

    void SetMarqueeEnabled(bool enabled);

    void SetColorloopEnabled(bool enabled);

    void SetFireballsEnabled(bool enabled);

    void SetPoliceEnabled(bool enabled);

    std::shared_ptr<huestream::ConstantAnimation> m_alpha_saw;
    std::shared_ptr<huestream::ConstantAnimation> m_alpha_square;
    std::shared_ptr<huestream::ConstantAnimation> m_alpha_sine;
    std::shared_ptr<huestream::ConstantAnimation> m_alpha_colorloop;
    std::shared_ptr<huestream::ConstantAnimation> m_alpha_marquee;
    std::shared_ptr<huestream::ConstantAnimation> m_alpha_fireballs;
    std::shared_ptr<huestream::ConstantAnimation> m_alpha_police;

protected:
    HueStreamPtr m_hueStream;
    CCriticalSection m_cs;

    double GetSpeed();

    bool IsRunning();

    static UINT SequenceRound(LPVOID pParam);

    static UINT SequenceLtoR(LPVOID pParam);

    static UINT SequenceFtoB(LPVOID pParam);

private:
    bool m_running;
    double m_speed;
    std::condition_variable m_cv;
    std::mutex m_cv_mutex;
    std::shared_ptr<huestream::ConstantAnimation> m_speedAnimation;
    std::shared_ptr<huestream::BrightnessSawEffect> m_saw;
    std::shared_ptr<huestream::BrightnessSquareWaveEffect> m_square;
    std::shared_ptr<huestream::RGBSineEffect> m_sine;
    std::shared_ptr<huestream::LightpointMarqueeEffect> m_marquee;
    std::shared_ptr<huestream::ColorloopEffect> m_colorloop;
    std::shared_ptr<huestream::FireballsEffect> m_fireballs;
    std::shared_ptr<huestream::AreaEffect> m_police_left;
    std::shared_ptr<huestream::AreaEffect> m_police_right;
};

