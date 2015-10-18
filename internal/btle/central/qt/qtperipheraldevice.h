#ifndef QTPERIPHERALDEVICE_H
#define QTPERIPHERALDEVICE_H

#include "btle/device.h"
#include <QLowEnergyController>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyService>

namespace btle {
    namespace central {
        namespace qt {
            class qtperipheraldevice: public QObject, public btle::device{
                Q_OBJECT
            public:
                qtperipheraldevice(const btle::bda& addr);

            public slots:

                void services_discovered(const QBluetoothUuid&);
                void service_discovery_done();
                void controller_error(QLowEnergyController::Error);

                //QLowEnergyService
                void service_state_changed(QLowEnergyService::ServiceState s);
                void characteristic_updated(const QLowEnergyCharacteristic &c,
                                          const QByteArray &value);
                void descriptor_written(const QLowEnergyDescriptor &d,
                                          const QByteArray &value);
                void service_error(QLowEnergyService::ServiceError e);

            private:

                QLowEnergyController* ctrl_;
                std::vector<QBluetoothUuid> uuids_;
                std::vector<QLowEnergyService*> qservices_;
            };
        }
    }
}

#endif // QTPERIPHERALDEVICE_H
