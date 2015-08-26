
#include "btle/central/linux/hciconnectdevicemessage.h"
#include "btle/central/centralpluginobserver.h"
#include "btle/central/linux/bluezperipheraldevice.h"

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

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

using namespace btle;
using namespace btle::central;
using namespace btle::central::linux_platform;

hciconnectdevicemessage::hciconnectdevicemessage(int handle,centralpluginobserver* observer)
: handle_(handle),
  observer_(observer)
{
}

void hciconnectdevicemessage::process(bluezperipheraldevice* dev)
{
    bdaddr_t bdaddr={0};
    uint16_t interval, latency, max_ce_length, max_interval, min_ce_length;
    uint16_t min_interval, supervision_timeout, window;
    uint8_t initiator_filter, own_bdaddr_type, peer_bdaddr_type;
    peer_bdaddr_type = (uint8_t)dev->addr().type();
    initiator_filter = 0;

    for(int i=5; i!=-1; --i)
    {
        bdaddr.b[i]=*(dev->addr().string_value().c_str()+i);
    }
    interval = htobs(0x0004);
    window = htobs(0x0004);
    own_bdaddr_type = 0x00;
    min_interval = htobs(0x000F);
    max_interval = htobs(0x000F);
    latency = htobs(0x0000);
    supervision_timeout = htobs(0x0C80);
    min_ce_length = htobs(0x0001);
    max_ce_length = htobs(0x0001);

    uint16_t device_handle(0);
    int err = hci_le_create_conn(handle_, interval, window, initiator_filter,
    peer_bdaddr_type, bdaddr, own_bdaddr_type, min_interval,
    max_interval, latency, supervision_timeout,
    min_ce_length, max_ce_length, &device_handle, 25000);
    if( err == 0 )
    {
        dev->set_connection_handle(handle_);
        dev->att_socket_.connect(dev->addr(),4);
        observer_->device_connected(*dev);
    }
    else
    {
        observer_->device_disconnected(*dev);
    }
}
