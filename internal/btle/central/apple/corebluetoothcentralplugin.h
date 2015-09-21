#ifndef COREBLUETOOTHCENTRALPLUGIN_H
#define COREBLUETOOTHCENTRALPLUGIN_H

#include "btle/central/centralplugininterface.h"
#include "btle/central/apple/corebluetoothperipheraldevice.h"

#ifdef DESKTOP_BUILD
#import <IOBluetooth/IOBluetooth.h>
#else
#import <CoreBluetooth/CoreBluetooth.h>
#endif

namespace btle {
    namespace central {
        namespace apple {
            class corebluetoothcentralplugin;
        }
    }
}


@interface corebluetoothcentralpluginprivate : NSObject<CBCentralManagerDelegate,CBPeripheralDelegate>
{
@public
    btle::central::apple::corebluetoothcentralplugin* parent_;
    CBCentralManager* manager_;
}

@end

namespace btle {
    namespace central {
        namespace apple {
            class corebluetoothcentralplugin: public centralplugininterface, public base{
            public:
                corebluetoothcentralplugin(centralpluginobserver& observer);

            public: // from base

                std::string name();
                std::vector<device*>& devices();
                device* allocate_new_device(const bda& addr);
                int start();
                void stop();
                void start_scan(central_scan_parameters param = SCAN_HIGH_DUTY, const uuid_list* services = NULL );
                void stop_scan();
                void connect_device(device& dev);
                void disconnect_device(device& dev);
                void cancel_pending_connection(device& dev);
                void discover_services(device& dev);
                void discover_characteristics(device& dev, const service& srv);
                void read_characteristic_value(device& dev,const service& srv, const characteristic& chr);
                void write_characteristic_value(device& dev,const service& srv, const characteristic& chr, const std::string& data, characteristic_properties type);
                void set_characteristic_notify(device& dev,const service& srv, const characteristic& chr, bool notify);
                void write_descriptor(device& dev, const service& srv, const characteristic& chr, descriptor& desc, bool notify);
                void write_btle_ftp(device& dev, const std::string& buffer);
                int read_btle_ftp(device& dev, std::string& buffer, bool ack);

                centralpluginobserver& observer();
                corebluetoothperipheraldevice* find_device(CBPeripheral* peripheral);
                
            private:
                
                corebluetoothcentralpluginprivate* privateimpl_;

            };
        }
    }
}


#endif // COREBLUETOOTHCENTRALPLUGIN_H

