//
// Created by wies.hubbers@philips.com on 01/03/16.
//

#ifndef LIBHUESTREAM_TESTSERIALIZEBASE_H
#define LIBHUESTREAM_TESTSERIALIZEBASE_H

#include <vector>
#include <gtest/gtest.h>
#include <huestream/common/serialize/Serializable.h>
#include <huestream/effect/animation/data/TweenType.h>
#include <huestream/effect/animation/data/Point.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>

namespace huestream {

    class TestSerializeBase {

    public:
        void AddBoolAttribute(JSONNode *node, std::string name, bool value);

        void AddDoubleAttribute(JSONNode *node, std::string name, double value);

        void AddStringAttribute(JSONNode *node, std::string name, std::string value);

        void AddPointsAttribute(JSONNode *node, std::string name, PointListPtr points);

        void ASSERT_AttributeIsSetAndDoubleValue(JSONNode node, std::string name, double value);

        void ASSERT_AttributeIsSetAndStringValue(JSONNode node, std::string name, std::string value);

        void ASSERT_TweenAnimationWithBegin(JSONNode node, double begin, double end, double time, TweenType tweenType);

        void ASSERT_TweenAnimationWithoutBegin(JSONNode node, double end, double time, TweenType tweenType);

        void AssertTweenAnimationEndTimeAndTweenTime(JSONNode node, double end, double time, const TweenType &tweenType);

        void ASSERT_RandomAnimationWithLength(JSONNode node, double min, double max, double min_interval,
                                                 double max_interval, TweenType tweenType, double length);

        void ASSERT_RandomAnimationWithoutLength(JSONNode node, double min, double max, double min_interval,
                                                    double max_interval, TweenType tweenType);

        void ASSERT_CurveOptionsAllAttributesSet(JSONNode node, double multiplyFactor, double clipMin, double clipMax);

        void
        ASSERT_AttributeIsSetAndContainPoints(JSONNode node, std::string attribute_name, int start_coordinate_points,
                                              int num_coordinates);

        void AddCurveAttribute(JSONNode *node, std::string name, PointListPtr points);

        void
        ASSERT_AttributeIsSetAndAllCurveOptionsAreSet(JSONNode node, std::string attribute_name, double multiply_factor,
                                                      double clip_min, double clip_max);
    };
}  // namespace huestream

#endif //LIBHUESTREAM_TESTSERIALIZEBASE_H
