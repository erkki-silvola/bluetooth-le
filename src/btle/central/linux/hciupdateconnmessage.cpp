#include "btle/central/linux/hciupdateconnmessage.h"
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

hciupdateconnmessage::hciupdateconnmessage(int handle, centralpluginobserver* observer, const connectionparameters &parameters)
: handle_(handle),
  observer_(observer),
  parameters_(parameters)
{
}

void hciupdateconnmessage::process(bluezperipheraldevice* dev)
{
    if( !hci_le_conn_update(handle_,
                       dev->connection_handle(),
                       parameters_.interval_min(),
                       parameters_.interval_max(),
                       parameters_.latency(),
                       parameters_.timeout(),
                       0) )
    {
        observer_->device_connection_parameters_updated(*dev);
    }
}
