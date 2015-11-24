#ifndef QTPERIPHERALDEVICE_H
#define QTPERIPHERALDEVICE_H

#include "btle/device.h"
#include <QLowEnergyController>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyService>

namespace btle {
    namespace central {
        class centralpluginobserver;
        namespace qt {
            class qtperipheraldevice: public QObject, public btle::device{
                Q_OBJECT
            public:
                qtperipheraldevice(const btle::bda& addr,centralpluginobserver& observer);
                void connect_device();
                void disconnect_device();
                void discover_services();
                void set_characteristic_notify(const btle::service& srv, const btle::characteristic& chr, bool notify);
                void read_characteristic(const btle::service& srv, const btle::characteristic& chr);

            public slots:

                void device_connected();
                void device_disconnected();

                void services_discovered(const QBluetoothUuid&);
                void service_discovery_done();
                void controller_error(QLowEnergyController::Error);

                //QLowEnergyService
                void service_state_changed(QLowEnergyService::ServiceState s);
                void characteristic_updated(const QLowEnergyCharacteristic &c,
                                          const QByteArray &value);
                void descriptor_written(const QLowEnergyDescriptor &d,
                                          const QByteArray &value);
                void characteristic_read(const QLowEnergyCharacteristic &info,
                                        const QByteArray &value);
                void characteristic_written(const QLowEnergyCharacteristic &info,
                                           const QByteArray &value);
                void service_error(QLowEnergyService::ServiceError e);

            private:

                centralpluginobserver& observer_;
                QLowEnergyController* ctrl_;
                std::vector<QBluetoothUuid> uuids_;
                std::vector<QLowEnergyService*> qservices_;
                int ctr_;
            };
        }
    }
}

#endif // QTPERIPHERALDEVICE_H
