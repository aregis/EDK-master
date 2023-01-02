/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#include "gtest/gtest.h"
#include <memory>
#include <huestream/common/serialize/Serializable.h>
#include "huestream/config/ObjectBuilder.h"

namespace huestream {

    class A : public Serializable {
    public:
        static std::string type;

        int GetSomeData() const {
            return some_data_;
        }

        void SetSomeData(int some_data) {
            some_data_ = some_data;
        }

        virtual std::string GetTypeName() const override { return A::type; }

        virtual void Deserialize(JSONNode *node) override {
            Serializable::Deserialize(node);
            some_data_ = static_cast<int>((*node)["some_data"].as_int());
        }

        void Serialize(JSONNode *node) const override {
            huestream::Serializable::Serialize(node);
            node->push_back(JSONNode("some_data", some_data_));
        }

    private:
        int some_data_;
    };

    std::string A::type = "huestream::A";

    class B : public A {
    public:
        static std::string type;

        int GetOtherData() const {
            return other_data_;
        }

        void SetOtherData(int other_data) {
            other_data_ = other_data;
        }

        virtual std::string GetTypeName() const override { return B::type; }

        virtual void Deserialize(JSONNode *node) override {
            A::Deserialize(node);
            other_data_ = static_cast<int>((*node)["other_data"].as_int());
        }

        void Serialize(JSONNode *node) const override {
            A::Serialize(node);
            node->push_back(JSONNode("other_data", other_data_));
        }

    private:
        int other_data_;
    };

    std::string B::type = "huestream::B";


    class C : public Serializable {
    public:
        static std::string type;

        std::shared_ptr<B> GetB() const {
            return b_;
        }

        void SetB(std::shared_ptr<B> b) {
            b_ = b;
        }

        virtual std::string GetTypeName() const override { return C::type; }

        virtual void Deserialize(JSONNode *node) override {
            Serializable::Deserialize(node);
            auto jsonB = (*node)["b"];
            auto pB = std::static_pointer_cast<B>(Serializable::DeserializeFromJson(&jsonB));
            b_ = std::shared_ptr<B>(pB);
        }

        void Serialize(JSONNode *node) const override {
            Serializable::Serialize(node);
            JSONNode jsonB;
            jsonB.set_name("b");
            b_->Serialize(&jsonB);
            node->push_back(jsonB);
        }

    private:
        std::shared_ptr<B> b_;
    };

    std::string C::type = "huestream::C";

    class TestObjectBuilder : public ObjectBuilderBase {

    public:
        virtual std::shared_ptr<Serializable> ConstructInstanceOf(std::string type);
    };

    std::shared_ptr<Serializable> TestObjectBuilder::ConstructInstanceOf(std::string type) {

        if (type == A::type)
            return std::make_shared<A>();
        if (type == B::type)
            return std::make_shared<B>();
        if (type == C::type)
            return std::make_shared<C>();

        return nullptr;
    }

    class TestSerializable : public testing::Test {
        virtual void SetUp() {
            Serializable::SetObjectBuilder(std::make_shared<TestObjectBuilder>());
        }

        virtual void TearDown() {
            Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        }
    };

    TEST_F(TestSerializable, ReturnNullptrIfBuilderDoesNotKnowType) {
        JSONNode node;
        node.push_back(JSONNode("type", A::type + "x"));
        node.push_back(JSONNode("some_data", 34));
        EXPECT_EQ(nullptr, Serializable::DeserializeFromJson(&node));
    }

    TEST_F(TestSerializable, ReturnNullptrBuilderNotSet) {
        JSONNode node;
        node.push_back(JSONNode("type", A::type));
        node.push_back(JSONNode("some_data", 34));
        Serializable::SetObjectBuilder(nullptr);
        EXPECT_EQ(nullptr, Serializable::DeserializeFromJson(&node));
    }


    TEST_F(TestSerializable, BasicSerialize_A) {
        auto c = std::make_shared<A>();
        c->SetSomeData(34);

        JSONNode node;
        c->Serialize(&node);
        ASSERT_EQ(node["type"], A::type);
        ASSERT_EQ(node["some_data"], 34);
    }

    TEST_F(TestSerializable, BasicSerialize_B) {
        auto c = std::make_shared<B>();
        c->SetSomeData(34);
        c->SetOtherData(42);

        JSONNode node;
        c->Serialize(&node);
        ASSERT_EQ(node["type"], B::type);
        ASSERT_EQ(node["some_data"], 34);
        ASSERT_EQ(node["other_data"], 42);
    }

    TEST_F(TestSerializable, BasicSerialize_C) {
        auto b = std::make_shared<B>();
        b->SetSomeData(34);
        b->SetOtherData(42);

        auto c = std::make_shared<C>();
        c->SetB(b);

        JSONNode node;
        c->Serialize(&node);
        ASSERT_EQ(node["type"], C::type);
        ASSERT_EQ(node["b"]["type"], B::type);
        ASSERT_EQ(node["b"]["some_data"], 34);
        ASSERT_EQ(node["b"]["other_data"], 42);
    }


    TEST_F(TestSerializable, BasicDeserialize_A) {
        JSONNode node;
        node.push_back(JSONNode("type", A::type));
        node.push_back(JSONNode("some_data", 34));

        auto c = std::make_shared<A>();
        c->Deserialize(&node);
        ASSERT_EQ(c->GetSomeData(), 34);
    }

    TEST_F(TestSerializable, BasicDeserialize_B) {
        JSONNode node;
        node.push_back(JSONNode("type", B::type));
        node.push_back(JSONNode("some_data", 34));
        node.push_back(JSONNode("other_data", 42));

        auto c = std::make_shared<B>();
        c->Deserialize(&node);
        ASSERT_EQ(c->GetSomeData(), 34);
        ASSERT_EQ(c->GetOtherData(), 42);
    }

    TEST_F(TestSerializable, ConstructAndDeserialize_A) {
        JSONNode node;
        node.push_back(JSONNode("type", A::type));
        node.push_back(JSONNode("some_data", 34));

        auto c = std::static_pointer_cast<A>(Serializable::DeserializeFromJson(&node));
        ASSERT_EQ(c->GetSomeData(), 34);
    }

    TEST_F(TestSerializable, ConstructAndDeserialize_B) {
        JSONNode node;
        node.push_back(JSONNode("type", B::type));
        node.push_back(JSONNode("some_data", 34));
        node.push_back(JSONNode("other_data", 42));

        auto c = std::static_pointer_cast<B>(Serializable::DeserializeFromJson(&node));
        ASSERT_EQ(c->GetSomeData(), 34);
        ASSERT_EQ(c->GetOtherData(), 42);
    }

    TEST_F(TestSerializable, ConstructAndDeserialize_C) {
        JSONNode node;
        node.push_back(JSONNode("type", C::type));

        JSONNode nodeB;
        nodeB.set_name("b");
        nodeB.push_back(JSONNode("type", B::type));
        nodeB.push_back(JSONNode("some_data", 34));
        nodeB.push_back(JSONNode("other_data", 42));
        node.push_back(nodeB);

        auto c = std::static_pointer_cast<C>(Serializable::DeserializeFromJson(&node));
        ASSERT_EQ(c->GetB()->GetSomeData(), 34);
        ASSERT_EQ(c->GetB()->GetOtherData(), 42);
    }

    TEST_F(TestSerializable, SerializeDeserializeToText) {
        auto b = std::make_shared<B>();
        b->SetSomeData(34);
        b->SetOtherData(42);

        auto c = std::make_shared<C>();
        c->SetB(b);

        std::string jsonString = c->SerializeText();

        auto c2 = std::static_pointer_cast<C>(Serializable::DeserializeFromJsonText(jsonString));
        ASSERT_EQ(c2->GetB()->GetSomeData(), 34);
        ASSERT_EQ(c2->GetB()->GetOtherData(), 42);

        c->GetB()->SetOtherData(50);
        c->GetB()->SetSomeData(51);
        ASSERT_EQ(c->GetB()->GetSomeData(), 51);
        ASSERT_EQ(c->GetB()->GetOtherData(), 50);

        c->DeserializeText(jsonString);
        ASSERT_EQ(c->GetB()->GetSomeData(), 34);
        ASSERT_EQ(c->GetB()->GetOtherData(), 42);
    }

    TEST_F(TestSerializable, DeserializeAttribute_AttrbiuteNotSet_ReturnsDefaultValue) {

        JSONNode node;
        node.push_back(JSONNode("type", A::type));
        node.push_back(JSONNode("some_data", 34));
        node.set_name("test_attr");

        auto defaultA = std::static_pointer_cast<A>(Serializable::DeserializeFromJson(&node));
        ASSERT_NE(nullptr, defaultA);

        JSONNode rootNode;
        rootNode.push_back(node);

        auto deserialized = Serializable::DeserializeAttribute<A>(&rootNode, "not_set_attribute", defaultA);
        EXPECT_EQ(defaultA, deserialized);
    }

    TEST_F(TestSerializable, DeserializeAttribute_UnknownTypeForTheFactory_ReturnsDefaultValue) {

        std::shared_ptr<A> defaultA;

        {
            JSONNode node;
            node.push_back(JSONNode("type", A::type));
            node.push_back(JSONNode("some_data", 34));

            defaultA = std::static_pointer_cast<A>(Serializable::DeserializeFromJson(&node));
        }

        ASSERT_NE(nullptr, defaultA);

        JSONNode unknownAttributeNode;
        unknownAttributeNode.push_back(JSONNode("type", A::type + "_unknownType"));
        unknownAttributeNode.push_back(JSONNode("some_data", 40));
        unknownAttributeNode.set_name("test_attr");

        JSONNode rootNode;
        rootNode.push_back(unknownAttributeNode);

        auto deserialized = Serializable::DeserializeAttribute<A>(&rootNode, "test_attr", defaultA);
        EXPECT_EQ(defaultA, deserialized);
    }
}
