
#include "btle/central/qt/qtperipheraldevice.h"
#include "btle/central/centralpluginobserver.h"
#include "btle/log.h"

using namespace btle::central::qt;

qtperipheraldevice::qtperipheraldevice(const btle::bda& addr, centralpluginobserver &observer)
: device(addr),
  observer_(observer)
{
    ctrl_ = new QLowEnergyController(QBluetoothAddress(QString::fromStdString(bda_.to_string())));
    connect(ctrl_, SIGNAL(serviceDiscovered(QBluetoothUuid)),
            this, SLOT(services_discovered(QBluetoothUuid)));
    connect(ctrl_, SIGNAL(discoveryFinished()),
            this, SLOT(service_discovery_done()));
    connect(ctrl_, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(controller_error(QLowEnergyController::Error)));
    connect(ctrl_, SIGNAL(connected()),
                this, SLOT(device_connected()));
    connect(ctrl_, SIGNAL(disconnected()),
            this, SLOT(device_disconnected()));
}

void qtperipheraldevice::connect_device()
{
    ctrl_->connectToDevice();
}

void qtperipheraldevice::disconnect_device()
{
    ctrl_->disconnectFromDevice();
}

void qtperipheraldevice::discover_services()
{
    ctrl_->discoverServices();
}

void qtperipheraldevice::device_connected()
{
    observer_.device_connected(*this);
}

void qtperipheraldevice::device_disconnected()
{
    observer_.device_disconnected(*this);
}

void qtperipheraldevice::services_discovered(const QBluetoothUuid& gatt)
{
    _log("service discovered: %s", gatt.toString().toLocal8Bit().data());
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
        connect(srv, SIGNAL(stateChanged(QLowEnergyService::ServiceState)),
                this, SLOT(service_state_changed(QLowEnergyService::ServiceState)));
        connect(srv, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                this, SLOT(characteristic_updated(QLowEnergyCharacteristic,QByteArray)));
        connect(srv, SIGNAL(descriptorWritten(QLowEnergyDescriptor,QByteArray)),
                this, SLOT(descriptor_written(QLowEnergyDescriptor,QByteArray)));
        srv->discoverDetails();
    }
}

void qtperipheraldevice::controller_error(
    QLowEnergyController::Error err)
{
    _log_error(" %d",err);
}

void qtperipheraldevice::service_state_changed(
    QLowEnergyService::ServiceState s)
{
    _log("state changed: %d", s);
    switch (s)
    {
        case QLowEnergyService::ServiceDiscovered:
        {
            for(std::vector<QLowEnergyService*>::iterator it = qservices_.begin(); it != qservices_.end(); ++it)
            {
                // QList<QLowEnergyCharacteristic> characteristics()
                for(QList<QLowEnergyCharacteristic>::iterator itc = (*it)->characteristics().begin(); itc != (*it)->characteristics().end(); ++itc )
                {

                }
               // _log("service discovered: %s", (*it)->toString().toLocal8Bit().data());
            }
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
