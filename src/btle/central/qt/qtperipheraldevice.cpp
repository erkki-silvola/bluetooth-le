
#include "btle/central/qt/qtperipheraldevice.h"

using namespace btle::central::qt;

qtperipheraldevice::qtperipheraldevice(const btle::bda& addr)
: device(addr)
{
}

void qtperipheraldevice::discover_services()
{
    ctrl_ = new QLowEnergyController(QBluetoothAddress(QString::fromStdString(bda_.to_string())));
    connect(ctrl_, SIGNAL(serviceDiscovered(QBluetoothUuid)),
            this, SLOT(services_discovered(QBluetoothUuid)));
    connect(ctrl_, SIGNAL(discoveryFinished()),
            this, SLOT(service_discovery_done()));
    connect(ctrl_, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(controller_error(QLowEnergyController::Error)));
}

void qtperipheraldevice::services_discovered(const QBluetoothUuid& gatt)
{
    uuids_.push_back(gatt);
}

void qtperipheraldevice::service_discovery_done()
{
    for( std::vector<QBluetoothUuid>::const_iterator it = uuids_.begin(); it != uuids_.end(); ++it )
    {
        QLowEnergyService* srv = ctrl_->createServiceObject(*it,NULL);
        qservices_.push_back(srv);
        btle::service nsrv = btle::service(btle::uuid(srv->serviceUuid().toString().toStdString()));
        db_ << nsrv;
        srv->discoverDetails();
    }
}

void qtperipheraldevice::controller_error(
    QLowEnergyController::Error err)
{

}

void qtperipheraldevice::service_state_changed(
    QLowEnergyService::ServiceState s)
{
    switch (s)
    {
        case QLowEnergyService::ServiceDiscovered:
        {
             /*const QLowEnergyCharacteristic hrChar = m_service->characteristic(
                         QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement));
             if (!hrChar.isValid()) {
                 setMessage("HR Data not found.");
                 break;
             }

             const QLowEnergyDescriptor m_notificationDesc = hrChar.descriptor(
                         QBluetoothUuid::ClientCharacteristicConfiguration);
             if (m_notificationDesc.isValid()) {
                 m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
                 setMessage("Measuring");
                 m_start = QDateTime::currentDateTime();
             }*/

             QBluetoothUuid::HeartRateMeasurement;

             break;
         }
         default:
             //nothing for now
             break;
    }
}

void qtperipheraldevice::characteristic_updated(
    const QLowEnergyCharacteristic &c,
    const QByteArray &value)
{

}

void qtperipheraldevice::descriptor_written(
    const QLowEnergyDescriptor &d,
    const QByteArray &value)
{
    btle::descriptor desc(d.handle());
    const service* srv = db_.fetch_service_by_descriptor(desc);
    // TODO
}

void qtperipheraldevice::service_error(QLowEnergyService::ServiceError e)
{

}
