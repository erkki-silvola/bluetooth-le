#ifndef QTCENTRALPLUGIN_H
#define QTCENTRALPLUGIN_H

#include "central/qt/qtperipheraldevice.h"

#include <central/centralplugininterface.h>
#include <QtBluetooth/QtBluetooth>
#include <QBluetoothDeviceDiscoveryAgent>

namespace btle {
    namespace central {
        namespace qt {
        class qtcenralplugin: public QObject,
                              public centralplugininterface{
            Q_OBJECT
            public:
                qtcenralplugin(centralpluginobserver& observer);
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

            public slots:

                void deviceDiscovered(const QBluetoothDeviceInfo&);
                void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error);
                void deviceScanFinished();

            private:

                qtperipheraldevice* find_device(const QBluetoothDeviceInfo& info);

            private:
                QBluetoothDeviceDiscoveryAgent* agent_;
            };
        }
    }
}

#endif // QTCENTRALPLUGIN_H

