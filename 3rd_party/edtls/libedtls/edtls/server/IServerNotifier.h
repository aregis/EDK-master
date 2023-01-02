/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_SERVER_ISERVERNOTIFIER_H_
#define EDTLS_SERVER_ISERVERNOTIFIER_H_

class IServerNotifier {
 public:
  virtual ~IServerNotifier() = default;
  virtual void receive_data(char *buffer, unsigned int bytes_received) = 0;
  virtual void peer_closed() = 0;
  virtual void server_stopped() = 0;
};

#endif  // EDTLS_SERVER_ISERVERNOTIFIER_H_
