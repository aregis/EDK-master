/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/util/Rand.h>

#include <chrono>
#include <random>


static auto seed = std::chrono::system_clock::now().time_since_epoch().count();
static std::default_random_engine generator(static_cast<unsigned int>(seed));
static std::uniform_real_distribution<double> distribution(0.0, 1.0);

namespace huestream {

double Rand(double min, double max) {
    return distribution(generator) * (max - min) + min;
}
}  // namespace huestream
