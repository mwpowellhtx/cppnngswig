//
// Copyright (c) 2017 Michael W Powell <mwpowellhtx@gmail.com>
//
// This software is supplied under the terms of the MIT License, a
// copy of which should be located in the distribution where this
// file was obtained (LICENSE.txt).  A copy of the license may also be
// found online at https://opensource.org/licenses/MIT.
//

#include <nngcpp.h>

#include "../catch/catch_macros.hpp"
#include "../catch/catch_tags.h"

#include "../helpers/constants.h"

#include <memory>

namespace constants {

    const std::string nng_addr = "inproc://nng";
    const std::string test_addr = "inproc://test";

    const std::string hello = "hello";
    const std::string again = "again";

    const auto hello_buf = to_buffer(hello);
    const auto again_buf = to_buffer(again);
}

/* This is interesting in the event we actually need/want to separate the code under
test with a returning error number. */
#define NNG_TESTS_INIT() int errnum

#define NNG_TESTS_REQUIRE_ERR_EQ(code, expected) errnum = code; \
    REQUIRE(errnum == expected)

struct c_style_fixture {

    ::nng_socket push;
    ::nng_socket pull;

    c_style_fixture() : push(0), pull(0) {

        REQUIRE(::nng_push_open(&push) == 0);
        REQUIRE(::nng_pull_open(&pull) == 0);

        REQUIRE(push > 0);
        REQUIRE(pull > 0);
    }

    virtual ~c_style_fixture() {
        ::nng_close(push);
        ::nng_close(pull);
        ::nng_fini();
    }
};

namespace nng {

    struct message_pipe_fixture : public message_pipe {

        message_pipe_fixture(message_base* const mbp) : message_pipe(mbp) {
        }

        ::nng_pipe get_pid() const {
            return pid;
        }
    };
}

#define NNG_TESTS_VERIFY_TEST_CASE_DATA() \
    REQUIRE_THAT(constants::hello, Catch::Matchers::Equals("hello")); \
    REQUIRE_THAT(constants::again, Catch::Matchers::Equals("again")); \
    REQUIRE_THAT(constants::nng_addr, Catch::Matchers::Equals("inproc://nng")); \
    REQUIRE_THAT(constants::test_addr, Catch::Matchers::Equals("inproc://test"))

TEST_CASE("Catch translation of NNG C reconnect unit tests", Catch::Tags(
    "reconnect", "nng", "c").c_str()) {

    using namespace std;
    using namespace std::chrono;
    using namespace nng;
    using namespace constants;

    SECTION("Verify that we have the required data") {
        NNG_TESTS_VERIFY_TEST_CASE_DATA();
    }

    SECTION("We can create a pipeline") {

        /* When the scope rolls back, we want to destroy the fixture, which
        effectively closes the sockets, and finishes the NNG session. */
        c_style_fixture $;

        auto& pull = $.pull;
        auto& push = $.push;

        duration_type const timeout = 10ms;

        REQUIRE(::nng_setopt_ms(pull, NNG_OPT_RECONNMINT, timeout.count()) == 0);
        REQUIRE(::nng_setopt_ms(pull, NNG_OPT_RECONNMAXT, timeout.count()) == 0);

        SECTION("Dialing before listening works") {

            // With Reconnect Times configured, Should re-dial until Listener comes online.
            REQUIRE(::nng_dial(push, test_addr.c_str(), nullptr, NNG_FLAG_NONBLOCK) == 0);
            this_thread::sleep_for(100ms);
            REQUIRE(::nng_listen(pull, test_addr.c_str(), nullptr, 0) == 0);

            SECTION("We can send a frame") {

                ::nng_msg* msgp = nullptr;

                this_thread::sleep_for(100ms);

                REQUIRE(::nng_msg_alloc(&msgp, 0) == 0);
                REQUIRE(msgp != nullptr);
                NNG_TESTS_APPEND_STR(msgp, hello.c_str());
                REQUIRE(::nng_sendmsg(push, msgp, 0) == 0);

                /* Nullifying the pointer is NOT a mistake. It is part of the zero-copy message ownership
                semantics. Once the message is "sent", the framework effectively assumes ownership. */
                msgp = nullptr;

                // Which Receiving a Message should allocate when we did not provide one.
                REQUIRE(::nng_recvmsg(pull, &msgp, 0) == 0);
                NNG_TESTS_CHECK_STR(msgp, hello.c_str());

                ::nng_msg_free(msgp);
            }
        }

        SECTION("Reconnection works") {

            ::nng_listener l;

            REQUIRE(::nng_dial(push, test_addr.c_str(), nullptr, NNG_FLAG_NONBLOCK) == 0);
            REQUIRE(::nng_listen(pull, test_addr.c_str(), &l, 0) == 0);
            this_thread::sleep_for(100ms);
            REQUIRE(::nng_listener_close(l) == 0);
            REQUIRE(::nng_listen(pull, test_addr.c_str(), nullptr, 0) == 0);

            SECTION("They still exchange frames") {
                ::nng_msg *msgp;
                ::nng_pipe p1;

                this_thread::sleep_for(100ms);
                REQUIRE(::nng_msg_alloc(&msgp, 0) == 0);
                NNG_TESTS_APPEND_STR(msgp, hello.c_str());
                REQUIRE(::nng_sendmsg(push, msgp, 0) == 0);
                // Ditto message passing ownership semantics.
                msgp = nullptr;
                REQUIRE(::nng_recvmsg(pull, &msgp, 0) == 0);
                REQUIRE(msgp != nullptr);
                NNG_TESTS_CHECK_STR(msgp, hello.c_str());
                REQUIRE((p1 = ::nng_msg_get_pipe(msgp)) > 0);
                ::nng_msg_free(msgp);

                SECTION("Even after pipe close") {
                    ::nng_pipe p2;

                    REQUIRE(::nng_pipe_close(p1) == 0);
                    this_thread::sleep_for(100ms);
                    REQUIRE(::nng_msg_alloc(&msgp, 0) == 0);
                    NNG_TESTS_APPEND_STR(msgp, again.c_str());
                    REQUIRE(::nng_sendmsg(push, msgp, 0) == 0);
                    msgp = nullptr;
                    REQUIRE(::nng_recvmsg(pull, &msgp, 0) == 0);
                    REQUIRE(msgp != nullptr);
                    NNG_TESTS_CHECK_STR(msgp, again.c_str());
                    REQUIRE((p2 = nng_msg_get_pipe(msgp)) > 0);
                    ::nng_msg_free(msgp);
                    REQUIRE(p2 != p1);
                }
            }
        }
    }
}

TEST_CASE("NNG C++ wrapper reconnect works", Catch::Tags(
    "reconnect", "nng", "cxx").c_str()) {

    using namespace std;
    using namespace std::chrono;
    using namespace nng;
    using namespace nng::protocol;
    using namespace constants;
    using namespace Catch::Matchers;
    using _opt_ = option_names;

    // TODO: not sure if I like having "session" part of these unit tests, per se. I almost want session to be the subject of its own set of unit tests (it should be, anyway).
    // Which "at-exit" destructor handles cleaning up the NNG resources: i.e. ::nng_fini.
    session _session_;

    SECTION("We can create a pipeline") {

        std::shared_ptr<latest_push_socket> push;
        std::shared_ptr<latest_pull_socket> pull;

        REQUIRE_NOTHROW(push = _session_.create_push_socket());
        REQUIRE_NOTHROW(pull = _session_.create_pull_socket());

        const auto reconnect_time = 10ms;

        REQUIRE_NOTHROW(pull->GetOptions()->SetDuration(_opt_::min_reconnect_duration, reconnect_time));
        REQUIRE_NOTHROW(pull->GetOptions()->SetDuration(_opt_::max_reconnect_duration, reconnect_time));

        SECTION("Dialing before listening works") {

            REQUIRE_NOTHROW(push->Dial(test_addr, flag_nonblock));
            this_thread::sleep_for(100ms);
            REQUIRE_NOTHROW(pull->Listen(test_addr));

            /* 'Frame' in this case is loosely speaking. We do not care about exposing the NNG msg structure,
            per se. Rather, we should simply be able to "frame" buffers or strings, accordingly. */
            SECTION("We can send a frame") {

                // TODO: TBD: put additional require/checks around these..
                auto bmp = std::make_unique<binary_message>();
                this_thread::sleep_for(100ms);
                // And with a little C++ operator overloading help:
                REQUIRE_NOTHROW(*bmp << hello);
                REQUIRE_NOTHROW(push->Send(*bmp));
                /* Ditto message passing semantics. The Send() operation effectively
                nullifies the internal message. */
                REQUIRE(bmp->HasOne() == false);

                REQUIRE_NOTHROW(pull->TryReceive(bmp.get()));
                REQUIRE(bmp->HasOne() == true);
                // Just verify that the message matches the buffer.
                REQUIRE_THAT(bmp->GetBody()->Get(), Equals(hello_buf));
            }
        }

        SECTION("Reconnection works") {

            REQUIRE_NOTHROW(push->Dial(test_addr, flag_nonblock));

            {
                // We do not need to see the Listener beyond this block.
                auto lp = std::make_unique<listener>();
                REQUIRE_NOTHROW(pull->Listen(test_addr, lp.get()));
                this_thread::sleep_for(100ms);
                // No need to burden "session" with this one, either.
                REQUIRE_NOTHROW(lp.reset());
            }

            // Then re-listen to the address.
            REQUIRE_NOTHROW(pull->Listen(test_addr));

            // TODO: TBD: we may provide comprehension of nng_pipe from a nngcpp POV, but I'm not sure the complexity of nng_msg how that is different from a simple send/receive?
            SECTION("They still exchange frames") {

                auto bmp = std::make_unique<binary_message>();

                this_thread::sleep_for(100ms);
                REQUIRE_NOTHROW(*bmp << hello);
                REQUIRE_NOTHROW(push->Send(*bmp));
                REQUIRE(bmp->HasOne() == false);

                // See notes above. Sending transfers ownership of the internal message to NNG.
                REQUIRE_NOTHROW(pull->TryReceive(bmp.get()));
                REQUIRE(bmp->HasOne() == true);
                // Just verify that the message matches the buffer.
                REQUIRE_THAT(bmp->GetBody()->Get(), Equals(hello_buf));

                // TODO: TBD: this one deserves its own unit test as well so that we are not cluttering the integration tests
                message_pipe_fixture mp1(bmp.get());
                REQUIRE(mp1.HasOne() == true);
                // TODO: TBD: deserves its own unit testing...
                REQUIRE(mp1.get_pid() > 0);
                /* Resetting the message pointer is effectively the same, if stronger, for purposes
                of this unit test, without necessarily needing to expose the free method. */
                REQUIRE_NOTHROW(bmp.reset());
                REQUIRE(bmp.get() == nullptr);

                SECTION("Even after pipe close") {

                    // Get the fixtured PID for test purposes prior to closing.
                    auto mp1_pid = mp1.get_pid();
                    REQUIRE_NOTHROW(mp1.Close());
                    REQUIRE(mp1.HasOne() == false);

                    this_thread::sleep_for(100ms);
                    auto bmp2 = std::make_unique<binary_message>();
                    REQUIRE_NOTHROW(*bmp2 << again);
                    // TODO: TBD: send/no-message -> receive/message is a pattern that deserves its own focused unit test...
                    REQUIRE_NOTHROW(push->Send(*bmp2));
                    REQUIRE(bmp2->HasOne() == false);

                    REQUIRE_NOTHROW(pull->TryReceive(bmp2.get()));
                    REQUIRE(bmp2->HasOne() == true);
                    // Just verify that the message matches the buffer.
                    REQUIRE_THAT(bmp2->GetBody()->Get(), Equals(again_buf));

                    message_pipe_fixture mp2(bmp2.get());
                    REQUIRE(mp2.HasOne() == true);
                    // Ditto resetting the pointer versus exposing the free method.
                    REQUIRE_NOTHROW(bmp2.reset());
                    REQUIRE(bmp2.get() == nullptr);
                    REQUIRE(mp2.get_pid() != mp1_pid);
                }
            }
        }
    }
}
