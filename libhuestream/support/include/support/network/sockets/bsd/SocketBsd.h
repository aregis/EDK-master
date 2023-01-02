/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/network/sockets/bsd/_test/CMethodDelegator.h"

#define ERRNO       (support::CMethodDelegator::get_errno())

#define CLOSE       support::CMethodDelegator::close
#define _SOCKET     support::CMethodDelegator::socket
#define BIND        support::CMethodDelegator::bind
#define CONNECT     support::CMethodDelegator::connect
#define SELECT      support::CMethodDelegator::select
#define SENDTO      support::CMethodDelegator::sendto
#define SEND        support::CMethodDelegator::send
#define RECV        support::CMethodDelegator::recv
#define GETSOCKOPT  support::CMethodDelegator::getsockopt
#define SETSOCKOPT  support::CMethodDelegator::setsockopt
#define INET_PTON   support::CMethodDelegator::inet_pton

#define SETNONBLOCKING support::CMethodDelegator::set_nonblocking

