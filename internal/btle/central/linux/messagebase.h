#ifndef MESSAGEBASE_H
#define MESSAGEBASE_H

#include <deque>

namespace btle {
    namespace central {
        class centralpluginobserver;
        namespace linux_platform {
            class bluezperipheraldevice;
            class messagebase{
            public:
                virtual ~messagebase();
                virtual void process(bluezperipheraldevice* dev)=0;
            };

            typedef std::deque<messagebase*> messages;
        }
    }
}

#endif // MESSAGEBASE_H

