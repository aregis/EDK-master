/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_DATA_COLOR_H_
#define HUESTREAM_COMMON_DATA_COLOR_H_

#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/common/serialize/Serializable.h"

#include <stdarg.h>

#include <vector>
#include <string>

namespace huestream {

    /**
     definition of color in RGBA color space
     */
    class Color : public Serializable {
    public:
        static constexpr const char* type = "huestream.Color";


    PROP_DEFINE(Color, double, red, R);
    PROP_DEFINE(Color, double, green, G);
    PROP_DEFINE(Color, double, blue, B);
    PROP_DEFINE(Color, double, alpha, Alpha);

    public:
        Color();

        Color(double red, double green, double blue);

        Color(double red, double green, double blue, double alpha);

        Color(const double xy[2], double brightness, double maxBrightness = 254);

        Color(int ct, double brightness, double maxBrightness);

        std::vector<double> GetRGBA() const;

        void GetYxy(double &Y, double &x, double &y) const;

        void GetRawRGB(double& _r, double& _g, double& _b) const;

        double GetCurrentBrightness() const;

        void ApplyBrightness(double value);

        void Clamp();

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        std::string GetTypeName() const override;

        bool operator==(const Color& color) const;

    private:
        double Clamp(double value);
        void SetXY(const double xy[2], double brightness, double maxBrightness = 254);

        double _rawRed;
        double _rawGreen;
        double _rawBlue;
    };

#if defined(GNU) && (GCC_MAJOR < 5) || defined(__clang__)

    class Colors : public std::vector<Color> {
    public:
        static Colors Create(int number, Color p1) {
            (void) number;
            auto list = Colors();
            list.push_back(p1);
            return list;
        }

        static Colors Create(int number, Color p1, Color p2) {
            (void) number;
            auto list = Colors();
            list.push_back(p1);
            list.push_back(p2);
            return list;
        }

        static Colors Create(int number, Color p1, Color p2, Color p3) {
            (void) number;
            auto list = Colors();
            list.push_back(p1);
            list.push_back(p2);
            list.push_back(p3);
            return list;
        }

        static Colors Create(int number, Color p1, Color p2, Color p3, Color p4) {
            (void) number;
            auto list = Colors();
            list.push_back(p1);
            list.push_back(p2);
            list.push_back(p3);
            list.push_back(p4);
            return list;
        }

        static Colors Create(int number, Color p1, Color p2, Color p3, Color p4, Color p5) {
            (void) number;
            auto list = Colors();
            list.push_back(p1);
            list.push_back(p2);
            list.push_back(p3);
            list.push_back(p4);
            list.push_back(p5);
            return list;
        }

        static Colors Create(int number, Color p1, Color p2, Color p3, Color p4, Color p5, Color p6) {
            (void) number;
            auto list = Colors();
            list.push_back(p1);
            list.push_back(p2);
            list.push_back(p3);
            list.push_back(p4);
            list.push_back(p5);
            list.push_back(p6);
            return list;
        }

        static Colors Create(int number, Color p1, Color p2, Color p3, Color p4, Color p5, Color p6, Color p7) {
            (void) number;
            auto list = Colors();
            list.push_back(p1);
            list.push_back(p2);
            list.push_back(p3);
            list.push_back(p4);
            list.push_back(p5);
            list.push_back(p6);
            list.push_back(p7);
            return list;
        }
    };

#else
    class Colors : public std::vector<Color> {
    public:
        static Colors Create(int number, ...) {
            auto colors = Colors();
            va_list ap;
            va_start(ap, number);
            for (auto j = 0; j < number; j++) {
                Color c = va_arg(ap, Color);
                colors.push_back(c);
            }
            va_end(ap);
            return colors;
        }
    };
#endif

}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_COLOR_H_
