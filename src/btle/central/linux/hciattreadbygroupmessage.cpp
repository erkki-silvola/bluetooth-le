#include "btle/central/linux/hciattreadbygroupmessage.h"
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


hciattreadbygroupmessage::hciattreadbygroupmessage(int handle, centralpluginobserver *observer, uint16_t start_handle, uint16_t end_handle)
{

}

void hciattreadbygroupmessage::process(bluezperipheraldevice *dev)
{
    uint8_t packet[6];
    uint16_t primary_service = 0x2800;
    std::memcpy(&packet[0],&start_handle_,sizeof(start_handle_));
    std::memcpy(&packet[sizeof(start_handle_)],&end_handle_,sizeof(end_handle_));
    std::memcpy(&packet[sizeof(start_handle_)+sizeof(end_handle_)],&primary_service,sizeof(primary_service));

    switch( int ret = ::write(handle_,&packet,6) )
    {
        case -1:
        {
            break;
        }
        default:
        {
            uint8_t buffer[1024];
            switch( ret = ::read(handle_,&buffer,1024) )
            {
                case -1:
                {
                    break;
                }
                default:
                {
                switch (buffer[0]) {
                    case 0x11: // read by group type response

                        break;
                    default:
                        // forward all other message to some base handler
                        break;
                    }
                    break;
                }
            }
            break;
        }
    }
}
