
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


hciattwritemessage::hciattwritemessage(
    int handle,
    centralpluginobserver *observer,
    const service &srv,
    const characteristic &chr)
: handle_(handle),
  observer_(observer),
  srv_(srv),
  chr_(chr),
  type_(characteristic_write)
{
}

hciattwritemessage::hciattwritemessage(
    int handle,
    centralpluginobserver *observer,
    const service &srv,
    const characteristic &chr,
    const descriptor& desc)
: handle_(handle),
  observer_(observer),
  srv_(srv),
  chr_(chr),
  desc_(desc),
  type_(descriptor_write)
{
}

void hciattwritemessage::process(bluezperipheraldevice *dev)
{
    //TODO
    switch(type_){
        case characteristic_write:{
            uint8_t packet[3]={0};
            packet[0]=0x12; // write
            uint16_t handle(chr_.attribute_handle());
            memcpy(&packet[1],&handle,sizeof(chr_.attribute_handle()));
            if( int err = dev->att_socket_.write(std::string((const char*)&packet,3)) )
            {

            }
            break;
        }
        case descriptor_write:{
            switch(desc_.type())
            {
                case CLIENT_CHARACTERISTIC_CONFIGURATION:
                {
                    //
                    if( int err = dev->att_socket_.write(std::string("\x01\x00",2)) )
                    {

                    }
                    break;
                }
            }
            break;
        }
        default:
            assert(false);
    }
}
