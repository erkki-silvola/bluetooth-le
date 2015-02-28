#ifndef GATTDATABASE_H
#define GATTDATABASE_H

#include "btle/base.h"
#include "btle/service.h"

namespace btle {
    class gattdatabase: public base {
    public:
        gattdatabase();

    public: // api

        gattdatabase& operator << (const service& srv);

    private:

        std::vector<service> services_;
    };
}

#endif // GATTDATABASE_H

