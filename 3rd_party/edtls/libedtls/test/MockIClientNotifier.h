/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef TEST_MOCKICLIENTNOTIFIER_H_
#define TEST_MOCKICLIENTNOTIFIER_H_

class MockIClientNotifier: public IClientNotifier  {
 public:
  MOCK_METHOD0(handshakeFailed, void());
  MOCK_METHOD0(peer_closed, void());
};

#endif  // TEST_MOCKICLIENTNOTIFIER_H_
