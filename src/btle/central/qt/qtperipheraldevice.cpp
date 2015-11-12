
#include <assert.h>

#include "btle/central/qt/qtperipheraldevice.h"
#include "btle/central/centralpluginobserver.h"
#include "btle/log.h"

using namespace btle::central::qt;

qtperipheraldevice::qtperipheraldevice(const btle::bda& addr, centralpluginobserver &observer)
: device(addr),
  observer_(observer),
  ctr_(0)
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
    ctr_ = 0;
    ctrl_->discoverServices();
}

void qtperipheraldevice::set_characteristic_notify(const btle::service& srv, const btle::characteristic& chr, bool notify)
{
    for(std::vector<QLowEnergyService*>::iterator it = qservices_.begin(); it != qservices_.end(); ++it)
    {
        if( (*it) == (QLowEnergyService*)srv.instance_id())
        {
            const QLowEnergyCharacteristic nchr = (*it)->characteristic(QBluetoothUuid(chr.uuid().uuid16bit()));
            if(nchr.isValid())
            {
                const QLowEnergyDescriptor desc = nchr.descriptor(
                                QBluetoothUuid::ClientCharacteristicConfiguration);
                if(desc.isValid())
                {
                    bool indicate = chr.properties() & btle::GATT_INDICATE;
                    QByteArray bytes = notify ? (!indicate ?
                                                 QByteArray::fromHex("0100") : QByteArray::fromHex("0200")) :
                                                (!indicate?
                                                 QByteArray::fromHex("0000") : QByteArray::fromHex("0100"));
                    (*it)->writeDescriptor(desc, bytes);
                }
            }
            break;
        }
    }
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
    _log("device gatt state changed: %d", s);
    switch (s)
    {
        case QLowEnergyService::ServiceDiscovered:
        {
            ++ctr_;
            if( qservices_.size() == ctr_ )
            {
                for(QLowEnergyService* it : qservices_)
                {
                    QList<QLowEnergyCharacteristic> chrs = it->characteristics();
                    btle::service nsrv = btle::service(btle::uuid((it)->serviceUuid().toString().toStdString()),(long)it);
                    const service* stored = db_.fetch_service(nsrv.uuid());
                    if(stored == NULL)
                    {
                        for( QLowEnergyCharacteristic& itc: chrs)
                        {
                            // TODO fix parent
                            btle::characteristic nchr = btle::characteristic(
                                        btle::uuid(itc.uuid().toString().toStdString()),
                                        itc.properties(),
                                        itc.handle(),
                                        itc.handle()+1,
                                        NULL);

                            QList<QLowEnergyDescriptor> ndescriptor = itc.descriptors();
                            for(QLowEnergyDescriptor& itd : ndescriptor)
                            {
                                btle::descriptor desc(itd.type(),
                                                      itd.handle(),
                                                      NULL);
                                nchr << desc;
                            }

                            nsrv << nchr;
                        }
                        db_ << nsrv;
                        btle::error err(0);
                        observer_.device_characteristics_discovered(*this,db_.services().back(),db_.services().back().characteristics(),err);
                    }
                 }
            }
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
    service* srv = db_.fetch_service_by_chr_handle(c.handle());
    characteristic* chr = db_.fetch_characteristic(c.handle());
    std::string bytes(value.toStdString());
    assert(srv && chr);
    if( chr->properties() & GATT_INDICATE ||
        chr->properties() & GATT_NOTIFY)
    {
        observer_.device_characteristic_notify_data_updated(*this,*srv,*chr,bytes);
    }
    else
    {
        btle::error err(0);
        observer_.device_characteristic_read(*this,*srv,*chr,bytes,err);
    }
}

void qtperipheraldevice::descriptor_written(
    const QLowEnergyDescriptor &d,
    const QByteArray &value)
{
    //btle::descriptor desc(d.handle());
    //const service* srv = db_.fetch_service_by_descriptor(desc);
    // TODO
}

void qtperipheraldevice::service_error(QLowEnergyService::ServiceError e)
{

}
