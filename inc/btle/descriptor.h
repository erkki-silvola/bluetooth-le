#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <vector>
#include "btle/base.h"


namespace btle {
    
    // forward declaration
    class characteristic;
    
    #define CHARACTERISTIC_EXTENDED_PROPERTIES   0x2900
    #define CHARACTERISTIC_USER_DESCRIPTION      0x2901
    #define CLIENT_CHARACTERISTIC_CONFIGURATION  0x2902
    #define SERVER_CHARACTERISTIC_CONFIGURATION  0x2903
    #define CHARACTERISTIC_FORMAT                0x2904
    #define CHARACTERISTIC_AGGREGATE_FORMAT      0x2905

    class BTLE_API descriptor: public base
    {
    public:
        descriptor();
        descriptor(uint16_t type,
                   btle::characteristic* parent);
        descriptor(uint16_t type,
                   bool notifying,
                   btle::characteristic* parent);
        descriptor(uint16_t type,
                   uint16_t handle,
                   btle::characteristic* parent);
        descriptor(uint16_t handle);
        descriptor(const descriptor& other);

    public: // api

        uint16_t type() const;
        uint16_t handle() const;
        bool is_notifying() const;
        void set_notifying(bool notifying);
        characteristic* parent();
        // operators
        bool operator == (const descriptor& other) const;
        
    public: // from base

        void invalidate();

    private:
        uint16_t type_;
        uint16_t handle_;
        bool is_notifying_;
        characteristic* chr_;
        long int instance_id_;
    };
    
    typedef std::vector<descriptor> descriptors;
}

#endif // DESCRIPTOR_H
