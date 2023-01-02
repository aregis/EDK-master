/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef TEST_MOCKISERVERNOTIFIER_H_
#define TEST_MOCKISERVERNOTIFIER_H_

#include "gmock/gmock.h"
#include "edtls/server/IServerNotifier.h"

class MockIServerNotifier: public IServerNotifier {
 public:
  MOCK_METHOD2(receive_data, void(char *buffer, unsigned int bytes_received));
  MOCK_METHOD0(peer_closed, void());
  MOCK_METHOD0(server_stopped, void());
};

#endif  // TEST_MOCKISERVERNOTIFIER_H_
