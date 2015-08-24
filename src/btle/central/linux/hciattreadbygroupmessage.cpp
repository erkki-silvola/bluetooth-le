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
    if( dev->att_socket_.write(std::string((const char*)packet,6)) > 0 )
    {
        std::string packet = dev->att_socket_.read_packet();
        if(packet.size())
        {
            switch(*packet.c_str())
            {
                case 0x01: // att error
                {
                    break;
                }
                case 0x11: // att read by group type
                {
                    int len(*packet.c_str());
                    int count(packet.size()-1/len);
                    for(;;)
                    {
                        uint16_t start(0);
                        uint16_t end(0);
                        memcpy(&start,packet.c_str(),sizeof(start));
                        memcpy(&end,packet.c_str()+sizeof(start),sizeof(end));
                        //btle::uuid uid();
                    }
                    break;
                }
                default:
                {
                    // TODO forward to device to handle
                    break;
                }
            }
        }
    }
}
