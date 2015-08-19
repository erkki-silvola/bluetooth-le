#include <algorithm>

#include "btle/exceptions/attribute_not_found.h"
#include "btle/exceptions/attribute_not_readable.h"
#include "btle/exceptions/attribute_not_writable.h"
#include "btle/exceptions/device_not_connected.h"
#include "btle/exceptions/service_not_found.h"
#include "btle/exceptions/not_supported.h"

#include "btle/central/collector.h"
#include "btle/central/centralpluginfactory.h"
#include "btle/gatt_services/gattservicefactory.h"
#include "btle/verify.h"
#include "btle/log.h"

#include "btle/bdascanfilter.h"
#include "btle/uuidscanfilter.h"

#include "btle/gatt_services/btlelibservice.h"

namespace {
    enum collector_flags{
        CLIENT_SCAN   = 0x01,
        INTERNAL_SCAN = 0x02,
        
    };

    // allways enable service changed notification ?
    #define SERVICE_CHANGED                             0x2A05

    // commonly read these chrs
    #define DEVICE_NAME                                 0x2A00
    #define APPEARANCE                                  0x2A01
    #define BATTERY_LEVEL                               0x2A19
    #define MODEL_NUMBER                                0x2A24
    #define SERIAL_NUMBER                               0x2A25
    #define FIRMWARE_REVISION                           0x2A26
    #define HARDWARE_REVISION                           0x2A27
    #define SOFTWARE_REVISION                           0x2A28
    #define MANUFACTURER_NAME                           0x2A29

    bool compare_ascent(btle::device* a, btle::device* b){
        return a->rssi_filter().mean_median() > b->rssi_filter().mean_median();
    }
    bool compare_descent(btle::device* a, btle::device* b){
        return a->rssi_filter().mean_median() < b->rssi_filter().mean_median();
    }
}

using namespace btle::central;
using namespace btle::gatt_services;

collector::collector()
: notify_uuids_(),
  read_uuids_(),
  plugin_(NULL),
  connectionhandler_(),
  flags_(0),
  filters_(),
  state_(STATE_POWERED_UNKNOWN),
  plugins_available_()
{
    centralpluginfactory::instance().populate(plugins_, *this);
    for( std::vector<centralplugininterface*>::const_iterator it = plugins_.begin(); it != plugins_.end(); ++it )
    {
        plugins_available_.push_back((*it)->name());
    }
    _log("Central plugins count: %i",plugins_.size());
    gatt_service_list services;
    gattservicefactory::instance().populate(services);
    _log("Build-in gatt services count: %i",services.size());
    // setup automatically all included gatt services notifications
    for( gatt_service_iterator it = services.begin(); it != services.end(); ++it )
    {
        for( uuid_iterator_const it_uuid = (*it)->mandatory_notifications().begin();
             it_uuid != (*it)->mandatory_notifications().end(); ++it_uuid )
        {
            notify_uuids_.push_back(*it_uuid);
        }
    }
    gattservicefactory::instance().deplete(services);
    connectionhandler_.attach(this);

    read_uuids_.push_back(uuid(DEVICE_NAME));
    read_uuids_.push_back(uuid(APPEARANCE));
    read_uuids_.push_back(uuid(BATTERY_LEVEL));
    read_uuids_.push_back(uuid(MODEL_NUMBER));
    read_uuids_.push_back(uuid(SERIAL_NUMBER));
    read_uuids_.push_back(uuid(FIRMWARE_REVISION));
    read_uuids_.push_back(uuid(HARDWARE_REVISION));
    read_uuids_.push_back(uuid(SOFTWARE_REVISION));
    read_uuids_.push_back(uuid(MANUFACTURER_NAME));
    // TODO add allso service chrs to read
    
}

collector::~collector()
{
    stop();
    for( scan_filters::iterator it = filters_.begin(); it != filters_.end(); ++it )
    {
        delete (*it);
    }
}

/**
 * @brief collector::plugins_available
 * @return string list of plugins available, hopefully all plugins has unique names
 */
const std::vector<std::string>& collector::plugins_available() const
{
    return plugins_available_;
}

/**
 * @brief collector::start
 * @param plugin_name
 * @return 0 plugin started succesfully or plugin error code
 */
int collector::start(const std::string& plugin_name)
{
    stop();
    for( std::vector<centralplugininterface*>::iterator it = plugins_.begin(); it != plugins_.end(); ++it )
    {
        if( (*it)->name().compare(plugin_name) != std::string::npos )
        {
            plugin_ = (*it);
            if( int err = plugin_->start() )
            {
                plugin_ = NULL;
                return err;
            }
            return 0;
        }
    }
    return -1;
}

/**
 * @brief collector::auto_start, auto start takes first plugin from the list
 * @return 0 or plugin error code
 */
int collector::auto_start()
{
    assert(plugins_.size());
    stop();
    plugin_ = plugins_[0];
    connectionhandler_.setup(&plugin_->devices(),this,this);
    if( int err = plugin_->start() )
    {
        plugin_ = NULL;
        return err;
    }
    return 0;
}

/**
 * @brief collector::stop, stops current plugin this may take time, varies by plugin
 */
void collector::stop()
{
    if(plugin_)
    {
        plugin_->stop();
        plugin_ = NULL;
    }
}


/**
 * @brief collector::add_scan_filter,
 * adds abstract scan filter, library has build-in filters which can be used @see uuidscanfilter ,
 * @see bdascanfilter and @see rssiscanfilter
 * @param filter, collector takes ownership
 */
void collector::add_scan_filter(scanfilterbase* filter)
{
    filters_.push_back(filter);
}

/**
 * @brief collector::clear_scan_filter
 * @param filter
 */
void collector::remove_scan_filter(scanfilterbase* filter)
{
    for( scan_filters::iterator it = filters_.begin(); it != filters_.end(); ++it )
    {
        if( (*it) == filter )
        {
            delete (*it);
            filters_.erase(it);
            return;
        }
    }
}

/**
 * @brief collector::clear_scan_filters
 */
void collector::clear_scan_filters()
{
    filters_.clear();
}

/**
 * @brief collector::start_scan
 */
void collector::start_scan()
{
    verify(plugin_)
    flags_ |= CLIENT_SCAN;
    if( state_ == STATE_POWERED_ON ) plugin_->start_scan();
    else _log_warning("BT STATE NOT POWERED!");
}

/**
 * @brief collector::stop_scan
 */
void collector::stop_scan()
{
    verify(plugin_)
    flags_ &= ~CLIENT_SCAN;
    // should be safe to call stop even if not powered
    plugin_->stop_scan();
}

/**
 * @brief collector::connection_handler
 * @return
 */
connectionhandler& collector::connection_handler()
{
    return connectionhandler_;
}

/**
 * @brief collector::devices_in_order, device list in ordered order
 * @param rssi_limit
 * @param ascent
 * @return device list in requested order, device ownership is not transferred
 */
btle::device_list collector::devices_in_order(int rssi_limit,bool ascent) const
{
    verify(plugin_)
    device_list list;
    for ( device_list::const_iterator it = plugin_->devices().begin(); it != plugin_->devices().end(); ++it )
    {
        if( (*it)->rssi_filter().mean_median() >= rssi_limit  )
            list.push_back(*it);
    }
    std::sort(list.begin(), list.end(), ascent ? ::compare_ascent : ::compare_descent);
    return list;
}

/**
 * @brief collector::set_auto_read_values,
 *        setup characteristic uuids to be read if found,
 *        NOTE this will override the default set!
 * @param list
 */
void collector::set_auto_read_values(const uuid_list& list)
{
    read_uuids_ = list;
}

/**
 * @brief collector::set_auto_notify_values, setup notification/indications to be enabled automatically
 *                                           note this overrides default set , which has been loaded from gatt service                   
 *                                           factory, use this functionality with
 *                                           extra care
 * @param list
 */
void collector::set_auto_notify_values(const uuid_list& list)
{
    notify_uuids_ = list;
}

/**
 * @brief collector::connect_device, for most basic use of the lib you only need
 *                                   to call this function after start, and wait the device is connected
 *                                   and start receiving heart rate etc...
 * @param addr
 */
void collector::connect_device(const bda& addr)
{
    verify(plugin_)
    device* dev(fetch_device(addr));
    if( !dev )
    {
        dev = plugin_->allocate_new_device(addr);
        assert(dev);
        plugin_->devices().push_back(dev);
    }
    connectionhandler_.connect_device(*dev);
}

/**
 * @brief collector::connect_device
 * @param dev
 */
void collector::connect_device(device& dev)
{
    verify(plugin_)
    connectionhandler_.connect_device(dev);
}

/**
 * @brief collector::disconnect_device
 * @param dev
 */
void collector::disconnect_device(device& dev)
{
    verify(plugin_)
    connectionhandler_.disconnect_device(dev);
}

/**
 * @brief collector::disconnect_device
 * @param addr
 */
void collector::disconnect_device(const bda& addr)
{
    verify(plugin_)
    if( device* dev = fetch_device(addr) )
    {
        // fail safe check that device exists
        connectionhandler_.disconnect_device(*dev);
    }
}

void collector::set_mode(collector_mode mode)
{
    switch(mode)
    {
        case COLLECTOR_POWER_SAVE_OFF:
        {
            // TODO
            break;
        }
        case COLLECTOR_POWER_SAVE_ON:
        {
            // TODO
            break;
        }
    }
}

/**
 * @brief collector::read_characteristic_value
 * @param dev
 * @param uid
 * @throws device_not_connected, device state is something else than CONNECTED
 */
void collector::read_characteristic_value(device& dev, const uuid& uid)
{
    verify(plugin_)
    if( dev.state() == btle::DEVICE_CONNECTED )
    {
        for( btle::service_iterator its = dev.db().services().begin(); its != dev.db().services().end(); ++its )
        {
            for(chr_iterator it_chr = its->characteristics().begin(); it_chr != its->characteristics().end(); ++it_chr )
            {
                if( it_chr->uuid() == uid )
                {
                    plugin_->read_characteristic_value(dev,*its,*it_chr);
                }
            }
        }
        if( dev.db().fetch_characteristic(uid) ) return;
        throw btle::exceptions::attribute_not_found("device does not contain this uuid");
    }
    throw btle::exceptions::device_not_connected("device not connected device is in state: " + dev.state_string());
}

/**
 * @brief collector::read_characteristic_value
 * @param dev
 * @param pair
 */
void collector::read_characteristic_value(device& dev, const uuid_pair& pair)
{
    verify(plugin_)
    if( dev.state() == btle::DEVICE_CONNECTED )
    {
        if( const characteristic* chr = dev.db().fetch_characteristic(pair) )
        {
            if( chr->properties() & btle::GATT_READ )
            {
                const service* srv = dev.db().fetch_service(pair.first);
                verify( srv );
                plugin_->read_characteristic_value(dev,*srv,*chr);
                return;
            }
            throw btle::exceptions::attribute_not_readable("attribute cannot be read");
        }
        throw btle::exceptions::attribute_not_found("device does not contain this uuid");
    }
    throw btle::exceptions::device_not_connected("device not connected device is in state: " + dev.state_string());
}

/**
 * @brief collector::write_characteristic_value
 * @param dev
 * @param uid
 */
void collector::write_characteristic_value(device& dev, const uuid& uid, const std::string& data, bool write_with_out_resp)
{
    verify(plugin_)
    if( dev.state() == btle::DEVICE_CONNECTED )
    {
        if( const characteristic* chr = dev.db().fetch_characteristic(uid) )
        {
            if( !write_with_out_resp )
            {
                if( chr->properties() & btle::GATT_WRITE )
                {
                    const service* srv = dev.db().fetch_service_by_chr_uuid(uid);
                    verify( srv != NULL );
                    plugin_->write_characteristic_value(dev,*srv,*chr,data,btle::GATT_WRITE);
                    return;
                }
            }
            if( chr->properties() & btle::GATT_WRITE_WITHOUT_RESPONSE )
            {
                const service* srv = dev.db().fetch_service_by_chr_uuid(uid);
                verify( srv );
                plugin_->write_characteristic_value(dev,*srv,*chr,data,btle::GATT_WRITE_WITHOUT_RESPONSE);
                return;
            }
            throw btle::exceptions::attribute_not_writable("attribute cannot be written");
        }
        throw btle::exceptions::attribute_not_found("device does not contain this uuid");
    }
    throw btle::exceptions::device_not_connected("device not connected device is in state: " + dev.state_string());
}

/**
 * @brief collector::write_characteristic_value
 * @param dev
 * @param pair
 */
void collector::write_characteristic_value(device& dev, const uuid_pair& pair, const std::string& data, bool write_with_out_resp)
{
    verify(plugin_)
    if( dev.state() == btle::DEVICE_CONNECTED )
    {
        if( const characteristic* chr = dev.db().fetch_characteristic(pair.second) )
        {
            if( !write_with_out_resp )
            {
                if( chr->properties() & btle::GATT_WRITE )
                {
                    const service* srv = dev.db().fetch_service(pair.first);
                    verify( srv );
                    plugin_->write_characteristic_value(dev,*srv,*chr,data,btle::GATT_WRITE);
                    return;
                }
            }
            if( chr->properties() & btle::GATT_WRITE_WITHOUT_RESPONSE )
            {
                const service* srv = dev.db().fetch_service(pair.first);
                verify( srv );
                plugin_->write_characteristic_value(dev,*srv,*chr,data,btle::GATT_WRITE_WITHOUT_RESPONSE);
                return;
            }
            throw btle::exceptions::attribute_not_writable("attribute cannot be written");
        }
        throw btle::exceptions::attribute_not_found("device does not contain this uuid");
    }
    throw btle::exceptions::device_not_connected("device not connected device is in state: " + dev.state_string());
}

/**
 * @brief collector::set_characteristic_notify
 * @param dev
 * @param uid
 * @param notify
 */
void collector::set_characteristic_notify(device& dev, const uuid& uid, bool notify)
{
    verify(plugin_)
    verify(plugin_)
    if( dev.state() == btle::DEVICE_CONNECTED )
    {
        if( const characteristic* chr = dev.db().fetch_characteristic(uid) )
        {
            if( chr->properties() & btle::GATT_READ )
            {
                const service* srv = dev.db().fetch_service_by_chr_uuid(uid);
                verify( srv );
                // TODO fix descriptor
                descriptor desc(CLIENT_CHARACTERISTIC_CONFIGURATION,NULL);
                plugin_->write_descriptor(dev,*srv,*chr,desc,notify);
                return;
            }
            throw btle::exceptions::attribute_not_readable("attribute cannot be read");
        }
        throw btle::exceptions::attribute_not_found("device does not contain this uuid");
    }
    throw btle::exceptions::device_not_connected("device not connected device is in state: " + dev.state_string());
}

/**
 * @brief collector::set_characteristic_notify
 * @param dev
 * @param pair
 * @param notify
 */
void collector::set_characteristic_notify(device& dev, const uuid_pair& pair, bool notify)
{
    verify(plugin_)
    if( dev.state() == btle::DEVICE_CONNECTED )
    {
        if( const characteristic* chr = dev.db().fetch_characteristic(pair) )
        {
            if( chr->properties() & btle::GATT_READ )
            {
                const service* srv = dev.db().fetch_service(pair.first);
                verify( srv );
                plugin_->read_characteristic_value(dev,*srv,*chr);
                return;
            }
            throw btle::exceptions::attribute_not_readable("attribute cannot be read");
        }
        throw btle::exceptions::attribute_not_found("device does not contain this uuid");
    }
    throw btle::exceptions::device_not_connected("device not connected device is in state: " + dev.state_string());
}

/**
 * @brief update_connection_parameters
 * @param dev
 * @param params
 */
void collector::update_connection_parameters(btle::device& dev, const connectionparameters& params)
{
    verify(plugin_)
    if( dev.state() == btle::DEVICE_CONNECTED )
    {
        if( plugin_->features() & btle::central::PLUGIN_CONNECTION_PARAMETER_CAPABILITY )
        {
            plugin_->update_parameters(dev,params);
            return;
        }
        throw btle::exceptions::not_supported("plugin does not support connection paramter update");
    }
    throw btle::exceptions::device_not_connected("device not connected device is in state: " + dev.state_string());
}

/**
 * @brief collector::write_file, write an single file over btle lib service
 * @param dev
 * @param stream file or message etc..
 * @param id unique id of the file to be written, default 0, but it's a good practise to have some unique id, it's for stream cancelation etc...
 */
void collector::write_file(btle::device& dev, std::ostream& stream, int identifier)
{
    verify(plugin_)
    if( dev.state() == btle::DEVICE_CONNECTED )
    {
        if( dev.db().fetch_service(uuid(BTLE_SERVICE)) )
        {
            if( dev.db().fetch_characteristic(uuid(BTLE_MTU)) )
            {
                btlelibservice* gattservice = (btlelibservice*)dev.gatt_service(uuid(BTLE_SERVICE));
                gattservice->write_service_value(uuid(BTLE_MTU), identifier, "TODO", &dev, NULL);
                return;
            }
            throw btle::exceptions::attribute_not_found("device does not contain btle characteristic mtu");
        }
        throw btle::exceptions::attribute_not_found("device does not contain btle service");
    }
    throw btle::exceptions::device_not_connected("device not connected device is in state: " + dev.state_string());
}

void collector::device_state_changed(btle::device& dev)
{
    device_state_changed_cb(dev);
    if( dev.state() == DEVICE_DISCONNECTED )
    {
        dev.clear_device_data();
    }
}

void collector::aquire_start_scan()
{
    verify(plugin_)
    flags_ |= INTERNAL_SCAN;
    if( state_ == STATE_POWERED_ON ) plugin_->start_scan();
    else _log_error("BT NOT POWERED!");
}

void collector::aquire_stop_scan()
{
    verify(plugin_)
    flags_ &= ~INTERNAL_SCAN;
    if( state_ == STATE_POWERED_ON ) plugin_->stop_scan();
    else _log_error("BT NOT POWERED!");
}

void collector::aquire_connect_device(btle::device& dev)
{
    verify(plugin_)
    if( state_ ==STATE_POWERED_ON ) plugin_->connect_device(dev);
    else _log_error("BT NOT POWERED");
}

void collector::aquire_disconnect_device(btle::device& dev)
{
    verify(plugin_)
    if( state_ ==STATE_POWERED_ON ) plugin_->disconnect_device(dev);
    else _log_error("BT NOT POWERED");
}

void collector::aquire_cancel_pending_connection(btle::device& dev)
{
    verify(plugin_)
    if( state_ ==STATE_POWERED_ON ) plugin_->cancel_pending_connection(dev);
    else _log_error("BT NOT POWERED");
}


void collector::plugin_state_changed(plugin_state state)
{
    state_ = state;
    switch (state) {
        case STATE_POWERED_ON:
        {
            if( flags_ & CLIENT_SCAN ||
                flags_ & INTERNAL_SCAN )
            {
                plugin_->start_scan();
            }
            break;
        }
        case STATE_POWERED_RESETTING:
        {
            // TODO invalidate all devices, propagate disconnected callbacks
            break;
        }
        default:
            break;
    }
}

void collector::new_device_discovered(device& dev,adv_fields& fields,int rssi)
{
    func_log

    process_device_discovered(dev,fields,rssi,true);
}

void collector::device_discovered(device& dev, adv_fields& fields, int rssi)
{
    process_device_discovered(dev,fields,rssi);
}

void collector::process_device_discovered(device& dev, adv_fields& fields, int rssi, bool new_device)
{
    dev.advertisement_fields() << fields;
    dev.rssi_filter() << rssi;
    // inform connectionhandler
    connectionhandler_.advertisement_head_received(dev);
    if( flags_ & CLIENT_SCAN )
    {
        if( filters_.size() )
        {
            for( std::vector<scanfilterbase*>::iterator it = filters_.begin(); it != filters_.end(); ++it )
            {
                if( (*it)->process(dev,fields,rssi) )
                {
                    // propagate callback
                    if(!new_device) device_discovered_cb(dev);
                    else new_device_discovered_cb(dev);
                    return;
                }
            }
        }
        else
        {
            if(!new_device) device_discovered_cb(dev);
            else new_device_discovered_cb(dev);
        }
    }
}

void collector::device_connected(device& dev)
{
    connectionhandler_.device_connected(dev);
    plugin_->discover_services(dev);
}

void collector::device_disconnected(device& dev)
{
    connectionhandler_.device_disconnected(dev);
}

void collector::device_services_discovered(device& dev, service_list& services, const error& err)
{
    if( err.code() == 0 )
    {
        dev.db() << services;
        for( service_iterator_const it = services.begin(); it != services.end(); ++it )
        {
            if( gattservicebase* gatt_srv = dev.gatt_service(it->uuid()) )
            {
                // TODO give heads up to client, the device has e.g. Heart Rate Service etc...
                // something like device_features_updated
                device_gatt_service_discovered_cb(dev,gatt_srv);
            }
            plugin_->discover_characteristics(dev,(*it));
        }
    }
    else device_service_discovery_failed_cb(dev,services,err);
}

void collector::device_characteristics_discovered(device& dev, service& srv, chr_list& chrs, const error& err)
{
    if( err.code() == 0 )
    {
        // first check auto read
        for( chr_iterator_const it = chrs.begin(); it != chrs.end(); ++it )
        {
            for( uuid_iterator_const it_uuid = read_uuids_.begin(); it_uuid != read_uuids_.end(); ++it_uuid )
            {
                if( (*it) == (*it_uuid) &&
                    it->properties() & GATT_READ )
                {
                    plugin_->read_characteristic_value(dev,srv,(*it));
                }
            }
        }
        // second check notify list
        for( chr_iterator_const it = chrs.begin(); it != chrs.end(); ++it )
        {
            for( uuid_iterator_const it_uuid = notify_uuids_.begin(); it_uuid != notify_uuids_.end(); ++it_uuid )
            {
                if( it->uuid() == (*it_uuid) &&
                    ( it->properties() & GATT_INDICATE ||
                      it->properties() & GATT_NOTIFY ))
                {
                    plugin_->set_characteristic_notify(dev,srv,(*it),true);
                }
            }
        }
        // run gatt service config
        /*if( gattservicebase* gatt_srv = dev.gatt_service(it->uuid()) )
        {

        }*/
    }
    else device_characteristic_discovery_failed_cb(dev,srv,chrs,err);
}

void collector::device_characteristic_read(
    device& dev,
    service& srv,
    characteristic& chr,
    std::string& data,
    const error& err)
{
    if( gattservicebase* gatt_service = dev.gatt_service(srv.uuid()) )
    {
        if( gatt_service->contains_characteristic_uuid(chr.uuid()) )
        {
            gatt_service->process_service_value_read(chr.uuid(),(const uint8_t*)data.c_str(),data.size(), err);
            device_service_value_updated_cb(dev,gatt_service);
            return;
        }
    }
    device_characteristic_read_cb(dev,srv,chr,data,err);
}

void collector::device_characteristic_written(device& dev, service& srv, characteristic& chr, const error& err)
{
    // TODO
}

void collector::device_characteristic_nofication_state_changed(device& dev, service& srv, characteristic& chr, bool notify, const error& err)
{
    if( gattservicebase* gatt_service = dev.gatt_service(srv.uuid()) )
    {
        gatt_service->set_active(chr.uuid(),err.code() == 0 ? notify : false);
    }
}

void collector::device_descriptor_written(device& dev, service& srv, characteristic& chr, descriptor& desc, const error& err)
{
    // TODO, NOTE device_characteristic_nofication_state_changed will be removed
    if( gattservicebase* gatt_service = dev.gatt_service(srv.uuid()) )
    {
        gatt_service->set_active(chr.uuid(),desc.is_notifying());
    }
}

void collector::device_characteristic_notify_data_updated(device& dev, service& srv, characteristic& chr, std::string& data)
{
    // notification or indication has been received
    if( gattservicebase* gatt_service = dev.gatt_service(srv.uuid()) )
    {
        if( gatt_service->contains_characteristic_uuid(chr.uuid()) )
        {
            gatt_service->process_service_notify_data(chr.uuid(),(const uint8_t*)data.c_str(),data.size());
            device_service_value_updated_cb(dev,gatt_service);
        }
    }
}

void collector::device_rssi_read(device& dev, int rssi)
{
    dev.rssi_filter() << rssi;
}

void collector::device_services_invalidated(device& dev)
{
    // TODO
}

void collector::device_connection_parameters_updated(device& dev)
{
    _log("device connection parameters updated: %s",dev.parameters().description().c_str());
}

void collector::new_device_discovered_cb(device& dev)
{
    _log("info: %s",dev.description().c_str());
}

void collector::device_discovered_cb(device& dev)
{
    _log("info: %s",dev.description().c_str());
}

/**
 * @brief collector::device_gatt_service_discovered_cb, this callbacks idea is
 * to be used in the next way:
 * if( srv )
 * {
 *      switch( srv->service_uuid().uuid16bit() )
 *      {
 *          case HEART_RATE_SERVICE:
 *          {
 *              // enable Heart rate specific ui etc...
 *              break;
 *          }
 *          etc...
 *      }
 * }
 * @param dev
 * @param srv
 */
void collector::device_gatt_service_discovered_cb(device& dev, const gattservicebase* srv)
{
    _log_warning("Device contains some included service or 3rd added service device: %s gatt uuid: %s",
                 dev.description().c_str(),
                 srv->service_uuid().description().c_str());
}

void collector::device_service_discovery_failed_cb(device& dev, const service_list& services, const error& err)
{
    _log_warning("device service discovery failed, override this callback for further logic device desc: %s error: %s",
                 dev.description().c_str(),
                 err.description().c_str());
}

void collector::device_characteristic_discovery_failed_cb(device& dev, const service& srv, const chr_list& chrs, const error& err)
{
    _log_warning("device service discovery failed, override this callback for further logic");
}

void collector::plugin_state_changed_cb(plugin_state state)
{
    _log_warning("plugin state: %i",state);
}

/**
 * @brief collector::device_btle_ftp_in_progress
 * @param dev remote device
 * @param progress in procentage 0-100%
 * @param id unique file identifier 0 = not specified
 */
void collector::device_btle_ftp_in_progress(device& dev, double progress, const std::string& data, int identifier)
{
    _log("File progress procentage: %d file identifier: %i",progress,identifier);
}

/**
 * @brief collector::device_btle_ftp_out_progress
 * @param dev remote device
 * @param progress in procentage 0-100%
 * @param id unique file identifier 0 = not specified
 */
void collector::device_btle_ftp_out_progress(device& dev, double progress, int identifier)
{
    _log("File progress procentage: %d file identifier: %i",progress,identifier);
}

btle::device* collector::fetch_device(const bda& addr)
{
    verify(plugin_)
    for( device_iterator it = plugin_->devices().begin(); it != plugin_->devices().end(); ++it )
    {
        if( (*it)->addr() == addr )
        {
            return (*it);
        }
    }
    return NULL;
}


