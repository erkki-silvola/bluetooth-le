
#include "btle/central/linux/hciattreadbytypemessage.h"
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


hciattreadbytypemessage::hciattreadbytypemessage(int handle, centralpluginobserver *observer, uint16_t att_handle, uint16_t type)
{

}

void hciattreadbytypemessage::process(bluezperipheraldevice *dev)
{

}
