/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_HELPERS_H_
#define EDTLS_WRAPPER_HELPERS_H_

#include <memory>
#include <iostream>
#include <string>
#include <cstdio>

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args) {
  int size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;  // Extra space for '\0'
  std::unique_ptr<char[]> buf(new char[ size ]);
  snprintf(buf.get(), size, format.c_str(), args ...);
  return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
}

#endif  // EDTLS_WRAPPER_HELPERS_H_
