
#include "btle/central/qt/qtperipheraldevice.h"

using namespace btle::central::qt;

qtperipheraldevice::qtperipheraldevice(const btle::bda& addr)
: device(addr)
{
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

}

void qtperipheraldevice::service_error(QLowEnergyService::ServiceError e)
{

}
