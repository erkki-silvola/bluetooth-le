#ifndef WIN8CENTRALPLUGIN_H
#define WIN8CENTRALPLUGIN_H

#include "btle/central/centralplugininterface.h"

namespace btle {
    namespace central {
        namespace win8 {
            class win8centralplugin: public centralplugininterface, public base{
            public:
                win8centralplugin(centralpluginobserver& observer);

            public: // from base

                std::string name();
                std::vector<device*>& devices();
                device* allocate_new_device(const bda& addr);
                int start();
                void stop();
                void start_scan( central_scan_parameters param = SCAN_HIGH_DUTY, const uuid_list* services = NULL );
                void stop_scan();
                void connect_device(device& dev);
                void disconnect_device(device& dev);
                void cancel_pending_connection(device& dev);
                void discover_services(device& dev);
                void discover_characteristics(device& dev, const service& srv);
                void read_characteristic_value(device& dev,const service& srv, const characteristic& chr);
                void write_characteristic_value(device& dev,const service& srv, const characteristic& chr, const std::string& data, characteristic_properties type);
                void set_characteristic_notify(device& dev,const service& srv, const characteristic& chr, bool notify);
                void write_descriptor(device& dev, const service& srv, const characteristic& chr, descriptor& desc, bool notify);

            private:

                //device_list devices_;
            };
        }
    }
}

#endif // WIN8CENTRALPLUGIN_H

