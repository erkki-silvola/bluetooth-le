#ifndef HCIATTREADBYGROUPMESSAGE_H
#define HCIATTREADBYGROUPMESSAGE_H

#include "btle/central/linux/messagebase.h"
#include "btle/connectionparameters.h"

namespace btle {
    namespace central {
        class centralpluginobserver;
        namespace linux_platform {
            class bluezperipheraldevice;
                class hciattreadbygroupmessage: public messagebase{
                public:

                    hciattreadbygroupmessage(int handle,
                                             centralpluginobserver* observer,
                                             uint16_t start_handle,
                                             uint16_t end_handle);

                public:

                    void process(bluezperipheraldevice* dev);

                private:

                    uint16_t start_handle_;
                    uint16_t end_handle_;
                    int handle_;
                };
            }
        }
    }
#endif // HCIATTREADBYGROUPMESSAGE_H

