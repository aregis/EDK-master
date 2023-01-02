/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_ICLIENTCONNECTIONHANDLER_H_
#define EDTLS_WRAPPER_ICLIENTCONNECTIONHANDLER_H_

class IClientConnectionHandler {
 public:
  virtual ~IClientConnectionHandler() = default;
  virtual void handle_peer_closed() = 0;
};

#endif  // EDTLS_WRAPPER_ICLIENTCONNECTIONHANDLER_H_
