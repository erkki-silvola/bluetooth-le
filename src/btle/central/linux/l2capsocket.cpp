
#include "btle/central/linux/l2capsocket.h"
#include "btle/log.h"

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

using namespace btle;
using namespace btle::central;
using namespace btle::central::linux_platform;

namespace {
    void routine(void* context)
    {
        l2capsocket* self = reinterpret_cast<l2capsocket*>(context);
    }
}

l2capsocket::l2capsocket()
: fd_(0)
{
}

int l2capsocket::connect(const bda &addr, unsigned int channel_id)
{
    fd_ = socket(PF_BLUETOOTH,SOCK_SEQPACKET,BTPROTO_L2CAP);
    if(fd_!=-1)
    {
        struct sockaddr_l2 l2addr={0};

        l2addr.l2_family = AF_BLUETOOTH;
        l2addr.l2_cid = htobs(channel_id);

        switch (addr.type())
        {
            case btle::ADDR_PRIVATE_NONRESOLVE:
            case btle::ADDR_PRIVATE_RESOLVE:
                l2addr.l2_bdaddr_type = BDADDR_LE_RANDOM;
                break;
            case btle::ADDR_PUBLIC:
            case btle::ADDR_STATIC:
                l2addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
                break;
            default:
                break;
        }

        int err = ::connect(fd_, reinterpret_cast<sockaddr*>(&l2addr), sizeof(l2addr));
        if(!err)
        {

        }
    }
    return fd_;
}

void l2capsocket::disconnect()
{
    ::close(fd_);
}

int l2capsocket::write(const std::string &packet)
{
    return ::write(fd_,packet.data(),packet.size());
    /*switch(int err = ::write(fd_,packet.data(),packet.size()))
    {
        case 0:
            break;
        case -1:
        {
            // TODO handle error
            break;
        }
        default:
        {
            _log("bytes written: %i",err);
            break;
        }
    }
    return err;*/
}

std::string l2capsocket::read_packet()
{
    uint8_t buffer[1024];
    switch(int err = ::read(fd_,&buffer,1024))
    {
        case -1:
        {
            // TODO
            return "";
        }
        default:
        {
            return std::string((const char*)&buffer,err);
            break;
        }
    }
}
