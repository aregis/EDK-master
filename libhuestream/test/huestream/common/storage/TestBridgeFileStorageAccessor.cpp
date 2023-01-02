#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <huestream/common/data/BridgeSettings.h>
#include <huestream/connect/BridgeFileStorageAccessor.h>

#include <fstream>
#include <memory>

using namespace huestream;
using namespace std;

class TestBridgeFileStorageAccessor : public testing::Test {
public:
    std::string _fileName;

    void SetUp() override {
        _fileName = "TestBridgeFileStorageAccessor.bridge.json";

        FILE *fp = fopen(_fileName.c_str(), "w");
        const char *unencrypted_data = "{\"type\":\"huestream.Bridge\","
                "\"Name\":\"\","
                "\"ModelId\":\"BSB002\","
                "\"SoftwareVersion\":\"01035797\","
                "\"Apiversion\":\"1.17.0\","
                "\"Id\":\"00:11:22:33:44\","
                "\"IpAddress\":\"192.168.1.1\","
                "\"IsValidIp\":true,"
                "\"IsAuthorized\":true,"
                "\"ClientKey\":\"DD129216F1A50E5D1C0CB356325745F2\","
                "\"User\":\"HJD77jsjs-7883kkKS@\","
                "\"SelectedGroup\":123}";
        if (fp != NULL)
        {
            fputs(unencrypted_data, fp);
            fclose(fp);
        }
    }

    void TearDown() override {
        std::remove(_fileName.c_str());
    }

    HueStreamDataPtr get_complete_bridge_data() const {
        auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
        bridge->SetId("00:11:22:33:44");
        bridge->SetUser("HJD77jsjs-7883kkKS@");
        bridge->SetModelId("BSB002");
        bridge->SetApiversion("1.17.0");
				bridge->SetSwversion("1940094000");
        bridge->SetClientKey("DD129216F1A50E5D1C0CB356325745F2");
        {
            auto group1 = std::make_shared<Group>();
            group1->SetId("123");
            group1->SetName("My entertainment setup1");
            group1->AddLight("22", 0.1, 0.1);
            group1->AddLight("12", 0.3, 0.2);
        }
        bridge->SetSelectedGroup("123");
        bridge->SetIsValidIp(true);
        bridge->SetIsAuthorized(true);
        bridge->SetIpAddress("192.168.1.1");

        auto hueStreamData = std::make_shared<HueStreamData>(std::make_shared<BridgeSettings>());
        hueStreamData->SetActiveBridge(bridge);

        return hueStreamData;
    }

    std::string get_corrupt_json_string() const {
        return "\"bridge\":\"no\"";
    }
};

TEST_F(TestBridgeFileStorageAccessor, Load_EmptyEncryptionKey) {
    auto bridgeStorageAccessorPtr = std::make_shared<BridgeFileStorageAccessor>(_fileName, std::make_shared<AppSettings>(), std::make_shared<BridgeSettings>());
    bridgeStorageAccessorPtr->Load([this](OperationResult oRes, HueStreamDataPtr bPtr){
        ASSERT_EQ(OperationResult::OPERATION_SUCCESS, oRes);
        ASSERT_NE(nullptr, bPtr);
    });
}

TEST_F(TestBridgeFileStorageAccessor, Save_EmptyEncryptionKey) {
    auto data = get_complete_bridge_data();

    auto bridgeStorageAccessorPtr = std::make_shared<BridgeFileStorageAccessor>(_fileName, std::make_shared<AppSettings>(), std::make_shared<BridgeSettings>());
    bridgeStorageAccessorPtr->Save(data, [this](OperationResult oRes){
        ASSERT_EQ(OperationResult::OPERATION_SUCCESS, oRes);
    });
    ASSERT_TRUE(std::ifstream(_fileName));
}

TEST_F(TestBridgeFileStorageAccessor, SaveLoad_WithEncryption) {
    auto data = get_complete_bridge_data();

    auto appConfig = std::make_shared<AppSettings>();
    appConfig->SetStorageEncryptionKey("very_secure_encryption_key");
    auto storageAccessor = std::make_shared<BridgeFileStorageAccessor>(_fileName, appConfig, std::make_shared<BridgeSettings>());
    storageAccessor->Save(data, [this](OperationResult oRes){
        EXPECT_EQ(OperationResult::OPERATION_SUCCESS, oRes);
    });
    ASSERT_TRUE(std::ifstream(_fileName));

    storageAccessor->Load([this, data](OperationResult oRes, HueStreamDataPtr loadedData) {
        EXPECT_EQ(OperationResult::OPERATION_SUCCESS, oRes);
        ASSERT_NE(nullptr, loadedData);

        auto activeBridge = loadedData->GetActiveBridge();
        ASSERT_NE(nullptr, activeBridge);
        EXPECT_EQ(activeBridge->SerializeCompact(), data->GetActiveBridge()->SerializeCompact());
    });
}

TEST_F(TestBridgeFileStorageAccessor, SaveLoad_WithWrongEncryptionKey) {
    auto data = get_complete_bridge_data();

    {
        auto appConfig = std::make_shared<AppSettings>();
        appConfig->SetStorageEncryptionKey("very_secure_encryption_key");
        auto storageAccessor = std::make_shared<BridgeFileStorageAccessor>(_fileName, appConfig, std::make_shared<BridgeSettings>());
        storageAccessor->Save(data, [this](OperationResult oRes) {
            EXPECT_EQ(OperationResult::OPERATION_SUCCESS, oRes);
        });
        ASSERT_TRUE(std::ifstream(_fileName));
    }

    {
        auto appConfig = std::make_shared<AppSettings>();
        appConfig->SetStorageEncryptionKey("hacker_encryption_key");
        auto storageAccessor = std::make_shared<BridgeFileStorageAccessor>(_fileName, appConfig, std::make_shared<BridgeSettings>());
        storageAccessor->Load([this, data](OperationResult oRes, HueStreamDataPtr loadedData) {
            EXPECT_EQ(OperationResult::OPERATION_SUCCESS, oRes);
            ASSERT_NE(nullptr, loadedData);

            auto activeBridge = loadedData->GetActiveBridge();
            ASSERT_TRUE(activeBridge->IsEmpty());
        });
    }
}

TEST_F(TestBridgeFileStorageAccessor, SaveLoad_WithInvalidEncryptionKey) {
    auto data = get_complete_bridge_data();

    {
        auto storageAccessor = std::make_shared<BridgeFileStorageAccessor>(_fileName, std::make_shared<AppSettings>(), std::make_shared<BridgeSettings>());
        storageAccessor->Save(data, [this](OperationResult oRes) {
            EXPECT_EQ(OperationResult::OPERATION_SUCCESS, oRes);
        });
        ASSERT_TRUE(std::ifstream(_fileName));
    }

    {
        auto appConfig = std::make_shared<AppSettings>();
        appConfig->SetStorageEncryptionKey("hacker_encryption_key");
        auto storageAccessor = std::make_shared<BridgeFileStorageAccessor>(_fileName, appConfig, std::make_shared<BridgeSettings>());
        storageAccessor->Load([this, data](OperationResult oRes, HueStreamDataPtr loadedData) {
            EXPECT_EQ(OperationResult::OPERATION_FAILED, oRes);
        });
    }
}

TEST_F(TestBridgeFileStorageAccessor, Load_FileHasCorruptedData) {
    {
        std::fstream file;
        file.open(_fileName, std::fstream::out);
        file << get_corrupt_json_string();
    }

    ASSERT_TRUE(std::ifstream(_fileName));

    auto storageAccessor = std::make_shared<BridgeFileStorageAccessor>(_fileName, std::make_shared<AppSettings>(), std::make_shared<BridgeSettings>());
    storageAccessor->Load([this](OperationResult oRes, HueStreamDataPtr loadedData) {
        EXPECT_EQ(OperationResult::OPERATION_SUCCESS, oRes);
        ASSERT_NE(nullptr, loadedData);

        auto activeBridge = loadedData->GetActiveBridge();
        ASSERT_TRUE(activeBridge->IsEmpty());
    });
}

TEST_F(TestBridgeFileStorageAccessor, Load_FileIsEmpty) {
    {
        std::fstream file;
        file.open(_fileName, std::fstream::out);
    }

    ASSERT_TRUE(std::ifstream(_fileName));

    auto storageAccessor = std::make_shared<BridgeFileStorageAccessor>(_fileName, std::make_shared<AppSettings>(), std::make_shared<BridgeSettings>());
    storageAccessor->Load([this](OperationResult oRes, HueStreamDataPtr loadedData) {
        EXPECT_EQ(OperationResult::OPERATION_SUCCESS, oRes);
        ASSERT_NE(nullptr, loadedData);

        auto activeBridge = loadedData->GetActiveBridge();
        ASSERT_TRUE(activeBridge->IsEmpty());
    });
}

TEST_F(TestBridgeFileStorageAccessor, Load_FileDoesntExist) {
    std::remove(_fileName.c_str());
    ASSERT_FALSE(std::ifstream(_fileName));

    auto storageAccessor = std::make_shared<BridgeFileStorageAccessor>(_fileName, std::make_shared<AppSettings>(), std::make_shared<BridgeSettings>());
    storageAccessor->Load([this](OperationResult oRes, HueStreamDataPtr loadedData) {
        EXPECT_EQ(OperationResult::OPERATION_FAILED, oRes);
    });
}