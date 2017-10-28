#include "writer.h"
#include "../core/invocation.hpp"

namespace nng {

    _OptionWriter::_OptionWriter()
        : _setopt()
        , _setopt_int()
        , _setopt_sz()
        , _setopt_duration() {
    }

    void _OptionWriter::set_setters(const setopt_func& setopt
        , const setopt_int_func& setopt_int
        , const setopt_sz_func& setopt_sz
        , const setopt_duration_func& setopt_duration) {

        // Cast the constness away from the setters long enough to install the new ones.
        const_cast<setopt_func&>(_setopt) = setopt;
        const_cast<setopt_int_func&>(_setopt_int) = setopt_int;
        const_cast<setopt_sz_func&>(_setopt_sz) = setopt_sz;
        const_cast<setopt_duration_func&>(_setopt_duration) = setopt_duration;
    }

    _OptionWriter::~_OptionWriter() {
    }

    void _OptionWriter::set(const std::string& name, const void* valp, size_type sz) {
        invocation::with_default_error_handling(_setopt, name.c_str(), valp, sz);
    }

    void _OptionWriter::set(const std::string& name, const std::string& val) {
        invocation::with_default_error_handling(_setopt, name.c_str(), val.c_str(), val.length());
    }

    void _OptionWriter::set_int(const std::string& name, int val) {
        invocation::with_default_error_handling(_setopt_int, name.c_str(), val);
    }

    void _OptionWriter::set_sz(const std::string& name, size_type val) {
        invocation::with_default_error_handling(_setopt_sz, name.c_str(), val);
    }

    void _OptionWriter::set(const std::string& name, const duration_type& val) {
        set_milliseconds(name, val.count());
    }

    void _OptionWriter::set_milliseconds(const std::string& name, duration_rep_type val) {
        invocation::with_default_error_handling(_setopt_duration, name.c_str(), val);
    }
}