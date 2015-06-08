#ifndef COREBLUETOOTHPERIPHERALPLUGIN_H
#define COREBLUETOOTHPERIPHERALPLUGIN_H

#include "peripheral/peripheralplugininterface.h"

namespace btle {
    namespace peripheral {
        namespace apple {
            class corebluetoothperipheralplugin: public peripheralplugininterface{
            public:

                corebluetoothperipheralplugin();

            public:

                unsigned int features();
                const std::string& name();
                int start();
                void stop();
                peripheralplugininterface& operator << (const service& srv);
                void start_advertising(int ms,const adv_fields& fields);
                void stop_advertising();
                std::vector<advertisement_data_type> supported_adtypes();
                std::string read_characteristic(device& central, characteristic& chr);
                void characteristic_written(device& central, characteristic& chr);
                std::string read_descriptor(device& central, characteristic& chr, descriptor& desc);
                std::string descriptor_written(device& central, characteristic& chr, descriptor& desc);
                void characteristic_notify_changed(device& central, characteristic& srv, bool notify);

            private:
            };
        }
    }
}

#endif // COREBLUETOOTHPERIPHERALPLUGIN_H

