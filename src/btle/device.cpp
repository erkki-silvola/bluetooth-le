
#include "btle/device.h"
#include "btle/gatt_services/gattservicefactory.h"

#include <sstream>

using namespace btle;
using namespace btle::gatt_services;

namespace{
    const char* state2string(connection_state state)
    {
        switch(state){
            case DEVICE_CONNECTED:          return "DEVICE_CONNECTED";
            case DEVICE_CONNECTING:         return "DEVICE_CONNECTING";
            case DEVICE_CONNECTION_PARK:    return "DEVICE_CONNECTION_PARK";
            case DEVICE_DISCONNECTED:       return "DEVICE_DISCONNECTED";
            case DEVICE_DISCONNECTING:      return "DEVICE_DISCONNECTING";
            case DEVICE_DISCONNECTION_PARK: return "DEVICE_DISCONNECTION_PARK";
            default: return "UNKNOWN";
        }
    }
}

device::device(const bda &addr)
: base(),
  advertisement_fields_(),
  name_(),
  db_(),
  state_(DEVICE_DISCONNECTED),
  bda_(addr),
  rssifilter_(),
  gatt_services_(),
  advertisement_type_(ADV_IND),
  statistics_()
{
    gattservicefactory::instance().populate(gatt_services_);
}

device::~device()
{
    gattservicefactory::instance().deplete(gatt_services_);
}

/**
 * @brief device::name, name from advertisement data, if any
 * @return
 */
const std::string& device::name()
{
    return advertisement_fields_.name();
}

gattdatabase& device::db()
{
    return db_;
}

const gattdatabase& device::db() const
{
    return db_;
}

rssifilter& device::rssi_filter()
{
    return rssifilter_;
}

connection_state device::state() const
{
    return state_;
}

std::string device::state_string() const
{
    return state2string(state_);
}

const bda& device::addr() const
{
    return bda_;
}

const gatt_services::gattservicebase* device::gatt_service(const uuid& uid) const
{
    for( gatt_service_iterator_const it = gatt_services_.begin(); it != gatt_services_.end(); ++it )
    {
        if( (*it)->service_uuid() == uid )
        {
            return (const gattservicebase*)(*it);
        }
    }
    return NULL;
}

gatt_services::gattservicebase* device::gatt_service(const uuid& uid)
{
    for( gatt_service_iterator it = gatt_services_.begin(); it != gatt_services_.end(); ++it )
    {
        if( (*it)->service_uuid() == uid )
        {
            return (gattservicebase*)(*it);
        }
    }
    return NULL;
}

advertisementfields& device::advertisement_fields()
{
    return advertisement_fields_;
}

connectionparameters& device::parameters()
{
    return parameters_;
}

bool device::operator == (const device& other) const
{
    return bda_ == other.addr();
}

btle::advertisement_type device::advertisement_type() const
{
    return advertisement_type_;
}

void device::clear_device_data(bool verbose)
{
    for( gatt_service_iterator it = gatt_services_.begin(); it != gatt_services_.end(); ++it )
    {
        (*it)->reset();
    }
    if( verbose )
    {
        device::invalidate();
    }
}

uint16_t device::connection_handle()
{
    return connection_handle_;
}

void device::set_connection_handle(uint16_t handle)
{
    connection_handle_ = handle;
}

std::string device::description() const
{
    std::stringstream ss;
    ss << "Device addr: " << bda_.description() << "\n In connection state: " << state_string();
    return ss.str();
}

void device::set_state(connection_state state)
{
    state_ = state;
}

void device::invalidate()
{
    base::invalidate();
    bda_.invalidate();
    parameters_.invalidate();
    advertisement_fields_.invalidate();
    db_.invalidate();
}

/**
 * @brief device::mutex, shall allways be used when accessing
 * device content e.g.
 *
 * dev.mutex().lock()
 * int rssi = dev.rssi_filter().current();
 * dev.mutex().unlock();
 *
 * @return
 */
std::mutex& device::mutex()
{
    return mutex_;
}

