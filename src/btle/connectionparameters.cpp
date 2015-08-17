

#include "btle/connectionparameters.h"
#include "btle/utility.h"

namespace {
    #define INTERVAL_MULTIPLYER 1.25
}

using namespace btle;

connectionparameters::connectionparameters()
: base("","Connection parameters not available!"),
  connection_interval_(0),
  connection_latency_(0),
  connection_timeout_(0),
  connection_interval_min_(0),
  connection_interval_max_(0)
{
}

connectionparameters::connectionparameters(
    uint16_t connection_interval,
    uint16_t connection_latency,
    uint16_t connection_timeout)
: base("","Connection parameters interval: " + utility::to_string(connection_interval*1.25) +
          " Latency: " + utility::to_string(connection_latency) +
          " Timeout: " + utility::to_string(connection_timeout)),
  connection_interval_(connection_interval),
  connection_latency_(connection_latency),
  connection_timeout_(connection_timeout)
{
}

uint16_t connectionparameters::interval() const
{
    return connection_interval_;
}

uint16_t connectionparameters::interval_min() const
{
    return connection_interval_min_;
}

uint16_t connectionparameters::interval_max() const
{
    return connection_interval_max_;
}

uint16_t connectionparameters::latency() const
{
    return connection_latency_;
}

uint16_t connectionparameters::timeout() const
{
    return connection_timeout_;
}

void connectionparameters::invalidate()
{
    connection_interval_=0;
    connection_latency_=0;
    connection_timeout_=0;
}
