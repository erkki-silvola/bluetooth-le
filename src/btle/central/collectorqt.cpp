
#include "btle/central/collectorqt.h"

using namespace btle::central;

collectorqt::collectorqt(QObject* parent)
: QObject(parent)
{
}

collectorqt::~collectorqt()
{
}

void collectorqt::device_discovered_cb(device& dev)
{
    emit signal_device_discovered(&dev);
}

void collectorqt::device_service_value_updated_cb(device& dev, const gatt_services::gattservicebase* srv)
{
    emit signal_device_service_value_updated(&dev,srv);
}

void collectorqt::device_service_value_updated_cb(device& dev, const service& srv, const characteristic& chr, const std::string& data)
{
    emit signal_device_service_value_updated(&dev,&srv,&chr,&data);
}

void collectorqt::device_gatt_service_discovered_cb(device& dev, const gatt_services::gattservicebase *srv)
{
    emit signal_device_gatt_service_discovered(&dev,srv);
}

void collectorqt::device_service_discovery_failed_cb(device& dev, const service_list& services, const error& err)
{
    emit signal_device_service_discovery_failed(&dev,&services,&err);
}

void collectorqt::device_characteristic_discovery_failed_cb(device& dev, const service& srv, const chr_list& chrs, const error& err)
{
    emit signal_device_characteristic_discovery_failed(&dev,&srv,&chrs,&err);
}

void collectorqt::device_characteristic_read_cb(device& dev, const service& srv, const characteristic& chrs, const std::string& data, const error& err)
{
    emit signal_device_characteristic_read(&dev,&srv,&chrs,&data,&err);
}

void collectorqt::device_state_changed_cb(device& dev)
{
    emit signal_device_state_changed(&dev);
}

void collectorqt::device_connection_parameters_updated(device& dev)
{

}
