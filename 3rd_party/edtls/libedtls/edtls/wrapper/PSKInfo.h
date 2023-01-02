/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_PSKINFO_H_
#define EDTLS_WRAPPER_PSKINFO_H_

#include <memory.h>
#include <assert.h>

#include <vector>

struct PSK {
  unsigned char key_[16];
  PSK(){
    memset(key_, 0x00, 16);
  }
};

struct PSKInfo {
  PSK psk_;
  const char *identity_;
  PSKInfo(): psk_(), identity_("") {}
  PSKInfo(const char *identity, std::vector<unsigned char> psk) : psk_(), identity_(identity) {
    assert(psk.size() == 16);
    unsigned int i = 0;
    for (auto item : psk){
      psk_.key_[i] = item;
      ++i;
    }
  }
};

#endif  // EDTLS_WRAPPER_PSKINFO_H_
