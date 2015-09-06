
#include "btle/central/linux/hciattwritemessage.h"
#include "btle/central/centralpluginobserver.h"
#include "btle/central/linux/bluezperipheraldevice.h"

#include <assert.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <cstring>
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


hciattwritemessage::hciattwritemessage(int handle, centralpluginobserver *observer, uint16_t att_handle)
{

}

void hciattwritemessage::process(bluezperipheraldevice *dev)
{
    //TODO
    uint8_t packet[3]={0};
    packet[0]=0x12; // write
    memcpy(&packet[1],&att_handle_,sizeof(att_handle_));
    if( int err = dev->att_socket_.write(std::string((const char*)&packet,3)) )
    {

    }
}

