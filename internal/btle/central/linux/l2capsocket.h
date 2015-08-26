#ifndef L2CAPSOCKET_H
#define L2CAPSOCKET_H

#include "btle/bda.h"

#include <thread>

namespace btle {
    namespace central {
        namespace linux_platform {
            class l2capsocket{
            public:
                l2capsocket();
            public:
                int connect(const bda& addr, unsigned int channel_id);
                void disconnect();
                int write(const std::string& packet);
                std::string read_packet();
            private:
                int fd_;
            };
        }
    }
}

#endif // L2CAPSOCKET_H

