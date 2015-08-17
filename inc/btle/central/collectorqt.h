#ifndef COLLECTORQT_H
#define COLLECTORQT_H

#include <QObject>
#include <QVector>
#include "btle/central/collector.h"

namespace btle {
    namespace central {
        class BTLE_API collectorqt: public QObject, public collector{
        Q_OBJECT

        public:
            explicit collectorqt(QObject* parent=NULL);
            ~collectorqt();

        public:

            void device_discovered_cb(device& dev);
            void device_service_value_updated_cb(device& dev, const gatt_services::gattservicebase* srv);
            void device_service_value_updated_cb(device& dev, const service& srv, const characteristic& chr, const std::string& data);
            void device_gatt_service_discovered_cb(device& dev, const gatt_services::gattservicebase *srv);
            void device_service_discovery_failed_cb(device& dev, const service_list& services, const error& err);
            void device_characteristic_discovery_failed_cb(device& dev, const service& srv, const chr_list& chrs, const error& err);
            void device_characteristic_read_cb(device& dev, const service& srv, const characteristic& chrs, const std::string& data, const error& err);
            void device_state_changed_cb(device& dev);
            void device_connection_parameters_updated(device& dev);

        signals:

            void signal_device_discovered(device* dev);
            void signal_device_service_value_updated(device* dev, const gatt_services::gattservicebase* srv);
            void signal_device_service_value_updated(device* dev, const service* srv, const characteristic* chr, const std::string* data);
            void signal_device_gatt_service_discovered(device* dev, const gatt_services::gattservicebase *srv);
            void signal_device_service_discovery_failed(device* dev, const service_list* services, const error* err);
            void signal_device_characteristic_discovery_failed(device* dev, const service* srv, const chr_list* chrs, const error* err);
            void signal_device_characteristic_read(device* dev, const service* srv, const characteristic* chrs, const std::string* data, const error* err);
            void signal_device_state_changed(device* dev);

        private: // data

        };
    }
}

#endif // COLLECTORQT_H

