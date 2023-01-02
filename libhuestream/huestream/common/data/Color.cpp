/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/Color.h>

#include <algorithm>
#include <string>
#include <vector>
#include <cmath>

namespace huestream {

PROP_IMPL(Color, double, red, R);
PROP_IMPL(Color, double, green, G);
PROP_IMPL(Color, double, blue, B);
PROP_IMPL(Color, double, alpha, Alpha);


Color::Color() : _red(0), _green(0), _blue(0), _alpha(0), _rawRed(0), _rawGreen(0), _rawBlue(0) {}

Color::Color(double red, double green, double blue)
    : _red(red), _green(green), _blue(blue), _alpha(1), _rawRed(red), _rawGreen(green), _rawBlue(blue) {}

Color::Color(double red, double green, double blue, double alpha)
    : _red(red), _green(green), _blue(blue), _alpha(alpha), _rawRed(red), _rawGreen(green), _rawBlue(blue) {}

Color::Color(const double xy[2], double brightness, double maxBrightness) : _alpha(1) {
    SetXY(xy, brightness, maxBrightness);
}

Color::Color(int ct, double brightness, double maxBrightness) : _alpha(1){
    // Very basic support of 5 color points only, so choose the closest one.
    if (ct <= 196) {
        double mirek_153_to_xy[2] = { 0.312987362, 0.323103173 };
        SetXY(mirek_153_to_xy, brightness, maxBrightness);
    }
    else if (ct <= 279) {
        double mirek_239_to_xy[2] = { 0.372681144, 0.371764524 };
        SetXY(mirek_239_to_xy, brightness, maxBrightness);
    }
    else if (ct <= 365)
    {
        double mirek_325_to_xy[2] = { 0.431628222, 0.402182599 };
        SetXY(mirek_325_to_xy, brightness, maxBrightness);
    }
    else if (ct <= 454)
    {
        double mirek_411_to_xy[2] = { 0.483089605, 0.414386536 };
        SetXY(mirek_411_to_xy, brightness, maxBrightness);
    }
    else {
        double mirek_500_to_xy[2] = { 0.526685851, 0.413295716 };
        SetXY(mirek_500_to_xy, brightness, maxBrightness);
    }
}

std::vector<double> Color::GetRGBA() const {
    auto v = std::vector<double>();
    v.push_back(_red);
    v.push_back(_green);
    v.push_back(_blue);
    v.push_back(_alpha);

    return v;
}

void Color::GetYxy(double &Y, double &x, double &y) const {
    double r = _rawRed;
    double g = _rawGreen;
    double b = _rawBlue;

    if (r == 0.0 && g == 0.0 && b == 0.0)
    {
        Y = x = y = 0.0;
        return;
    }

    r = (r > 0.04045) ? pow(((r + 0.055) / 1.055), 2.4) : r / 12.92;
    g = (g > 0.04045) ? pow(((g + 0.055) / 1.055), 2.4) : g / 12.92;
    b = (b > 0.04045) ? pow(((b + 0.055) / 1.055), 2.4) : b / 12.92;

    r *= 100.0;
    g *= 100.0;
    b *= 100.0;

    double X = r * 0.4124 + g * 0.3576 + b * 0.1805;
    Y = r * 0.2126 + g * 0.7152 + b * 0.0722;
    double Z = r * 0.0193 + g * 0.1192 + b * 0.9505;

    x = X / (X + Y + Z);
    y = Y / (X + Y + Z);
}

void Color::GetRawRGB(double& _r, double& _g, double& _b) const {
    _r = _rawRed;
    _g = _rawGreen;
    _b = _rawBlue;
}

double Color::GetCurrentBrightness() const {
    double max = std::max(std::max(_red, _green), _blue);
    return max;
}

void Color::ApplyBrightness(double value) {
    value = Clamp(value);

    double currentBri = GetCurrentBrightness();

        if (currentBri <= 0.0) {
        return;
    }

    // set to max
    _red /= currentBri;
    _green /= currentBri;
    _blue /= currentBri;

    _red *= value;
    _green *= value;
    _blue *= value;
}

void Color::Clamp() {
    _red = Clamp(_red);
    _green = Clamp(_green);
    _blue = Clamp(_blue);
    _alpha = Clamp(_alpha);
}

void Color::Serialize(JSONNode *node) const {
    Serializable::Serialize(node);
    SerializeValue(node, AttributeR, _red);
    SerializeValue(node, AttributeG, _green);
    SerializeValue(node, AttributeB, _blue);
    SerializeValue(node, AttributeAlpha, _alpha);
}

void Color::Deserialize(JSONNode *node) {
    Serializable::Deserialize(node);
    DeserializeValue(node, AttributeR, &_red, 0);
    DeserializeValue(node, AttributeG, &_green, 0);
    DeserializeValue(node, AttributeB, &_blue, 0);
    DeserializeValue(node, AttributeAlpha, &_alpha, 0);
}

std::string Color::GetTypeName() const {
    return type;
}

double Color::Clamp(double value) {
    if (value > 1) return 1;
    if (value < 0) return 0;
    return value;
}

void Color::SetXY(const double xy[2], double brightness, double maxBrightness)
{
    if (brightness <= 0.0 || xy[1] <= 0.0)
    {
        _red = _green = _blue = _rawRed = _rawGreen = _rawBlue = 0.0;
    }
    else
    {
        double z = 1.0 - xy[0] - xy[1];

        double Y = brightness / maxBrightness;
        double X = (Y / xy[1]) * xy[0];
        double Z = (Y / xy[1]) * z;

        // XYZ to RGB
        _red = X * 3.2406 + Y * -1.5372 + Z * -0.4986;
        _green = X * -0.9689 + Y * 1.8758 + Z * 0.0415;
        _blue = X * 0.0557 + Y * -0.2040 + Z * 1.0570;

        // Gamma correction
        _red = _red > 0.0031308 ? std::pow(_red, 1.0 / 2.4) * 1.055 - 0.055 : _red * 12.92;
        _green = _green > 0.0031308 ? std::pow(_green, 1.0 / 2.4) * 1.055 - 0.055 : _green * 12.92;
        _blue = _blue > 0.0031308 ? std::pow(_blue, 1.0 / 2.4) * 1.055 - 0.055 : _blue * 12.92;

        _rawRed = _red;
        _rawGreen = _green;
        _rawBlue = _blue;

        double max = std::max({ _red, _green, _blue });

        if (max > 1.0)
        {
            _red /= max;
            _green /= max;
            _blue /= max;
        }

        Clamp();
    }
}

bool Color::operator==(const Color& color) const
{
    return std::abs(_red - color._red) <= 0.01 && std::abs(_green - color._green) <= 0.01 && std::abs(_blue - color._blue) <= 0.01 && std::abs(_alpha - color._alpha) <= 0.01;
}

}  // namespace huestream
