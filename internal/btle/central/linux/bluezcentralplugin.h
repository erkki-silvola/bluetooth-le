#ifndef BLUEZCENTRALPLUGIN_H
#define BLUEZCENTRALPLUGIN_H

#include "btle/central/linux/bluezperipheraldevice.h"
#include "btle/central/centralplugininterface.h"

#include <bluetooth/bluetooth.h>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace btle {
    namespace central {
        namespace linux_platform {
            class bluezcentralplugin: public centralplugininterface, public base{
            public:
                bluezcentralplugin(centralpluginobserver& observer);

            public:

               void scan_routine();

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
                void update_parameters(device& dev, const connectionparameters& params);
                void write_btle_ftp(device& dev, const std::string& buffer);
                int read_btle_ftp(device& dev, std::string& buffer, bool ack);

            private:

                bluezperipheraldevice* find_device(const bda& addr);

            private:

                int id_;
                int handle_;
                std::thread main_;
                std::mutex mutex_;
                std::condition_variable started_;
            };
        }
    }
}

#endif // BLUEZCENTRALPLUGIN_H

