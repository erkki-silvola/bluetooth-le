
#include "central/qt/qtcentralplugin.h"
#include "central/centralpluginregisterer.h"
#include "btle/log.h"

using namespace btle;
using namespace btle::central;
using namespace btle::central::qt;

namespace {
    centralpluginregisterer<qtcenralplugin> registration;
}

qtcenralplugin::qtcenralplugin(centralpluginobserver& observer)
: centralplugininterface(observer)
{
    agent_ = new QBluetoothDeviceDiscoveryAgent();
    connect(agent_, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(deviceDiscovered(const QBluetoothDeviceInfo&)));
    connect(agent_, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(deviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(agent_, SIGNAL(finished()), this, SLOT(deviceScanFinished()));
}

std::string qtcenralplugin::name()
{
    return "QtBluetooth";
}

std::vector<device*>& qtcenralplugin::devices()
{
    return devices_;
}

device* qtcenralplugin::allocate_new_device(const bda& addr)
{
    return NULL;
}

int qtcenralplugin::start()
{
    observer_.plugin_state_changed(STATE_POWERED_ON);
    return 0;
}

void qtcenralplugin::stop()
{

}

void qtcenralplugin::start_scan( central_scan_parameters param, const uuid_list* services )
{
    agent_->start();
}

void qtcenralplugin::stop_scan()
{
    agent_->stop();
}

void qtcenralplugin::connect_device(device& dev)
{

}

void qtcenralplugin::disconnect_device(device& dev)
{

}

void qtcenralplugin::cancel_pending_connection(device& dev)
{

}

void qtcenralplugin::discover_services(device& dev)
{

}

void qtcenralplugin::discover_characteristics(device& dev, const service& srv)
{

}

void qtcenralplugin::read_characteristic_value(device& dev,const service& srv, const characteristic& chr)
{

}

void qtcenralplugin::write_characteristic_value(device& dev,const service& srv, const characteristic& chr, const std::string& data, characteristic_properties type)
{

}

void qtcenralplugin::set_characteristic_notify(device& dev,const service& srv, const characteristic& chr, bool notify)
{

}

void qtcenralplugin::write_descriptor(device& dev, const service& srv, const characteristic& chr, descriptor& desc, bool notify)
{

}

void qtcenralplugin::update_parameters(device& dev, const connectionparameters& params)
{

}

void qtcenralplugin::write_btle_ftp(device& dev, const std::string& buffer)
{

}

int qtcenralplugin::read_btle_ftp(device& dev, std::string& buffer, bool ack)
{

}

void qtcenralplugin::deviceDiscovered(const QBluetoothDeviceInfo& info)
{
    if(info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        _log("device discovered: %s",info.name().toStdString().c_str());
       btle::adv_fields fields;
       fields[btle::GAP_ADTYPE_LOCAL_NAME_COMPLETE]=advertisementdata(info.name().toStdString());
       //TODO add more
       qtperipheraldevice* dev = find_device(info);
       if(dev==NULL)
       {
            dev = new qtperipheraldevice(btle::bda(info.address().toString().toStdString()));
       }
       observer_.device_discovered(*dev,fields,info.rssi());
    }
}

void qtcenralplugin::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error err)
{
    _log("Error: %i",err);
}

void qtcenralplugin::deviceScanFinished()
{
    _log("Scan finished")
    agent_->start();
}

qtperipheraldevice* qtcenralplugin::find_device(const QBluetoothDeviceInfo& info)
{
    qtperipheraldevice* dev = NULL;
    for(device_iterator it = devices_.begin(); it != devices_.end(); ++it )
    {
        if( (*it)->addr() == info.address().toString().toStdString() )
        {
            return (qtperipheraldevice*)(*it);
        }
    }
    return dev;
}

