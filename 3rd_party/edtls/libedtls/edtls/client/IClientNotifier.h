/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_CLIENT_ICLIENTNOTIFIER_H_
#define EDTLS_CLIENT_ICLIENTNOTIFIER_H_

class IClientNotifier {
 public:
  virtual ~IClientNotifier() = default;
  virtual void handshakeFailed() = 0;
  virtual void peer_closed() = 0;
};

#endif  // EDTLS_CLIENT_ICLIENTNOTIFIER_H_
