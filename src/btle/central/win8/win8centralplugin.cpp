
#include "btle/central/win8/win8centralplugin.h"
#include "btle/central/centralpluginregisterer.h"

using namespace btle::central::win8;
using namespace btle::central;
using namespace btle;

namespace {
    centralpluginregisterer<win8centralplugin> registration;
}

win8centralplugin::win8centralplugin(centralpluginobserver &observer)
: centralplugininterface(observer),
  base("","WIN8 central plugin implementation")
{

}

std::string win8centralplugin::name()
{
    return "WIN8_CENTRAL";
}

std::vector<device*>& win8centralplugin::devices()
{
    return devices_;
}

device* win8centralplugin::allocate_new_device(const bda& addr)
{
    return NULL;
}

int win8centralplugin::start()
{
    return 0;
}

void win8centralplugin::stop()
{

}

void win8centralplugin::start_scan(central_scan_parameters param, const uuid_list* services )
{

}

void win8centralplugin::stop_scan()
{

}

void win8centralplugin::connect_device(device& dev)
{

}

void win8centralplugin::disconnect_device(device& dev)
{

}

void win8centralplugin::cancel_pending_connection(device& dev)
{

}

void win8centralplugin::discover_services(device& dev)
{

}

void win8centralplugin::discover_characteristics(device& dev, const service& srv)
{

}

void win8centralplugin::read_characteristic_value(device& dev,const service& srv, const characteristic& chr)
{

}

void win8centralplugin::write_characteristic_value(device& dev,const service& srv, const characteristic& chr, const std::string& data, characteristic_properties type)
{

}

void win8centralplugin::set_characteristic_notify(device& dev,const service& srv, const characteristic& chr, bool notify)
{

}

void win8centralplugin::write_descriptor(device& dev, const service& srv, const characteristic& chr, descriptor& desc, bool notify)
{
    switch( desc.type() )
    {
        case CLIENT_CHARACTERISTIC_CONFIGURATION:
        {
            // TODO
            break;
        }
        case SERVER_CHARACTERISTIC_CONFIGURATION:
        {
            // TODO
            break;
        }
        default:
            break; // TODO throw not_implemented
    }
}


