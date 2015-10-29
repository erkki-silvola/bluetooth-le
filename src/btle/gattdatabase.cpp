
#include "btle/gattdatabase.h"

using namespace btle;

gattdatabase::gattdatabase()
: base(),
  services_()
{
}

gattdatabase& gattdatabase::operator << (const service& srv)
{
    for (service_iterator_const it = services_.begin(); it != services_.end(); ++it) {
        if( (*it) == srv ){
            return *this;
        }
    }
    services_.push_back(srv);
    return *this;
}

gattdatabase& gattdatabase::operator << (const service_list& services)
{
    services_ = services;
    return *this;
}

std::vector<service>& gattdatabase::services()
{
    return services_;
}

const characteristic* gattdatabase::fetch_characteristic(const uuid& uid) const
{
    for( std::vector<service>::const_iterator it_srv = services_.begin(); it_srv != services_.end(); ++it_srv )
    {
        for(chr_iterator_const it_chr = it_srv->characteristics().begin(); it_chr != it_srv->characteristics().end(); ++it_chr )
        {
            if(it_chr->uuid() == uid)
            {
                return (const characteristic*)&(*it_chr);
            }
        }
    }
    return NULL;
}

const characteristic* gattdatabase::fetch_characteristic(const uuid_pair& pair) const
{
    for( std::vector<service>::const_iterator it_srv = services_.begin(); it_srv != services_.end(); ++it_srv )
    {
        if(it_srv->uuid() == pair.first)
        {
            for(chr_iterator_const it_chr = it_srv->characteristics().begin(); it_chr != it_srv->characteristics().end(); ++it_chr )
            {
                if(it_chr->uuid() == pair.second)
                {
                    return (const characteristic*)&(*it_chr);
                }
            }
        }
    }
    return NULL;
}

const service* gattdatabase::fetch_service(const uuid& uid) const
{
    for( std::vector<service>::const_iterator it_srv = services_.begin(); it_srv != services_.end(); ++it_srv )
    {
        if(it_srv->uuid() == uid)
        {
            return (const service*)&(*it_srv);
        }
    }
    return NULL;
}

const service* gattdatabase::fetch_service_by_chr_uuid(const uuid& uid) const
{
    for( std::vector<service>::const_iterator it_srv = services_.begin(); it_srv != services_.end(); ++it_srv )
    {
        for(chr_iterator_const it_chr = it_srv->characteristics().begin(); it_chr != it_srv->characteristics().end(); ++it_chr )
        {
            if(it_chr->uuid() == uid)
            {
                return (const service*)&(*it_srv);
            }
        }
    }
    return NULL;
}

const service* gattdatabase::fetch_service_by_descriptor(const descriptor& desc) const
{
    for( std::vector<service>::const_iterator it_srv = services_.begin(); it_srv != services_.end(); ++it_srv )
    {
        for(chr_iterator_const it_chr = it_srv->characteristics().begin(); it_chr != it_srv->characteristics().end(); ++it_chr )
        {
            for( descriptors::const_iterator it_desc = it_chr->descriptors().begin(); it_desc != it_chr->descriptors().end(); ++ it_desc)
            {
                if( desc == (*it_desc) )
                {
                    return (const service*)&(*it_srv);
                }
            }
        }
    }
    return NULL;
}

void gattdatabase::update_characteristic_data(const service& srv, const characteristic& chr, const std::string& data)
{
    for( std::vector<service>::iterator it_srv = services_.begin(); it_srv != services_.end(); ++it_srv )
    {
        if(srv == (*it_srv))
        {
            for(chr_iterator it_chr = it_srv->characteristics().begin(); it_chr != it_srv->characteristics().end(); ++it_chr )
            {
                if( chr == (*it_chr) )
                {
                    it_chr->set_string_value(data);
                    return;
                }
            }
        }
    }
}

void gattdatabase::update_descriptor(const service& srv, const characteristic& chr, const descriptor& desc, bool notifying)
{
    for( std::vector<service>::iterator it_srv = services_.begin(); it_srv != services_.end(); ++it_srv )
    {
        if(srv == (*it_srv))
        {
            for(chr_iterator it_chr = it_srv->characteristics().begin(); it_chr != it_srv->characteristics().end(); ++it_chr )
            {
                if( chr == (*it_chr) )
                {
                    for( descriptors::iterator it_desc = it_chr->descriptors().begin(); it_desc != it_chr->descriptors().end(); ++ it_desc)
                    {
                        if( desc == (*it_desc) )
                        {
                            it_desc->set_notifying(notifying);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void gattdatabase::update_service_chrs(const service& srv, const chr_list& chrs)
{
    for( std::vector<service>::iterator it_srv = services_.begin(); it_srv != services_.end(); ++it_srv )
    {
        if(srv == (*it_srv))
        {
            (*it_srv) << chrs;
        }
    }
}

void gattdatabase::invalidate()
{
    base::invalidate();
    services_.clear();
}

