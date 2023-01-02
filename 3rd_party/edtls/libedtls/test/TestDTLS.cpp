/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include "gtest/gtest.h"
#include "edtls/client/DTLSClient.h"
#include "edtls/server/DTLSServer.h"
#include "MockIServerNotifier.h"
#include "MockIClientNotifier.h"
#include "MockIPSKProvider.h"
#include "edtls/wrapper/mbedtls/MbedtlsServerWrapperFactory.h"
#include "edtls/wrapper/mbedtls/MbedtlsServerWrapper.h"
#include "edtls/wrapper/mbedtls/MbedtlsClientWrapperFactory.h"
#include "edtls/wrapper/mbedtls/MbedtlsClientWrapper.h"
#include "edtls/wrapper/mbedtls/DefaultEntropyProvider.h"
#include "edtls/wrapper/mbedtls/DefaultTimerProvider.h"
#include "edtls/wrapper/mbedtls/DefaultUDPClient.h"

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::InvokeWithoutArgs;
using ::testing::StrEq;
using ::testing::Range;

MATCHER_P(charStringMatcher, expCharString, "String does not match") {
  return strcmp((const char *) arg, (const char *) expCharString) == 0;
}

MATCHER_P2(MemoryMatcher, data, size, "Memory does not match") {
  return memcmp(arg, data, size) == 0;
}

static void DefaultPrintfLogger(const char* s) {
    printf("%s", s);
}

class TimeoutTrigger {
 private:
  std::mutex mutex;
  bool isTriggered;
  void sleep(unsigned int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  }

 public:
  TimeoutTrigger() : isTriggered(false) { }
  void trigger() {
    std::lock_guard<std::mutex> lock(mutex);
    isTriggered = true;
  }

  bool is_triggered() {
    std::lock_guard<std::mutex> lock(mutex);
    return isTriggered;
  }

  void wait_for_trigger(double timeout_ms) {
    auto start = std::chrono::high_resolution_clock::now();
    bool timeout = false;
    while (!is_triggered() && !timeout) {
      sleep(1);
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;
      timeout = elapsed.count() > timeout_ms;
    }
  }
};

class ACTestDTLS: public testing::TestWithParam<int> {
 protected:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

#define ASSERT_TRUE_WITH_TIMEOUT(function, timeout_ms) {\
    auto start = std::chrono::high_resolution_clock::now();\
    while (!(function)) {\
      sleep(1);\
      auto end = std::chrono::high_resolution_clock::now();\
      std::chrono::duration<double, std::milli> elapsed = end-start;\
      auto timeout = elapsed.count() > timeout_ms;\
      ASSERT_FALSE(timeout) << "on " << #function;\
    }\
  }

  virtual void sleep(unsigned int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  }

  virtual void startServer(DTLSServer *server, std::string address, std::string port, unsigned int timeoutSeconds) {
    server->start_async(address.c_str(), port.c_str(), timeoutSeconds);
  }

  virtual bool connectClient(DTLSClient *client, std::string address, std::string port, PSKInfo pskInfo) {
    return client->connect(address.c_str(), port.c_str(), pskInfo);
  }

  virtual void sendAndTestData(DTLSClient *client, const char *data, MockIServerNotifier* serverNotifier) {
    TimeoutTrigger received_data_trigger;
    EXPECT_CALL(*serverNotifier, receive_data(charStringMatcher(data), strlen(data) + 1)).Times(1).WillOnce(
        InvokeWithoutArgs(&received_data_trigger, &TimeoutTrigger::trigger));
    ASSERT_GT(client->send(const_cast<char *>(data), strlen(data) + 1), (unsigned int)0);
    received_data_trigger.wait_for_trigger(5000);
  }

  MbedtlsClientPlatformPtr CreateClientPlatform() {
    return std::make_shared<MbedtlsClientPlatform>(
        DefaultPrintfLogger,
        std::make_shared<DefaultEntropyProvider>(),
        std::make_shared<DefaultTimerProvider>(),
        std::make_shared<DefaultUDPClient>());
  }

  MbedtlsServerPlatformPtr CreateServerPlatform(PSKProviderPtr pskProvider) {
    return std::make_shared<MbedtlsServerPlatform>(
        DefaultPrintfLogger,
        std::make_shared<DefaultEntropyProvider>(),
        std::make_shared<DefaultTimerProvider>(),
        pskProvider);
  }
};


TEST_P(ACTestDTLS, StartStopServer) {
  MockIServerNotifier serverNotifier;
  auto pskProvider = std::make_shared<MockIPSKProvider>();
  DTLSServer server
      (MbedtlsServerWrapperFactory::get(CreateServerPlatform(pskProvider)), &serverNotifier, DefaultPrintfLogger);

  startServer(&server, "127.0.0.1", "1337", 10);
  server.stop();

  startServer(&server, "127.0.0.1", "1338", 10);
  server.stop();
  startServer(&server, "127.0.0.1", "1337", 10);
  server.stop();
}

TEST_P(ACTestDTLS, InputStringFromClientMustMatchOutputStringOnServer) {
  MockIServerNotifier serverNotifier;
  auto pskProvider = std::make_shared<MockIPSKProvider>();

  EXPECT_CALL(*pskProvider,
              getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                     strlen("Client_identity"),
                     _)).WillOnce(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                                                                     0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
                                                                                     0x0c, 0x0d, 0x0e, 0x0f})),
                                        Return(true)));

  MockIClientNotifier clientNotifier;
  DTLSClient client(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier);
  DTLSServer server(MbedtlsServerWrapperFactory::get(CreateServerPlatform(pskProvider)), &serverNotifier, DefaultPrintfLogger);
  auto str = "Hello World!";
  PSKInfo pskInfo("Client_identity",
                  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f});

  startServer(&server, "127.0.0.1", "1337", 10);
  ASSERT_TRUE(connectClient(&client, "127.0.0.1", "1337", pskInfo));
  sendAndTestData(&client, str, &serverNotifier);
}

TEST_P(ACTestDTLS, StopServerAfterTimeout) {
  MockIServerNotifier serverNotifier;
  auto pskProvider = std::make_shared<MockIPSKProvider>();

  EXPECT_CALL(*pskProvider,
              getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                     strlen("Client_identity"),
                     _)).WillOnce(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                                                                     0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
                                                                                     0x0c, 0x0d, 0x0e, 0x0f})),
                                        Return(true)));

  MockIClientNotifier clientNotifier;
  DTLSClient client(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier);
  DTLSServer server(MbedtlsServerWrapperFactory::get(CreateServerPlatform(pskProvider)), &serverNotifier, DefaultPrintfLogger);
  PSKInfo pskInfo("Client_identity",
                  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f});

  const int timeout = 2;
  {
    EXPECT_CALL(serverNotifier, server_stopped()).Times(0);
    startServer(&server, "127.0.0.1", "1337", timeout);
  }
  {
    ASSERT_TRUE(connectClient(&client, "127.0.0.1", "1337", pskInfo));
    sleep(1000);
    Mock::VerifyAndClearExpectations(&serverNotifier);
  }
  {
    EXPECT_CALL(serverNotifier, server_stopped()).Times(1);
    sleep(1400);
    Mock::VerifyAndClearExpectations(&serverNotifier);
  }
  {
    EXPECT_CALL(serverNotifier, server_stopped()).Times(0);
    sleep(400);
    Mock::VerifyAndClearExpectations(&serverNotifier);
  }
}

TEST_P(ACTestDTLS, HandleClientDisconnected) {
  MockIServerNotifier serverNotifier;
  MockIClientNotifier clientNotifier;
  auto pskProvider = std::make_shared<MockIPSKProvider>();

  EXPECT_CALL(*pskProvider,
              getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                     strlen("Client_identity"),
                     _)).WillOnce(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0x00, 0x01, 0x02, 0x03, 0x04,
                                                                                     0x05,
                                                                                     0x06, 0x07, 0x08, 0x09, 0x0a,
                                                                                     0x0b,
                                                                                     0x0c, 0x0d, 0x0e, 0x0f})),
                                        Return(true)));
  EXPECT_CALL(clientNotifier, handshakeFailed()).Times(0);

  DTLSClient client(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier);
  DTLSServer server
      (MbedtlsServerWrapperFactory::get(CreateServerPlatform(pskProvider)), &serverNotifier, DefaultPrintfLogger);
  PSKInfo pskInfo("Client_identity",
                  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f});

  startServer(&server, "127.0.0.1", "1337", 10);
  ASSERT_TRUE(connectClient(&client, "127.0.0.1", "1337", pskInfo));

  TimeoutTrigger peer_closed_trigger;
  EXPECT_CALL(serverNotifier, peer_closed()).Times(1).WillOnce(InvokeWithoutArgs(&peer_closed_trigger,
                                                                                 &TimeoutTrigger::trigger));
  client.close();
  peer_closed_trigger.wait_for_trigger(5000);
}

TEST_P(ACTestDTLS, HandleClientDisconnectedByDeletingIt) {
  MockIServerNotifier serverNotifier;
  MockIClientNotifier clientNotifier;
  auto pskProvider = std::make_shared<MockIPSKProvider>();

  EXPECT_CALL(*pskProvider,
              getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                     strlen("Client_identity"),
                     _)).WillOnce(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                                                                     0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
                                                                                     0x0c, 0x0d, 0x0e, 0x0f})),
                                        Return(true)));
  EXPECT_CALL(clientNotifier, handshakeFailed()).Times(0);

  DTLSServer server(MbedtlsServerWrapperFactory::get(CreateServerPlatform(pskProvider)), &serverNotifier, DefaultPrintfLogger);
  auto client = new DTLSClient(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier);
  PSKInfo pskInfo("Client_identity",
                  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f});


  startServer(&server, "127.0.0.1", "1337", 10);
  ASSERT_TRUE(connectClient(client, "127.0.0.1", "1337", pskInfo));

  TimeoutTrigger peer_closed_trigger;
  EXPECT_CALL(serverNotifier, peer_closed()).Times(1).WillOnce(InvokeWithoutArgs(&peer_closed_trigger,
                                                                                 &TimeoutTrigger::trigger));
  delete client;
  peer_closed_trigger.wait_for_trigger(5000);
}

TEST_P(ACTestDTLS, HandleClientReconnect) {
  MockIServerNotifier serverNotifier;
  MockIClientNotifier clientNotifier;
  auto pskProvider = std::make_shared<MockIPSKProvider>();

  EXPECT_CALL(*pskProvider,
              getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                     strlen("Client_identity"),
                     _)).Times(2).WillRepeatedly(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0x00, 0x01, 0x02,
                                                                                                    0x03, 0x04, 0x05,
                                                                                                    0x06, 0x07, 0x08,
                                                                                                    0x09, 0x0a, 0x0b,
                                                                                                    0x0c, 0x0d, 0x0e,
                                                                                                    0x0f})),
                                                       Return(true)));
  EXPECT_CALL(clientNotifier, handshakeFailed()).Times(0);

  DTLSServer server(MbedtlsServerWrapperFactory::get(CreateServerPlatform(pskProvider)), &serverNotifier, DefaultPrintfLogger);
  DTLSClient client(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier);
  PSKInfo pskInfo("Client_identity",
                  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f});


  startServer(&server, "127.0.0.1", "1337", 10);
  ASSERT_TRUE(connectClient(&client, "127.0.0.1", "1337", pskInfo));
  sendAndTestData(&client, "Hello World!", &serverNotifier);

  TimeoutTrigger peer_closed_trigger;
  EXPECT_CALL(serverNotifier, peer_closed()).Times(1).WillOnce(InvokeWithoutArgs(&peer_closed_trigger,
                                                                                 &TimeoutTrigger::trigger));
  client.close();
  peer_closed_trigger.wait_for_trigger(5000);

  startServer(&server, "127.0.0.1", "1337", 10);

  MockIClientNotifier clientNotifier2;
  EXPECT_CALL(clientNotifier, handshakeFailed()).Times(0);
  auto client2 = new DTLSClient(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier2);

  ASSERT_TRUE(connectClient(client2, "127.0.0.1", "1337", pskInfo));
  sendAndTestData(client2, "Hello World!", &serverNotifier);
}

TEST_P(ACTestDTLS, HandleClientDisconnectAndOtherClientConnect) {
  MockIServerNotifier serverNotifier;
  MockIClientNotifier clientNotifier;
  auto pskProvider = std::make_shared<MockIPSKProvider>();

  EXPECT_CALL(*pskProvider,
              getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                     strlen("Client_identity"),
                     _)).Times(2).WillRepeatedly(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0x00, 0x01, 0x02,
                                                                                                    0x03, 0x04, 0x05,
                                                                                                    0x06, 0x07, 0x08,
                                                                                                    0x09, 0x0a, 0x0b,
                                                                                                    0x0c, 0x0d, 0x0e,
                                                                                                    0x0f})),
                                                       Return(true)));
  EXPECT_CALL(clientNotifier, handshakeFailed()).Times(0);

  DTLSServer server(MbedtlsServerWrapperFactory::get(CreateServerPlatform(pskProvider)), &serverNotifier, DefaultPrintfLogger);
  DTLSClient client(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier);
  PSKInfo pskInfo("Client_identity",
                  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f});


  startServer(&server, "127.0.0.1", "1337", 10);
  ASSERT_TRUE(connectClient(&client, "127.0.0.1", "1337", pskInfo));

  TimeoutTrigger peer_closed_trigger;
  EXPECT_CALL(serverNotifier, peer_closed()).Times(1).WillOnce(InvokeWithoutArgs(&peer_closed_trigger,
                                                                                 &TimeoutTrigger::trigger));
  client.close();
  peer_closed_trigger.wait_for_trigger(5000);

  MockIClientNotifier clientNotifier2;

  DTLSClient client2(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier2);
  startServer(&server, "127.0.0.1", "1337", 10);
  ASSERT_TRUE(connectClient(&client2, "127.0.0.1", "1337", pskInfo));
  sendAndTestData(&client2, "Hello World!", &serverNotifier);
  client2.close();
  TimeoutTrigger peer_closed_trigger2;
  EXPECT_CALL(serverNotifier, peer_closed()).Times(1).WillOnce(InvokeWithoutArgs(&peer_closed_trigger2,
                                                                                 &TimeoutTrigger::trigger));
  peer_closed_trigger2.wait_for_trigger(5000);
}

TEST_P(ACTestDTLS, HandleMultipleClientsConnect) {
    MockIServerNotifier serverNotifier;
    MockIClientNotifier clientNotifier;
    MockIClientNotifier clientNotifier2;
    auto pskProvider = std::make_shared<MockIPSKProvider>();

    EXPECT_CALL(*pskProvider,
                getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                       strlen("Client_identity"),
                       _)).WillOnce(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0x00, 0x01, 0x02, 0x03, 0x04,
                                                                                       0x05,
                                                                                       0x06, 0x07, 0x08, 0x09, 0x0a,
                                                                                       0x0b,
                                                                                       0x0c, 0x0d, 0x0e, 0x0f})),
                                          Return(true)));
    EXPECT_CALL(clientNotifier2, handshakeFailed()).Times(1);
    EXPECT_CALL(clientNotifier, handshakeFailed()).Times(0);

    DTLSServer server
        (MbedtlsServerWrapperFactory::get(CreateServerPlatform(pskProvider)), &serverNotifier, DefaultPrintfLogger);
    DTLSClient client(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier);
    PSKInfo pskInfo("Client_identity",
                    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f});

    startServer(&server, "127.0.0.1", "1337", 10);
    ASSERT_TRUE(connectClient(&client, "127.0.0.1", "1337", pskInfo));

    DTLSClient client2(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), 2000, 4, &clientNotifier2);
    ASSERT_FALSE(connectClient(&client2, "127.0.0.1", "1337", pskInfo));
}

TEST_P(ACTestDTLS, HandleWrongPSK) {
  MockIServerNotifier serverNotifier;
  MockIClientNotifier clientNotifier;
  auto pskProvider = std::make_shared<MockIPSKProvider>();

  DTLSServer server
      (MbedtlsServerWrapperFactory::get(CreateServerPlatform(pskProvider)), &serverNotifier, DefaultPrintfLogger);
  DefaultEntropyProvider entropyProvider;
  DefaultUDPClient udpClient;
  DTLSClient client(MbedtlsClientWrapperFactory::get(CreateClientPlatform()), &clientNotifier);

  EXPECT_CALL(clientNotifier, handshakeFailed()).Times(1);
  EXPECT_CALL(*pskProvider,
              getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                     strlen("Client_identity"),
                     _)).WillOnce(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                                                     0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                                                     0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                                                     0xAA})),
                                        Return(true)));

  startServer(&server, "127.0.0.1", "1337", 10);
  ASSERT_FALSE(connectClient(&client,
                             "127.0.0.1",
                             "1337",
                             PSKInfo("Client_identity",
                                     {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
                                      0xBB, 0xBB, 0xBB})));
  server.stop();

  EXPECT_CALL(clientNotifier, handshakeFailed()).Times(1);
  EXPECT_CALL(*pskProvider, getKey(_, _, _)).WillOnce(Return(false));
  startServer(&server, "127.0.0.1", "1337", 10);
  ASSERT_FALSE(connectClient(&client,
                             "127.0.0.1",
                             "1337",
                             PSKInfo("Client_identity",
                                     {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                      0xAA, 0xAA, 0xAA})));
  server.stop();

  EXPECT_CALL(*pskProvider,
              getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                     strlen("Client_identity"),
                     _)).WillOnce(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                                                     0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                                                     0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                                                     0xAA})),
                                        Return(true)));
  startServer(&server, "127.0.0.1", "1337", 10);
  ASSERT_TRUE(connectClient(&client,
                            "127.0.0.1",
                            "1337",
                            PSKInfo("Client_identity",
                                    {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                     0xAA, 0xAA, 0xAA})));
  server.stop();

  EXPECT_CALL(*pskProvider,
              getKey(MemoryMatcher((const unsigned char *) "Client_identity", strlen("Client_identity")),
                     strlen("Client_identity"),
                     _)).WillOnce(DoAll(SetArgPointee<2>(std::vector<unsigned char>({0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                                                     0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                                                     0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                                                     0xAA})),
                                        Return(true)));
  startServer(&server, "127.0.0.1", "1337", 10);

  ASSERT_TRUE(connectClient(&client,
                            "127.0.0.1",
                            "1337",
                            PSKInfo("Client_identity",
                                    {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                     0xAA, 0xAA, 0xAA})));
  EXPECT_CALL(clientNotifier, handshakeFailed()).Times(1);
  ASSERT_FALSE(connectClient(&client,
                             "127.0.0.1",
                             "1337",
                             PSKInfo("Wrong",
                                     {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
                                      0xBB, 0xBB, 0xB})));
  server.stop();
}
INSTANTIATE_TEST_CASE_P(Stress_100, ACTestDTLS, Range(0, 100, 1));
INSTANTIATE_TEST_CASE_P(Stress_1, ACTestDTLS, Range(0, 1, 1));

