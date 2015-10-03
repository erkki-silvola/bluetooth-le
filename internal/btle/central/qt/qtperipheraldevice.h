#ifndef QTPERIPHERALDEVICE_H
#define QTPERIPHERALDEVICE_H

#include "btle/device.h"

namespace btle {
    namespace central {
        namespace qt {
            class qtperipheraldevice: public btle::device{
            public:
                qtperipheraldevice(const btle::bda& addr);

            private:

            };
        }
    }
}
#endif // QTPERIPHERALDEVICE_H

