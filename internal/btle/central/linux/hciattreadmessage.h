#ifndef HCIATTREADMESSAGE_H
#define HCIATTREADMESSAGE_H

#include "btle/central/linux/messagebase.h"
#include "btle_global.h"

namespace btle {
    namespace central {
        class centralpluginobserver;
        namespace linux_platform {
            class bluezperipheraldevice;
                class hciattreadmessage: public messagebase{
                public:

                    hciattreadmessage( int handle,
                                       centralpluginobserver* observer,
                                       uint16_t att_handle);

                public:

                    void process(bluezperipheraldevice* dev);

                private:

                    uint16_t att_handle_;
                    int handle_;
                };
            }
        }
    }

#endif // HCIATTREADMESSAGE_H

