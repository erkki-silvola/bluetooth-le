#ifndef HCIATTWRITEMESSAGE_H
#define HCIATTWRITEMESSAGE_H

#include "btle/central/linux/messagebase.h"
#include "btle/service.h"

namespace btle {
    namespace central {
        class centralpluginobserver;
        namespace linux_platform {
            class bluezperipheraldevice;
                class hciattwritemessage: public messagebase{
                public:

                    hciattwritemessage( int handle,
                                        centralpluginobserver* observer,
                                        const service& srv,
                                        const characteristic& chr);

                    hciattwritemessage( int handle,
                                        centralpluginobserver* observer,
                                        const service& srv,
                                        const characteristic& chr,
                                        const descriptor& desc);

                public:

                    void process(bluezperipheraldevice* dev);

                private:

                    enum write_type{
                        characteristic_write,
                        descriptor_write
                    };

                    int handle_;
                    centralpluginobserver* observer_;
                    service srv_;
                    characteristic chr_;
                    descriptor desc_;
                    write_type type_;
                };
            }
        }
    }


#endif // HCIATTWRITEMESSAGE_H

