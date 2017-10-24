#ifndef NNGCPP_SOCKET_H
#define NNGCPP_SOCKET_H

#include "types.h"

#include "enums.h"
#include "sender.h"
#include "receiver.h"
#include "messenger.h"
#include "options.h"

// nng should be in the include path.
#include <functional>
#include <string>

namespace nng {

    enum protocol_type {
        protocol_none = ::NNG_PROTO_NONE,

        proto_bus_v0 = ::NNG_PROTO_BUS_V0,
        proto_pair_v0 = ::NNG_PROTO_PAIR_V0,
        proto_pair_v1 = ::NNG_PROTO_PAIR_V1,
        proto_subscriber_v0 = ::NNG_PROTO_SUB_V0,
        proto_publisher_v0 = ::NNG_PROTO_PUB_V0,
        proto_requestor_v0 = ::NNG_PROTO_REQ_V0,
        proto_replier_v0 = ::NNG_PROTO_REP_V0,
        proto_pusher_v0 = ::NNG_PROTO_PUSH_V0,
        proto_puller_v0 = ::NNG_PROTO_PULL_V0,
        proto_surveyor_v0 = ::NNG_PROTO_SURVEYOR_V0,
        proto_respondent_v0 = ::NNG_PROTO_RESPONDENT_V0,
        proto_star_v0 = ::NNG_PROTO_STAR_V0,

        proto_bus = ::NNG_PROTO_BUS,
        proto_pair = ::NNG_PROTO_PAIR,
        proto_subscriber = ::NNG_PROTO_SUB,
        proto_publisher = ::NNG_PROTO_PUB,
        proto_requestor = ::NNG_PROTO_REQ,
        proto_replier = ::NNG_PROTO_REP,
        proto_pusher = ::NNG_PROTO_PUSH,
        proto_puller = ::NNG_PROTO_PULL,
        proto_surveyor = ::NNG_PROTO_SURVEYOR,
        proto_respondent = ::NNG_PROTO_RESPONDENT,
        // TODO: TBD: NNG_PROTO_STAR undefined as of now; expecting NNG_PROTO_STAR moving forward? with protocol to boot?
        proto_star = ::NNG_PROTO_STAR_V0
    };

    protocol_type to_protocol_type(int value);
    int to_int(const protocol_type value);

    class listener;
    class dialer;
    struct device_path;

    class socket : public sender, public receiver, public messenger {
    public:

        typedef messaging::buffer_vector_type buffer_vector_type;
        typedef messaging::binary_message binary_message_type;

        typedef ::nng_socket nng_type;

    private:

        friend class listener;
        friend class dialer;

        // For use with Device Thread Callback.
        friend void install_device_sockets_callback(const device_path* const);

        nng_type sid;

        options_reader_writer _options;

        void configure_options(nng_type sid);

    protected:

        typedef std::function<int(nng_type* const)> nng_ctor_func;

        socket(const nng_ctor_func& nng_ctor);

    public:

        virtual ~socket();

        // TODO: TBD: may want to comprehend nng's NNG_MAXADDRLEN at some level... expose as a static constant, for instance, bare minimum
        void listen(const std::string& addr, flag_type flags = flag_none);
        void listen(const std::string& addr, listener* const lp, flag_type flags = flag_none);

        void dial(const std::string& addr, flag_type flags = flag_none);
        void dial(const std::string& addr, dialer* const dp, flag_type flags = flag_none);

        void close();
        void shutdown();

        bool has_one() const;

        virtual options_reader_writer* const options();

        virtual void send(binary_message_type* const bmp, flag_type flags = flag_none) override;

        virtual void send(const buffer_vector_type* const bufp, flag_type flags = flag_none) override;
        virtual void send(const buffer_vector_type* const bufp, size_type sz, flag_type flags = flag_none) override;

        virtual std::unique_ptr<binary_message_type> receive(flag_type flags = flag_none) override;
        virtual bool try_receive(binary_message_type* const bmp, flag_type flags = flag_none) override;

        virtual buffer_vector_type receive(size_type& sz, flag_type flags = flag_none) override;
        virtual bool try_receive(buffer_vector_type* const bufp, size_type& sz, flag_type flags = flag_none) override;

        protocol_type get_protocol() const;
        protocol_type get_peer() const;
    };
}

#endif // NNGCPP_SOCKET_H
