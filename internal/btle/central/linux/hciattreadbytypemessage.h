#ifndef HCIATTREADBYTYPEMESSAGE_H
#define HCIATTREADBYTYPEMESSAGE_H
#include "btle/central/linux/messagebase.h"
#include "btle/connectionparameters.h"

namespace btle {
    namespace central {
        class centralpluginobserver;
        namespace linux_platform {
            class bluezperipheraldevice;
                class hciattreadbytypemessage: public messagebase{
                public:

                    hciattreadbytypemessage( int handle,
                                             centralpluginobserver* observer,
                                             uint16_t att_handle,
                                             uint16_t type);

                public:

                    void process(bluezperipheraldevice* dev);

                private:

                    uint16_t att_handle_;
                    uint16_t type_;
                    int handle_;
                };
            }
        }
    }

#endif // HCIATTREADBYTYPEMESSAGE_H

