#ifndef CONNECTIONPARAMETERS_H
#define CONNECTIONPARAMETERS_H

#include "btle_global.h"
#include "btle/base.h"

namespace btle {
    /**
     * @brief The connectionparameters class, common class to hold connection parameters
     * NOTE not all platforms offer this
     */
    class BTLE_API connectionparameters : public base{
    public:

        connectionparameters();
        connectionparameters( uint16_t connection_interval,
                              uint16_t connection_latency,
                              uint16_t connection_timeout );

    public: //

        uint16_t interval() const;
        uint16_t interval_min() const;
        uint16_t interval_max() const;
        uint16_t latency() const;
        uint16_t timeout() const;

    public: // from base

        void invalidate();

    private:

        uint16_t connection_interval_;
        uint16_t connection_latency_;
        uint16_t connection_timeout_;
        uint16_t connection_interval_min_;
        uint16_t connection_interval_max_;
        /*
            add later these
        uint8_t  clock_accuracy_;
        uint8_t  connection_effective_interval_;
        uint16_t connection_interval_min_;
        uint16_t connection_interval_max_;*/
    };
}

#endif // CONNECTIONPARAMETERS_H

