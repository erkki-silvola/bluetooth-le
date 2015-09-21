
#include "btle/central/apple/corebluetoothcentralplugin.h"
#include "btle/central/centralpluginregisterer.h"
#include "btle/service.h"
#include "btle/utility.h"
#include "btle/gatt_services/btlelibservice.h"
#include "btle/log.h"

using namespace btle::central::apple;
using namespace btle::central;
using namespace btle;

namespace {
    centralpluginregisterer<corebluetoothcentralplugin> registration;
    
    #define UUID_2_STRING(a) std::string([[[a UUID] UUIDString] UTF8String])
    
    inline void process_advertisement_fields(NSDictionary* dic, adv_fields& fields)
    {
        if( NSString* name = [dic objectForKey:CBAdvertisementDataLocalNameKey] )
        {
            fields[btle::GAP_ADTYPE_LOCAL_NAME_COMPLETE] = advertisementdata([name UTF8String]);
        }
        if( NSNumber* power = [dic objectForKey:CBAdvertisementDataTxPowerLevelKey] )
        {
            fields[btle::GAP_ADTYPE_POWER_LEVEL] = advertisementdata([[power stringValue] UTF8String]);
        }
        if( NSData* service_data = [dic objectForKey:CBAdvertisementDataServiceDataKey] )
        {
            fields[GAP_ADTYPE_SERVICE_DATA] = advertisementdata(std::string((const char*)[service_data bytes],[service_data length]));
        }
        if( NSData* man_data = [dic objectForKey:CBAdvertisementDataManufacturerDataKey] )
        {
            fields[GAP_ADTYPE_MANUFACTURER_SPECIFIC] = advertisementdata(std::string((const char*)[man_data bytes],[man_data length]));
        }
        if( NSArray* uuids = [dic objectForKey:CBAdvertisementDataServiceUUIDsKey] )
        {
            std::string uuids_bin;
            int len=2;
            for( CBUUID* uid : uuids )
            {
                uuids_bin.append((const char*)[[uid data] bytes],[[uid data] length]);
                len = (int)[[uid data] length];
            }
            if( [uuids count] == 1 )
            {
                if(len == 2) fields[GAP_ADTYPE_16BIT_COMPLETE] = advertisementdata(uuids_bin);
                else fields[GAP_ADTYPE_128BIT_COMPLETE] = advertisementdata(uuids_bin);
            }
            else if( [uuids count] > 1 )
            {
                if(len == 2) fields[GAP_ADTYPE_16BIT_MORE] = advertisementdata(uuids_bin);
                else fields[GAP_ADTYPE_128BIT_MORE] = advertisementdata(uuids_bin);
            }
        }
    }
}


@implementation corebluetoothcentralpluginprivate

-(id) init:(corebluetoothcentralplugin*) plugin
{
    self = [super init];
    if(self)
    {
        parent_ = plugin;
        manager_ = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
#ifdef DESKTOP_BUILD
        [manager_ retain];
#endif
    }
    return self;
}

-(void) dealloc
{
}

- (void) centralManagerDidUpdateState:(CBCentralManager *)central
{
    // TODO
    switch ([central state]) {
        case CBCentralManagerStatePoweredOn:
        {
            parent_->observer().plugin_state_changed(STATE_POWERED_ON);
            break;
        }
        case CBCentralManagerStatePoweredOff:
        {
            parent_->observer().plugin_state_changed(STATE_POWERED_OFF);
            break;
        }
        case CBCentralManagerStateResetting:
        {
            parent_->observer().plugin_state_changed(STATE_POWERED_RESETTING);
            break;
        }
        case CBCentralManagerStateUnauthorized:
        case CBCentralManagerStateUnknown:
        {
            parent_->observer().plugin_state_changed(STATE_POWERED_UNKNOWN);
            break;
        }
        case CBCentralManagerStateUnsupported:
        {
            parent_->observer().plugin_state_changed(STATE_POWERED_NON_SUPPORTED);
            break;
        }
        default:
            break;
    }
}

- (void) centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)aPeripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
    corebluetoothperipheraldevice* dev = parent_->find_device(aPeripheral);
    adv_fields fields;
    process_advertisement_fields(advertisementData, fields);
    if( dev == NULL )
    {
        dev = new corebluetoothperipheraldevice(std::string([[[aPeripheral identifier] UUIDString] UTF8String]));
#ifdef DESKTOP_BUILD
        [aPeripheral retain];
#endif
        dev->peripheral_ = aPeripheral;
        [dev->peripheral_ setDelegate:self];
        parent_->devices().push_back(dev);
        parent_->observer().new_device_discovered(*dev,fields,[RSSI intValue]);
    }
    else
    {
#ifdef DESKTOP_BUILD
        [aPeripheral retain];
#endif
        dev->peripheral_ = aPeripheral;
        [dev->peripheral_ setDelegate:self];
        parent_->observer().device_discovered(*dev,fields,[RSSI intValue]);
    }
}

- (void)centralManager:(CBCentralManager *)central didRetrievePeripherals:(NSArray *)peripherals
{
    // TODO
}

- (void)centralManager:(CBCentralManager *)central didRetrieveConnectedPeripherals:(NSArray *)peripherals
{
    // TODO, allways when scanning starts fetch connected peripherals
}

- (void) centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)aPeripheral
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(aPeripheral));
    assert(dev);
    parent_->observer().device_connected(*dev);
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)aPeripheral error:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(aPeripheral));
    assert(dev);
    parent_->observer().device_disconnected(*dev);
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)aPeripheral error:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(aPeripheral));
    assert(dev);
    parent_->observer().device_disconnected(*dev);
}

//
- (void) peripheral:(CBPeripheral *)aPeripheral didDiscoverServices:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(aPeripheral));
    assert(dev);
    btle::service_list list;
    btle::error err(0);
    if( error == nil ){
        dev->process_services_discovered(aPeripheral, list);
    }
    else{
        err = btle::error((int)[error code],"Unknown CoreBluetooth error");
    }
    parent_->observer().device_services_discovered(*dev, list, err);
}

- (void) peripheral:(CBPeripheral *)aPeripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(aPeripheral));
    assert(dev);
    btle::service_list list;
    btle::error err(0);
    if( error == nil ){
        btle::service* srv( dev->process_characteristics_discovered(service) );
        assert( srv );
        parent_->observer().device_characteristics_discovered(*dev, *srv, srv->characteristics(), err);
    }
    else{
        err = btle::error((int)[error code],"Unknown CoreBluetooth error");
        btle::service srv( (uuid(UUID_2_STRING(service))) );
        parent_->observer().device_characteristics_discovered(*dev, srv, srv.characteristics(), err);
    }
}

- (void) peripheral:(CBPeripheral *)aPeripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(aPeripheral));
    assert(dev);
    btle::error err(0);
    btle::service* srv = NULL;
    btle::characteristic* chr = NULL;
    dev->fetch_service_and_characteristic(characteristic, srv, chr);
    assert( srv && chr );
    if( [characteristic isNotifying] ){
        if( error == nil ){
            std::string data((const char*)[[characteristic value] bytes],[[characteristic value] length]);
//          NSLog(@"data hex: %s", utility::to_hex_string(data).c_str());
            parent_->observer().device_characteristic_notify_data_updated(*dev, *srv, *chr, data);
        }
        else{
            _log_error("Unknown CoreBluetooth error");
        }
    }
    else{
        std::string data="";
        if( error == nil ){
            data = std::string((const char*)[[characteristic value] bytes],[[characteristic value] length]);
        }
        else{
            err = btle::error((int)[error code],"Unknown CoreBluetooth error");
        }
        parent_->observer().device_characteristic_read(*dev, *srv, *chr, data, err);
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didReadRSSI:(NSNumber *)RSSI error:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(peripheral));
    assert(dev);
    if( error == nil ){
        parent_->observer().device_rssi_read(*dev, (int)[RSSI integerValue]);
    }
    else{
        _log_error("Rssi read failed code : %i",[error code]);
    }
}

- (void)peripheral:(CBPeripheral *)aPeripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(aPeripheral));
    assert(dev);
    btle::error err(0);
    btle::service* srv = NULL;
    btle::characteristic* chr = NULL;
    dev->fetch_service_and_characteristic(characteristic, srv, chr);
    assert( srv && chr );
    assert( chr->contains_descriptor_type(CLIENT_CHARACTERISTIC_CONFIGURATION) );
    descriptor* desc = chr->descriptor_by_type(CLIENT_CHARACTERISTIC_CONFIGURATION);
    if( error == nil ){
        desc->set_notifying([characteristic isNotifying]);
    }
    else{
        desc->set_notifying(false); // ?
        err = btle::error((int)[error code],"Unknown CoreBluetooth error");
    }
    parent_->observer().device_descriptor_written(*dev,*srv,*chr,*desc,err);
}

- (void)peripheral:(CBPeripheral *)aPeripheral didWriteValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(aPeripheral));
    assert(dev);
    btle::error err(0);
    btle::service* srv = NULL;
    btle::characteristic* chr = NULL;
    dev->fetch_service_and_characteristic(characteristic, srv, chr);
    assert( srv && chr );
    if( error ){
        err = btle::error((int)[error code],"Unknown CoreBluetooth error");
    }
    parent_->observer().device_characteristic_written(*dev,*srv,*chr,err);
}

-(void)peripheral:(CBPeripheral *)peripheral didDiscoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
    // this extra, e.g. for Server Characteristic Conf...
    
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForDescriptor:(CBDescriptor *)descriptor error:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(peripheral));
    assert(dev);
    btle::error err(0);
    btle::service* srv = NULL;
    btle::characteristic* chr = NULL;
    dev->fetch_service_and_characteristic(descriptor, srv, chr);
    assert( srv && chr );
    // TODO
}

- (void)peripheral:(CBPeripheral *)peripheral didWriteValueForDescriptor:(CBDescriptor *)descriptor error:(NSError *)error
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(peripheral));
    assert(dev);
    btle::error err(0);
    btle::service* srv = NULL;
    btle::characteristic* chr = NULL;
    dev->fetch_service_and_characteristic(descriptor, srv, chr);
    assert( srv && chr );
    // TODO
}

- (void)peripheral:(CBPeripheral *)peripheral didModifyServices:(NSArray *)invalidatedServices
{
    func_log
    
    corebluetoothperipheraldevice* dev(parent_->find_device(peripheral));
    assert(dev);
    parent_->observer().device_services_invalidated(*dev);
}

@end

corebluetoothcentralplugin::corebluetoothcentralplugin(centralpluginobserver &observer)
: centralplugininterface(observer),
  base("","CoreBluetooth central plugin implementation")
{
}

std::string corebluetoothcentralplugin::name()
{
    func_log

    return "APPLE";
}

std::vector<device*>& corebluetoothcentralplugin::devices()
{
    func_log

    return devices_;
}

device* corebluetoothcentralplugin::allocate_new_device(const bda& addr)
{
    func_log

    return new corebluetoothperipheraldevice(addr);
}

int corebluetoothcentralplugin::start()
{
    func_log

    privateimpl_ = [[corebluetoothcentralpluginprivate alloc] init: this];
#ifdef DESKTOP_BUILD
    [privateimpl_ retain];
#endif
    return 0;
}

void corebluetoothcentralplugin::stop()
{
    func_log

#ifdef DESKTOP_BUILD
    [privateimpl_ release];
#endif
    privateimpl_= nil;
}

void corebluetoothcentralplugin::start_scan( central_scan_parameters param, const uuid_list* services )
{
    func_log

    NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber  numberWithBool:YES], CBCentralManagerScanOptionAllowDuplicatesKey, nil];
    [privateimpl_->manager_ scanForPeripheralsWithServices:nil options:options];
}

void corebluetoothcentralplugin::stop_scan()
{
    func_log

    [privateimpl_->manager_ stopScan];
}

void corebluetoothcentralplugin::connect_device(device& dev)
{
    func_log

    [privateimpl_->manager_ connectPeripheral:((corebluetoothperipheraldevice&)dev).peripheral_ options:nil];
}

void corebluetoothcentralplugin::disconnect_device(device& dev)
{
    func_log

    [privateimpl_->manager_ cancelPeripheralConnection:((corebluetoothperipheraldevice&)dev).peripheral_];
}

void corebluetoothcentralplugin::cancel_pending_connection(device& dev)
{
    func_log

    [privateimpl_->manager_ cancelPeripheralConnection:((corebluetoothperipheraldevice&)dev).peripheral_];
}

void corebluetoothcentralplugin::discover_services(device& dev)
{
    func_log

    [((corebluetoothperipheraldevice&)dev).peripheral_ discoverServices:nil];
}

void corebluetoothcentralplugin::discover_characteristics(device& dev, const service& srv)
{
    func_log

    corebluetoothperipheraldevice& core_dev = ((corebluetoothperipheraldevice&)dev);
    CBService* service = core_dev.fetch_service(srv);
    assert(service);
    [core_dev.peripheral_ discoverCharacteristics:nil forService:service];
}

void corebluetoothcentralplugin::read_characteristic_value(device& dev,const service& srv, const characteristic& chr)
{
    func_log

    corebluetoothperipheraldevice& core_dev = ((corebluetoothperipheraldevice&)dev);
    CBService* service = core_dev.fetch_service(srv);
    CBCharacteristic* aChr = core_dev.fetch_characteristic(chr);
    assert(service && aChr);
    [core_dev.peripheral_ readValueForCharacteristic:aChr];
}

void corebluetoothcentralplugin::write_characteristic_value(device& dev,const service& srv, const characteristic& chr, const std::string& data, characteristic_properties type)
{
    func_log

    corebluetoothperipheraldevice& core_dev = ((corebluetoothperipheraldevice&)dev);
    CBService* service = core_dev.fetch_service(srv);
    CBCharacteristic* aChr = core_dev.fetch_characteristic(chr);
    assert(service && aChr);
    [core_dev.peripheral_ writeValue:[NSData dataWithBytes:data.c_str() length:data.size()] forCharacteristic:aChr type:type == GATT_WRITE ? CBCharacteristicWriteWithResponse : CBCharacteristicWriteWithoutResponse];
}

void corebluetoothcentralplugin::set_characteristic_notify(device& dev,const service& srv, const characteristic& chr, bool notify)
{
    func_log

    // will be deprecated
    corebluetoothperipheraldevice& core_dev = ((corebluetoothperipheraldevice&)dev);
    CBCharacteristic* aChr = core_dev.fetch_characteristic(chr);
    assert(aChr);
    [core_dev.peripheral_ setNotifyValue:notify forCharacteristic:aChr];
}

void corebluetoothcentralplugin::write_descriptor(device& dev, const service& srv, const characteristic& chr, descriptor& desc, bool notify)
{
    func_log

    corebluetoothperipheraldevice& core_dev = ((corebluetoothperipheraldevice&)dev);
    CBCharacteristic* aChr = core_dev.fetch_characteristic(chr);
    assert(aChr);
    switch (desc.type()) {
        case CLIENT_CHARACTERISTIC_CONFIGURATION:
        {
            [core_dev.peripheral_ setNotifyValue:notify forCharacteristic:aChr];
            break;
        }
        default:
            break;
    }
}

void corebluetoothcentralplugin::write_btle_ftp(device& dev, const std::string& buffer)
{
    func_log

    dispatch_sync(dispatch_get_main_queue(), ^{
        corebluetoothperipheraldevice& core_dev = ((corebluetoothperipheraldevice&)dev);
        CBCharacteristic* aChr = core_dev.fetch_characteristic(uuid(BTLE_MTU));
        assert(aChr);
        [core_dev.peripheral_ writeValue:[NSData dataWithBytes:buffer.c_str() length:buffer.size()]
                  forCharacteristic:aChr type:CBCharacteristicWriteWithoutResponse];
    });
}

int corebluetoothcentralplugin::read_btle_ftp(device& dev, std::string& buffer, bool ack)
{
    return 0;
}

centralpluginobserver& corebluetoothcentralplugin::observer()
{
    return observer_;
}

corebluetoothperipheraldevice* corebluetoothcentralplugin::find_device(CBPeripheral* peripheral)
{
    std::string identifier_str( [[[peripheral identifier] UUIDString] UTF8String] );
    for( std::vector<device*>::iterator it = devices_.begin(); it != devices_.end(); ++it )
    {
        if( (*it)->addr() == identifier_str )
        {
            return (corebluetoothperipheraldevice*)(*it);
        }
    }
    return NULL;
}

