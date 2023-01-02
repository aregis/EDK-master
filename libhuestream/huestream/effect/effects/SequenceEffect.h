/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_SEQUENCEEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_SEQUENCEEFFECT_H_

#include "huestream/common/data/Area.h"
#include "huestream/effect/effects/base/Effect.h"

#include <memory>
#include <vector>
#include <string>

namespace huestream {

    enum SequenceMode {
        SequenceModeAll = 0,
        SequenceModeRound,
        SequenceModeFrontBack,
        SequenceModeLeftRight,
        SequenceModeLast
    };


    class SequenceEffect : public Effect {
    protected:
        Color GetCurrentStepColor();

        std::vector<Area> _sequenceList;
        double _bri;
        std::vector<Color> _colors;
        size_t _stepCount;
        SequenceMode _mode;
        GroupPtr _group;

    public:
        explicit SequenceEffect(std::string name = "", unsigned int layer = 0);

        virtual ~SequenceEffect() {}

        static constexpr const char* type = "huestream.SequenceEffect";

        std::string GetTypeName() const override;

        void UpdateGroup(GroupPtr group) override;

        void Render() override;

        virtual void AddColor(Color color);

        virtual void Step();

        virtual void SetBri(double bri);
        double GetBri() const;

        virtual bool AddArea(const Area &area);

        Color GetColor(LightPtr light) override;

        virtual void InceaseBri(double amount);

        virtual void DecreaseBri(double amount);

        virtual void SetMode(SequenceMode mode);
        SequenceMode GetMode() const;
  };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_SEQUENCEEFFECT_H_
