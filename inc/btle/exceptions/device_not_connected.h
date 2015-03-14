#ifndef DEVICE_NOT_CONNECTED_H
#define DEVICE_NOT_CONNECTED_H

#include <stdexcept>
#include <string>
#include "btle_global.h"

namespace btle {
    namespace exceptions {
        class BTLE_API device_not_connected: public std::runtime_error{
        public:
            explicit device_not_connected(const std::string& what);
            device_not_connected();
            device_not_connected(const device_not_connected& other);
        };
    }
}

#endif // DEVICE_NOT_CONNECTED_H

