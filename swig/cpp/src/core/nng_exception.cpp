#include "nng_exception.h"

namespace nng {

    nng_exception::nng_exception() : std::exception() {
        error_code = ec_eunknown;
    }

    nng_exception::nng_exception(int ec) : std::exception() {
        error_code = static_cast<error_code_type>(ec);
        message = ::nng_strerror(ec);
    }

    nng_exception::nng_exception(error_code_type ec) : std::exception() {
        error_code = ec;
        message = ::nng_strerror(static_cast<int>(ec));
    }

    nng_exception::~nng_exception() {        
    }
}
