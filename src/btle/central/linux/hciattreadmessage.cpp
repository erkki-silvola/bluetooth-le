
#include "btle/central/linux/hciattreadmessage.h"
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


hciattreadmessage::hciattreadmessage(
    int handle,
    centralpluginobserver *observer,
    const service &srv,
    const characteristic &chr)
: handle_(handle),
  observer_(observer),
  srv_(srv),
  chr_(chr)
{
}

void hciattreadmessage::process(bluezperipheraldevice *dev)
{
    uint8_t packet[3]={0};
    packet[0]=0x0A; // read
    uint16_t handle(chr_.attribute_handle());
    memcpy(&packet[1],&handle,sizeof(chr_.attribute_handle()));
    if( int err = dev->att_socket_.write(std::string((const char*)&packet,3)) )
    {

    }
}

