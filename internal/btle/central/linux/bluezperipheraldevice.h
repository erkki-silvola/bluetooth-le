#ifndef BLUEZPERIPHERALDEVICE_H
#define BLUEZPERIPHERALDEVICE_H

#include "btle/device.h"
#include "btle/central/linux/messagebase.h"
#include "btle/central/linux/l2capsocket.h"
#include "btle/atomiclist.h"

#include <mutex>
#include <condition_variable>

namespace btle {
    namespace central {
        namespace linux_platform {
            class bluezperipheraldevice: public device
            {
            public:
                bluezperipheraldevice(const btle::bda& addr);

            public: // message push

                void push(messagebase* message);

            public:

                void message_thread();

            private:

                std::thread thread_;
                messages    queue_;
                std::mutex  q_mutex_;
                std::condition_variable q_condition_;
                std::condition_variable started_;
                btle::atomiclist<messagebase*> messages_;
            public:
                l2capsocket att_socket_;
            };
        }
    }
}

#endif // BLUEZPERIPHERALDEVICE_H

