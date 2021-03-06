#ifndef NNGCPP_RECEIVER_H
#define NNGCPP_RECEIVER_H

#include "types.h"
#include "enums.h"

#include "async/basic_async_service.h"

#include "../messaging/messaging.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace nng {

    class IReceiver {
    protected:

        IReceiver();

    public:

        virtual ~IReceiver();

        // TODO: TBD: this one may get somewhat involved...
        virtual std::unique_ptr<binary_message> Receive(flag_type flags = flag_none) = 0;
        virtual bool TryReceive(binary_message* const bmp, flag_type flags = flag_none) = 0;

        virtual buffer_vector_type Receive(size_type& sz, flag_type flags = flag_none) = 0;
        virtual bool TryReceive(buffer_vector_type* const bufp, size_type& sz, flag_type flags = flag_none) = 0;

        // TODO: TBD: ditto ISender re: extending even through send-only API.
        virtual void ReceiveAsync(basic_async_service* const svcp) = 0;
    };
}

#endif // NNGCPP_RECEIVER_H
