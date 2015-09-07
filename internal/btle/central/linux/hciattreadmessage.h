#ifndef HCIATTREADMESSAGE_H
#define HCIATTREADMESSAGE_H

#include "btle/central/linux/messagebase.h"
#include "btle/service.h"

namespace btle {
    namespace central {
        class centralpluginobserver;
        namespace linux_platform {
            class bluezperipheraldevice;
                class hciattreadmessage: public messagebase{
                public:

                    hciattreadmessage( int handle,
                                       centralpluginobserver* observer,
                                       const service& srv,
                                       const characteristic& chr);

                public:

                    void process(bluezperipheraldevice* dev);

                private:

                    int handle_;
                    centralpluginobserver* observer_;
                    service srv_;
                    characteristic chr_;
                };
            }
        }
    }

#endif // HCIATTREADMESSAGE_H

