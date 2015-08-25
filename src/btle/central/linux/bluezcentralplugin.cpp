

#include "btle/central/linux/bluezcentralplugin.h"
#include "btle/central/centralpluginregisterer.h"
#include "btle/central/linux/hciconnectdevicemessage.h"
#include "btle/central/linux/hciattreadbygroupmessage.h"
#include "btle/log.h"
#include "btle_global.h"
#include "btle/utility.h"

#include <assert.h>
#include <sys/ioctl.h>
#include <stdio.h>

#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <signal.h>

#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

using namespace btle::central::linux_platform;
using namespace btle::central;
using namespace btle;

namespace {
    centralpluginregisterer<bluezcentralplugin> registration;

    void routine(void* context)
    {
        bluezcentralplugin* parent(reinterpret_cast<bluezcentralplugin*>(context));
        parent->scan_routine();
    }

    void process_adv_data(adv_fields& fields, const unsigned char* data, size_t length)
    {
        int offset(0);
        while(offset < length)
        {
            btle::advertisement_data_type type = (advertisement_data_type)*(data+offset+1);
            fields[type] = btle::advertisementdata(std::string((const char*)(data+offset+2),*(data) - 1));
            offset += *(data+offset) + 1;
        }
    }
}

bluezcentralplugin::bluezcentralplugin(centralpluginobserver &observer)
: centralplugininterface(observer),
  base("","LINUX central plugin implementation"),
  id_(0),
  handle_(0),
  main_(),
  mutex_(),
  started_()
{

}

#include <QDebug>
#include <QThread>

void bluezcentralplugin::scan_routine()
{
    _log("main routine started");
    mutex_.lock();
    started_.notify_all();
    mutex_.unlock();

    struct hci_filter nf, of;
    unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
    //struct sigaction sa;
    socklen_t olen;
    int len;
    olen = sizeof(of);

    if (getsockopt(handle_, SOL_HCI, HCI_FILTER, &of, &olen) == 0)
    {
        hci_filter_clear(&nf);
        hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
        hci_filter_set_event(EVT_LE_META_EVENT, &nf);

        if (setsockopt(handle_, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) == 0)
        {
            while (1) {
                evt_le_meta_event *meta;
                le_advertising_info *info;
                char addr[18];
                while ((len = read(handle_, buf, sizeof(buf))) < 0) {
                    if (errno == EINTR)
                    {
                        len = 0;
                        return;
                    }
                    if (errno == EAGAIN || errno == EINTR)
                        continue;
                    return;
                }
                ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
                len -= (1 + HCI_EVENT_HDR_SIZE);
                meta = (evt_le_meta_event *) ptr;
                switch(meta->subevent)
                {
                    case 0x02:
                    {
                        info = (le_advertising_info *) (meta->data + 1);
                        //qDebug() << QString::fromStdString(utility::to_hex_string(info->data,info->length));
                        //qDebug() << "thread id: " << QThread::currentThreadId();

                        adv_fields fields;
                        process_adv_data(fields,info->data,info->length);
                        bluezperipheraldevice* dev(find_device(btle::bda((const char*)info->bdaddr.b,(address_type)info->bdaddr_type)));

                        if(dev == NULL)
                        {
                            dev = new bluezperipheraldevice(btle::bda((const char*)info->bdaddr.b,(address_type)info->bdaddr_type));
                            devices_.push_back(dev);
                            observer_.new_device_discovered(*dev,fields,-100);
                        }
                        else
                        {
                            observer_.device_discovered(*dev,fields,-100);
                        }

                        char name[30];
                        memset(name, 0, sizeof(name));
                        ba2str(&info->bdaddr, addr);
                        printf("%s %s\n", addr, name);
                        break;
                    }
                    case 0x03: // connection update comlete
                    {

                    }
                    case 0x04: // LE Read Remote Used Features Complete event
                    {

                    }
                    case 0x05: // LE long term key request event
                    {

                    }
                    default:
                        return;
                }
            }
        }
        else
        {
            printf("Could not set socket options\n");
            return;
        }
    }
    else
    {
        printf("Could not get socket options\n");
        return;
    }
}

const std::string& bluezcentralplugin::name()
{
    return "LINUX_CENTRAL";
}

std::vector<device*>& bluezcentralplugin::devices()
{
    return devices_;
}

device* bluezcentralplugin::allocate_new_device(const bda& addr)
{
    return NULL;
}

int bluezcentralplugin::start()
{
    id_ = hci_get_route(NULL);
    int on = 1;
    if((handle_ = hci_open_dev(id_)) >= 0)
    {
        //if( ioctl(handle_, FIONBIO, (char *)&on) == 0)
        {
            main_ = std::thread(::routine,this);
            std::unique_lock<std::mutex> lock(mutex_);
            started_.wait(lock);
            observer_.plugin_state_changed(btle::STATE_POWERED_ON);
            return 0;
        }
        //else return -1;
    }
    return handle_;
}

void bluezcentralplugin::stop()
{
    hci_close_dev(handle_);
}

void bluezcentralplugin::start_scan(central_scan_parameters param, const uuid_list* services )
{
    // TODO change from full window to approx. 80%
    int err(0);
    if( (err = hci_le_set_scan_parameters(handle_, 0x01, htobs(0x0010), htobs(0x0010), 0x00, 0x00, 1000)) == 0)
    {
        if( (err = hci_le_set_scan_enable(handle_, 0x01, 1, 1000)) == 0)
        {
            _log("le scanning...");
        }
        else _log_error(" err: %i",err);
    }
    else _log_error(" err: %i",err);
}

void bluezcentralplugin::stop_scan()
{

}

void bluezcentralplugin::connect_device(device& dev)
{
    bluezperipheraldevice* bdev = reinterpret_cast<bluezperipheraldevice*>(&dev);
    bdev->push(new hciconnectdevicemessage(handle_,&observer_));
}

void bluezcentralplugin::disconnect_device(device& dev)
{

}

void bluezcentralplugin::cancel_pending_connection(device& dev)
{

}

void bluezcentralplugin::discover_services(device& dev)
{
    bluezperipheraldevice* bdev = reinterpret_cast<bluezperipheraldevice*>(&dev);
    bdev->push(new hciattreadbygroupmessage(handle_,&observer_,0x0001,0xFFFF));
}

void bluezcentralplugin::discover_characteristics(device& dev, const service& srv)
{

}

void bluezcentralplugin::read_characteristic_value(device& dev,const service& srv, const characteristic& chr)
{

}

void bluezcentralplugin::write_characteristic_value(device& dev,const service& srv, const characteristic& chr, const std::string& data, characteristic_properties type)
{

}

void bluezcentralplugin::set_characteristic_notify(device& dev,const service& srv, const characteristic& chr, bool notify)
{

}

void bluezcentralplugin::write_descriptor(device& dev, const service& srv, const characteristic& chr, descriptor& desc, bool notify)
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

void bluezcentralplugin::update_parameters(device& dev, const connectionparameters& params)
{

}

void bluezcentralplugin::write_btle_ftp(device& dev, const std::string& buffer)
{

}

int bluezcentralplugin::read_btle_ftp(device& dev, std::string& buffer, bool ack)
{
    return 0;
}

bluezperipheraldevice* bluezcentralplugin::find_device(const bda& addr)
{
    for( device_list::iterator it = devices_.begin(); it != devices_.end(); ++it )
    {
        if( (*it)->addr() == addr )
        {
            return (bluezperipheraldevice*)(*it);
        }
    }
    return NULL;
}

