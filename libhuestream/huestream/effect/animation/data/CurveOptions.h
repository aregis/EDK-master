/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_DATA_CURVEOPTIONS_H_
#define HUESTREAM_EFFECT_ANIMATION_DATA_CURVEOPTIONS_H_

#include "huestream/common/serialize/Serializable.h"

#include <string>

namespace huestream {

    template<typename T>
    class Nullable {
    private:
        T value_;
        bool hasValue_;

    public:
        Nullable() : value_(0), hasValue_(false) {}

        explicit Nullable(T value) : value_(value), hasValue_(true) {}

        bool has_value() const { return hasValue_; }

        void clear_value() {
            hasValue_ = false;
            value_ = 0;
        }

        T get_value() const { return value_; }

        void set_value(T value) {
            hasValue_ = true;
            value_ = value;
        }
    };

    class CurveOptions : public Serializable {
    public:
        static constexpr const char* type = "huestream.CurveOptions";

    PROP_DEFINE(CurveOptions, double, multiplyFactor, MultiplyFactor);
    PROP_DEFINE(CurveOptions, Nullable<double>, clipMin, ClipMin);
    PROP_DEFINE(CurveOptions, Nullable<double>, clipMax, ClipMax);

    public:
        CurveOptions(double _multiplyFactor = 1, const Nullable<double> _clipMin = Nullable<double>(),
                     const Nullable<double> _clipMax = Nullable<double>());

        virtual ~CurveOptions() {}

        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_DATA_CURVEOPTIONS_H_
