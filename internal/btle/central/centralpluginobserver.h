#ifndef CENTRALPLUGINOBSERVER_H
#define CENTRALPLUGINOBSERVER_H

#include "btle/device.h"
#include "btle/btle_global.h"
#include "btle/error.h"

namespace btle {
    namespace central {
        class centralpluginobserver{
        public:
            virtual void plugin_state_changed(plugin_state state)=0;
            virtual void new_device_discovered(device& dev,adv_fields& fields,int rssi)=0;
            virtual void device_discovered(device& dev,adv_fields& fields,int rssi) = 0;
            virtual void device_connected(device& dev)=0;
            virtual void device_disconnected(device& dev)=0;
            virtual void device_services_discovered(device& dev, service_list& services, const error& err) = 0;
            virtual void device_characteristics_discovered(device& dev, service& srv, chr_list& chrs, const error& err) = 0;
            virtual void device_characteristic_read(device& dev, service& srv, characteristic& chr, std::string& data, const error& err) = 0;
            virtual void device_characteristic_written(device& dev, service& srv, characteristic& chr, const error& err) = 0;
            virtual void device_characteristic_nofication_state_changed(device& dev, service& srv, characteristic& chr, bool notify, const error& err) = 0;
            virtual void device_descriptor_written(device& dev, service& srv, characteristic& chr, descriptor& desc, const error& err) = 0;
            virtual void device_characteristic_notify_data_updated(device& dev, service& srv, characteristic& chr, std::string& data) = 0;
            virtual void device_rssi_read(device& dev, int rssi)=0;
            virtual void device_services_invalidated(device& dev)=0;
            virtual void device_connection_parameters_updated(device& dev)=0;
        };
    }
}

#endif // CENTRALPLUGINOBSERVER_H

