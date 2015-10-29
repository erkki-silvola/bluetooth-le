#ifndef GATTDATABASE_H
#define GATTDATABASE_H

#include "btle/base.h"
#include "btle/service.h"

namespace btle {
    class BTLE_API gattdatabase: public base {
    public:
        gattdatabase();

    public: // api

        gattdatabase& operator << (const service& srv);
        gattdatabase& operator << (const service_list& services);

    public: // api
        
        std::vector<service>& services();

        const characteristic* fetch_characteristic(const uuid& uid) const;
        const characteristic* fetch_characteristic(const uuid_pair& pair) const;

        const service* fetch_service(const uuid& uid) const;
        const service* fetch_service_by_chr_uuid(const uuid& uid) const;
        const service* fetch_service_by_descriptor(const descriptor& desc) const;

        void update_characteristic_data(const service& srv, const characteristic& chr, const std::string& data);
        void update_descriptor(const service& srv, const characteristic& chr, const descriptor& desc, bool notifying);
        void update_service_chrs(const service& srv, const chr_list& chrs);
        
    public: // from base

        void invalidate();

    private:

        std::vector<service> services_;
    };
}

#endif // GATTDATABASE_H

