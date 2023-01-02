//
// Created by wies.hubbers@philips.com on 01/03/16.
//

#include <huestream/effect/animation/animations/RandomAnimation.h>
#include <huestream/effect/animation/data/CurveOptions.h>
#include <huestream/effect/animation/data/Point.h>
#include "TestSerializeBase.h"
#include <huestream/effect/animation/data/CurveData.h>

namespace huestream {
    void TestSerializeBase::AddBoolAttribute(JSONNode *node, std::string name, bool value) {
        node->push_back(JSONNode(name, value));
    }

    void TestSerializeBase::AddDoubleAttribute(JSONNode *node, std::string name, double value) {
        node->push_back(JSONNode(name, value));
    }

    void TestSerializeBase::AddStringAttribute(JSONNode *node, std::string name, std::string value) {
        node->push_back(JSONNode(name, value));
    }

    void TestSerializeBase::ASSERT_AttributeIsSetAndDoubleValue(JSONNode node,
                                                                std::string name,
                                                                double value) {
        ASSERT_TRUE(SerializerHelper::IsAttributeSet(&node, name));
        ASSERT_EQ(node[name].as_float(), value);
    }

    void TestSerializeBase::ASSERT_AttributeIsSetAndStringValue(JSONNode node,
                                                                std::string name,
                                                                std::string value) {
        ASSERT_TRUE(SerializerHelper::IsAttributeSet(&node, name));
        ASSERT_EQ(node[name].as_string(), value);
    }

    void TestSerializeBase::ASSERT_TweenAnimationWithBegin(JSONNode node,
                                                          double begin,
                                                          double end,
                                                          double time,
                                                          TweenType tweenType) {
        ASSERT_AttributeIsSetAndDoubleValue(node, TweenAnimation::AttributeBegin, begin);
        AssertTweenAnimationEndTimeAndTweenTime(node, end, time, tweenType);
    }

    void TestSerializeBase::ASSERT_TweenAnimationWithoutBegin(JSONNode node,
                                                             double end,
                                                             double time,
                                                             TweenType tweenType) {
        ASSERT_FALSE(SerializerHelper::IsAttributeSet(&node, TweenAnimation::AttributeBegin));
        AssertTweenAnimationEndTimeAndTweenTime(node, end, time, tweenType);
    }


    void TestSerializeBase::AssertTweenAnimationEndTimeAndTweenTime(JSONNode node,
                                                                   double end,
                                                                   double time,
                                                                   const TweenType &tweenType) {
        ASSERT_EQ(node[Serializable::AttributeType].as_string(), std::string(TweenAnimation::type));
        TestSerializeBase::ASSERT_AttributeIsSetAndDoubleValue(node, TweenAnimation::AttributeEnd, end);
        TestSerializeBase::ASSERT_AttributeIsSetAndDoubleValue(node, TweenAnimation::AttributeTime, time);
        TestSerializeBase::ASSERT_AttributeIsSetAndStringValue(node,
                                                               TweenAnimation::AttributeTweenType,
                                                               TweenTypeHelper::ToString(tweenType));
    }

    void TestSerializeBase::ASSERT_RandomAnimationWithLength(JSONNode node,
                                                                double min,
                                                                double max,
                                                                double min_interval,
                                                                double max_interval,
                                                                TweenType tweenType,
                                                                double length) {
        ASSERT_AttributeIsSetAndStringValue(node, Serializable::AttributeType, RandomAnimation::type);
        ASSERT_AttributeIsSetAndDoubleValue(node, RandomAnimation::AttributeMinValue, min);
        ASSERT_AttributeIsSetAndDoubleValue(node, RandomAnimation::AttributeMaxValue, max);
        ASSERT_AttributeIsSetAndDoubleValue(node, RandomAnimation::AttributeMinInterval, min_interval);
        ASSERT_AttributeIsSetAndDoubleValue(node, RandomAnimation::AttributeMaxInterval, max_interval);
        ASSERT_AttributeIsSetAndStringValue(node,
                                            RandomAnimation::AttributeTweenType,
                                            TweenTypeHelper::ToString(tweenType));
        ASSERT_AttributeIsSetAndDoubleValue(node, RandomAnimation::AttributeLength, length);
    }

    void TestSerializeBase::ASSERT_RandomAnimationWithoutLength(JSONNode node,
                                                                   double min,
                                                                   double max,
                                                                   double min_interval,
                                                                   double max_interval,
                                                                   TweenType tweenType) {
        ASSERT_AttributeIsSetAndStringValue(node, Serializable::AttributeType, RandomAnimation::type);
        ASSERT_AttributeIsSetAndDoubleValue(node, RandomAnimation::AttributeMinValue, min);
        ASSERT_AttributeIsSetAndDoubleValue(node, RandomAnimation::AttributeMaxValue, max);
        ASSERT_AttributeIsSetAndDoubleValue(node, RandomAnimation::AttributeMinInterval, min_interval);
        ASSERT_AttributeIsSetAndDoubleValue(node, RandomAnimation::AttributeMaxInterval, max_interval);
        ASSERT_AttributeIsSetAndStringValue(node,
                                            RandomAnimation::AttributeTweenType,
                                            TweenTypeHelper::ToString(tweenType));
        ASSERT_FALSE(SerializerHelper::IsAttributeSet(&node, RandomAnimation::AttributeLength));
    }


    void TestSerializeBase::ASSERT_CurveOptionsAllAttributesSet(JSONNode node,
                                                                double multiplyFactor,
                                                                double clipMin,
                                                                double clipMax) {

        ASSERT_AttributeIsSetAndStringValue(node, Serializable::AttributeType, CurveOptions::type);
        ASSERT_AttributeIsSetAndDoubleValue(node, CurveOptions::AttributeMultiplyFactor, multiplyFactor);
        ASSERT_AttributeIsSetAndDoubleValue(node, CurveOptions::AttributeClipMin, clipMin);
        ASSERT_AttributeIsSetAndDoubleValue(node, CurveOptions::AttributeClipMax, clipMax);
    }

    void TestSerializeBase::ASSERT_AttributeIsSetAndAllCurveOptionsAreSet(JSONNode node, std::string attribute_name,
                                                                          double multiply_factor, double clip_min,
                                                                          double clip_max) {
        ASSERT_TRUE(SerializerHelper::IsAttributeSet(&node, attribute_name));
        auto options = node[attribute_name];
        ASSERT_CurveOptionsAllAttributesSet(options, multiply_factor, clip_min, clip_max);

    }

    void TestSerializeBase::ASSERT_AttributeIsSetAndContainPoints(JSONNode node,
                                                                  std::string attribute_name,
                                                                  int start_coordinate_points,
                                                                  int num_coordinates) {
        ASSERT_TRUE(SerializerHelper::IsAttributeSet(&node, attribute_name));
        auto points = node[attribute_name];
        auto count = start_coordinate_points;
        for (auto pointIt = points.begin(); pointIt != points.end(); ++pointIt) {
            auto pointNode = *pointIt;
            ASSERT_AttributeIsSetAndDoubleValue(pointNode, Point::AttributeX, count++);
            ASSERT_AttributeIsSetAndDoubleValue(pointNode, Point::AttributeY, count++);
        }
        ASSERT_EQ(start_coordinate_points + num_coordinates * 2, count);

    }

    void TestSerializeBase::AddCurveAttribute(JSONNode *node, std::string name, PointListPtr points) {

        auto c = CurveData(points);
        JSONNode j;
        c.Serialize(&j);
        j.set_name(name);
        node->push_back(j);

    }


    void TestSerializeBase::AddPointsAttribute(JSONNode *node,
                                               std::string name,
                                               PointListPtr points) {
        JSONNode arrayNode(JSON_ARRAY);
        arrayNode.set_name(name);
        for (auto i = 0; i < static_cast<int>(points->size()); ++i) {
            JSONNode v;
            points->at(i)->Serialize(&v);
            arrayNode.push_back(v);
        }
        node->push_back(arrayNode);
    }
}  // namespace huestream
