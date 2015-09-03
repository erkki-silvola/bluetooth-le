
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
#include <stdlib.h>
#include <glib.h>

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
        perror("connection error");
        observer_->device_disconnected(*dev);
    }

    /*const char *src;
    const char *dst;
    const char *dst_type;
    const char *sec_level;
    int psm;
    int mtu;
    BtIOConnect connect_cb;
    GError **gerr;

    GIOChannel *chan;
    bdaddr_t sba, dba;
    uint8_t dest_type;
    GError *tmp_err = NULL;
    BtIOSecLevel sec;

    str2ba(dst, &dba);

    if (src != NULL) {
        if (!strncmp(src, "hci", 3))
                hci_devba(atoi(src + 3), &sba);
            else
                str2ba(src, &sba);
        } else
        bacpy(&sba, BDADDR_ANY);

        if (strcmp(dst_type, "random") == 0)
            dest_type = BDADDR_LE_RANDOM;
        else
        dest_type = BDADDR_LE_PUBLIC;

        if (strcmp(sec_level, "medium") == 0)
            sec = BT_IO_SEC_MEDIUM;
    else if (strcmp(sec_level, "high") == 0)
            sec = BT_IO_SEC_HIGH;
        else
        sec = BT_IO_SEC_LOW;

        if (psm == 0)
            chan = bt_io_connect(connect_cb, NULL, NULL, &tmp_err,
                    BT_IO_OPT_SOURCE_BDADDR, &sba,
                BT_IO_OPT_SOURCE_TYPE, BDADDR_LE_PUBLIC,
                    BT_IO_OPT_DEST_BDADDR, &dba,
                    BT_IO_OPT_DEST_TYPE, dest_type,
                BT_IO_OPT_CID, ATT_CID,
                    BT_IO_OPT_SEC_LEVEL, sec,
                    BT_IO_OPT_INVALID);
        else
            chan = bt_io_connect(connect_cb, NULL, NULL, &tmp_err,
                    BT_IO_OPT_SOURCE_BDADDR, &sba,
                    BT_IO_OPT_DEST_BDADDR, &dba,
                    BT_IO_OPT_PSM, psm,
                    BT_IO_OPT_IMTU, mtu,
                    BT_IO_OPT_SEC_LEVEL, sec,
                    BT_IO_OPT_INVALID);

        if (tmp_err) {
            g_propagate_error(gerr, tmp_err);
            return NULL;
        }*/
}
