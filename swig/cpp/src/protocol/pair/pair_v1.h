#ifndef CPPNNG_PROT_PAIR_V1_H
#define CPPNNG_PROT_PAIR_V1_H

#include "../../core/socket.h"

namespace nng {

    namespace protocol {
        
        namespace v1 {
            
            class pair_socket : public _Socket {
                public:

                    pair_socket();

                    virtual ~pair_socket();
            };
        }

        typedef v1::pair_socket latest_pair_socket;
    }
}

#endif // CPPNNG_PROT_PAIR_V1_H
