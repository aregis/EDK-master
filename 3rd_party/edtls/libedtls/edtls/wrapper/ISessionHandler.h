/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_ISESSIONHANDLER_H_
#define EDTLS_WRAPPER_ISESSIONHANDLER_H_

class ISessionHandler {
 public:
  virtual ~ISessionHandler() = default;
  virtual void handle_timeout() = 0;
  virtual void handle_peer_closed() = 0;
  virtual void handle_unrecoverable_read_error() = 0;
};

#endif  // EDTLS_WRAPPER_ISESSIONHANDLER_H_
