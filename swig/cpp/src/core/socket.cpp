#include "socket.h"
#include "messenger.h"
#include "listener.h"
#include "dialer.h"
#include "invocation.hpp"

#include <vector>

namespace nng {

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    using std::bind;

    socket::socket(const nng_ctor_func& nng_ctor)
        : having_one(), sender(), receiver(), messenger()
        , sid(0), _options() {

        invocation::with_default_error_handling(nng_ctor, &sid);
        configure_options(sid);
    }

    socket::~socket() {
        close();
    }

    void socket::configure_options(nng_type sid) {

        _options.set_getters(
            bind(&::nng_getopt, sid, _1, _2, _3)
            , bind(::nng_getopt_int, sid, _1, _2)
            , bind(::nng_getopt_size, sid, _1, _2)
            , bind(&::nng_getopt_ms, sid, _1, _2)
        );

        _options.set_setters(
            bind(&::nng_setopt, sid, _1, _2, _3)
            , bind(::nng_setopt_int, sid, _1, _2)
            , bind(::nng_setopt_size, sid, _1, _2)
            , bind(&::nng_setopt_ms, sid, _1, _2)
        );
    }

    void socket::close() {
        if (!has_one()) { return; }
        // Close is its own operation apart from Shutdown.
        const auto op = bind(&::nng_close, sid);
        invocation::with_default_error_handling(op);
        // Closed is closed.
        configure_options(sid = 0);
    }

    void socket::shutdown() {
        // Shutdown is its own operation apart from Closed.
        const auto op = bind(&::nng_shutdown, sid);
        invocation::with_default_error_handling(op);
        // Which socket can still be in operation.
    }

    bool socket::has_one() const {
        return sid > 0;
    }

    // TODO: TBD: ditto ec handling...
    void socket::listen(const std::string& addr, flag_type flags) {
        const auto& op = bind(&::nng_listen, sid, _1, _2, _3);
        invocation::with_default_error_handling(op, addr.c_str(), nullptr, static_cast<int>(flags));
    }

    void socket::listen(const std::string& addr, listener* const lp, flag_type flags) {
        const auto& op = bind(&::nng_listen, sid, _1, _2, _3);
        invocation::with_default_error_handling(op, addr.c_str()
            , lp ? &(lp->lid) : nullptr, static_cast<int>(flags));
        if (lp) { lp->on_listened(); }
    }

    void socket::dial(const std::string& addr, flag_type flags) {
        const auto& op = bind(&::nng_dial, sid, _1, _2, _3);
        invocation::with_default_error_handling(op, addr.c_str(), nullptr, flags);
    }

    void socket::dial(const std::string& addr, dialer* const dp, flag_type flags) {
        const auto& op = bind(&::nng_dial, sid, _1, _2, _3);
        invocation::with_default_error_handling(op, addr.c_str()
            , dp ? &(dp->did) : nullptr, static_cast<int>(flags));
        if (dp) { dp->on_dialed(); }
    }

    template<class Buffer_>
    void send(int sid, const Buffer_& buf, std::size_t sz, flag_type flags) {
        // &buf[0] ????
        const auto& op = bind(&::nng_send, sid, _1, _2, _3);
        invocation::with_default_error_handling(op, (void*)buf.data(), sz
            , static_cast<int>(flags));
    }
    
    void socket::send(const buffer_vector_type* const bufp, flag_type flags) {
        nng::send(sid, *bufp, bufp->size(), flags);
    }

    void socket::send(const buffer_vector_type* const bufp, size_type sz, flag_type flags) {
        nng::send(sid, *bufp, sz, flags);
    }

    void socket::send(binary_message* const bmp, flag_type flags) {
        auto* msgp = bmp->get_msgp();
        if (msgp == nullptr) { return; }
        const auto op = bind(&::nng_sendmsg, sid, msgp, _1);
        invocation::with_default_error_handling(op, static_cast<int>(flags));
        // If we got this far then simply signal On-Sent to the message.
        bmp->on_sent();
    }

    template<class Buffer_>
    bool try_receive(int sid, Buffer_& buf, std::size_t& sz, flag_type flags) {
        buf.resize(sz);
        // &buf[0] ????
        const auto& op = bind(&::nng_recv, sid, _1, _2, _3);
        invocation::with_default_error_handling(op, (void*)buf.data(), &sz
            , static_cast<int>(flags));
        return sz > 0;
    }

    std::unique_ptr<binary_message> socket::receive(flag_type flags) {
        auto bmup = std::make_unique<binary_message>();
        try_receive(bmup.get(), flags);
        return bmup;
    }

    bool socket::try_receive(binary_message* const bmp, flag_type flags) {
        /* So this is somewhat of a long way around, but it represents the cost of NNG message
        ownership semantics. The cost has to be paid at some point, either on the front side or
        the back side, so we pay for it here in additional semantics. */
        msg_type* msgp = nullptr;
        try {
            const auto& recv_ = bind(&::nng_recvmsg, sid, &msgp, _1);
            invocation::with_default_error_handling(recv_, static_cast<int>(flags));
        }
#if 0
        catch (std::exception& ex) {
#else
        catch (...) {
#endif
            // TODO: TBD: this is probably (PROBABLY) about as good as we can expect here...
            if (msgp) {
                const auto& free_ = bind(&::nng_msg_free, msgp);
                invocation::with_void_return_value(free_);
            }
            // Re-throw the exception after taking care of potential memory allocation.
            throw;
        }
        bmp->set_msgp(msgp);
        return bmp->has_one();
    }

    buffer_vector_type socket::receive(size_type& sz, flag_type flags) {
        buffer_vector_type buf;
        // TODO: TBD: if we do not or cannot receive anything, return whatever we do have?
        this->try_receive(&buf, sz, flags);
        return buf;
    }

    bool socket::try_receive(buffer_vector_type* const bufp, size_type& sz, flag_type flags) {
        return nng::try_receive(sid, *bufp, sz, flags);
    }

    options_reader_writer* const socket::options() {
        return &_options;
    }

    protocol_type to_protocol_type(int value) {
        return static_cast<protocol_type>(value);
    }

    int to_int(const protocol_type value) {
        return static_cast<int>(value);
    }

    protocol_type socket::get_protocol() const {
        return to_protocol_type(::nng_protocol(sid));
    }

    protocol_type socket::get_peer() const {
        return to_protocol_type(::nng_peer(sid));
    }
}
