
#include "btle/central/apple/corebluetoothcentralplugin.h"
#include "btle/central/centralpluginregisterer.h"
#include "btle/service.h"
#include "btle/log.h"

using namespace btle::central::apple;
using namespace btle::central;
using namespace btle;

namespace {
    centralpluginregisterer<corebluetoothcentralplugin> registration;
    
    #define UUID_2_STRING(a) std::string([[[a UUID] UUIDString] UTF8String])
}


@implementation corebluetoothcentralpluginprivate

-(id) init:(corebluetoothcentralplugin*) plugin
{
    self = [super init];
    if(self)
    {
        parent_ = plugin;
        manager_ = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
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
    if( dev == NULL )
    {
        dev = new corebluetoothperipheraldevice(std::string([[[aPeripheral identifier] UUIDString] UTF8String]));
        dev->peripheral_ = aPeripheral;
        [dev->peripheral_ setDelegate:self];
        parent_->devices().push_back(dev);
    }
    // TODO
    parent_->observer().device_discovered(*dev);
}

- (void)centralManager:(CBCentralManager *)central didRetrievePeripherals:(NSArray *)peripherals
{
    // TODO
}

- (void)centralManager:(CBCentralManager *)central didRetrieveConnectedPeripherals:(NSArray *)peripherals
{
    // TODO
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
    btle::service_list list;
    btle::error err(0);
    if( error == nil ){
        btle::service* srv = NULL;
        btle::characteristic* chr = NULL;
        dev->fetch_service_and_characteristic(characteristic, srv, chr);
        std::string data((const char*)[[characteristic value] bytes],[[characteristic value] length]);
        assert( srv && chr );
        parent_->observer().device_characteristic_notify_data_updated(*dev, *srv, *chr, data);
    }
    else{
        err = btle::error((int)[error code],"Unknown CoreBluetooth error");
    }
}

- (void)peripheralDidUpdateRSSI:(CBPeripheral *)aPeripheral error:(NSError *)error
{
}

- (void)peripheral:(CBPeripheral *)aPeripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
}

- (void)peripheral:(CBPeripheral *)aPeripheral didWriteValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
}

-(void)peripheral:(CBPeripheral *)peripheral didDiscoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForDescriptor:(CBDescriptor *)descriptor error:(NSError *)error
{
}

- (void)peripheral:(CBPeripheral *)peripheral didWriteValueForDescriptor:(CBDescriptor *)descriptor error:(NSError *)error
{
}

@end

corebluetoothcentralplugin::corebluetoothcentralplugin(centralpluginobserver &observer)
: centralplugininterface(observer),
  base("","CoreBluetooth central plugin implementation")
{
}

const std::string& corebluetoothcentralplugin::name()
{
    static std::string name = "APPLE";
    return name;
}

std::vector<device*>& corebluetoothcentralplugin::devices()
{
    return devices_;
}

device* corebluetoothcentralplugin::allocate_new_device(const bda& addr)
{
    return NULL;
}

int corebluetoothcentralplugin::start()
{
    privateimpl_ = [[corebluetoothcentralpluginprivate alloc] init: this];
    return 0;
}

void corebluetoothcentralplugin::stop()
{
    privateimpl_= nil;
}

void corebluetoothcentralplugin::start_scan( const uuid_list* services )
{
    NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber  numberWithBool:YES], CBCentralManagerScanOptionAllowDuplicatesKey, nil];

    [privateimpl_->manager_ scanForPeripheralsWithServices:nil options:options];
}

void corebluetoothcentralplugin::stop_scan()
{
    [privateimpl_->manager_ stopScan];
}

void corebluetoothcentralplugin::connect_device(device& dev)
{
    [privateimpl_->manager_ connectPeripheral:((corebluetoothperipheraldevice&)dev).peripheral_ options:nil];
}

void corebluetoothcentralplugin::disconnect_device(device& dev)
{
    [privateimpl_->manager_ cancelPeripheralConnection:((corebluetoothperipheraldevice&)dev).peripheral_];
}

void corebluetoothcentralplugin::cancel_pending_connection(device& dev)
{
    [privateimpl_->manager_ cancelPeripheralConnection:((corebluetoothperipheraldevice&)dev).peripheral_];
}

void corebluetoothcentralplugin::discover_services(device& dev)
{
    [((corebluetoothperipheraldevice&)dev).peripheral_ discoverServices:nil];
}

void corebluetoothcentralplugin::discover_characteristics(device& dev, const service& srv)
{
    corebluetoothperipheraldevice& core_dev = ((corebluetoothperipheraldevice&)dev);
    CBService* service = core_dev.fetch_service(srv);
    assert(service);
    [core_dev.peripheral_ discoverCharacteristics:nil forService:service];
}

void corebluetoothcentralplugin::read_characteristic_value(device& dev,const service& srv, const characteristic& chr)
{

}

void corebluetoothcentralplugin::write_characteristic_value(device& dev,const service& srv, const characteristic& chr, const std::string& data, characteristic_properties type)
{

}

void corebluetoothcentralplugin::set_characteristic_notify(device& dev,const service& srv, const characteristic& chr, bool notify)
{
    corebluetoothperipheraldevice& core_dev = ((corebluetoothperipheraldevice&)dev);
    CBCharacteristic* aChr = core_dev.fetch_characteristic(chr);
    assert(aChr);
    [core_dev.peripheral_ setNotifyValue:notify forCharacteristic:aChr];
}

void corebluetoothcentralplugin::write_descriptor(device& dev, const service& srv, const characteristic& chr, descriptor& desc, bool notify)
{

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

