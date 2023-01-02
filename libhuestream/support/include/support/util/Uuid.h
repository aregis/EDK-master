/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"

/* EDK is compiled without exceptions (-fno-exceptions). This defines BOOST_NO_EXCEPTIONS in boost, used by boost::uuid.
 * Hence we explicitly define boost::throw_exception to do nothing */

#ifdef BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>

namespace boost {
    inline void throw_exception(std::exception const & e) {
            // do nothing
    }
}  // namespace boost
#endif
