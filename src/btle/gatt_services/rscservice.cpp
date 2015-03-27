
#include "btle/gatt_services/rscservice.h"
#include "btle/gatt_services/gattserviceregisterer.h"

#include <stdio.h>
#include <cstring>

using namespace btle::gatt_services;

namespace {
    gattserviceregisterer<rscservice> registration;
}

rscservice::rscservice()
{
    mandatory_notifications_.push_back(RSC_MEASUREMENT);
    included_characteristics_.push_back(RSC_MEASUREMENT);
    service_ = (RSC_SERVICE);
}

/**
 * @brief rscservice::speed, in km/h
 * @return
 */
double rscservice::speed() const
{
    return speed_;
}

/**
 * @brief rscservice::cadence, in RPM
 * @return
 */
int rscservice::cadence() const
{
    return cadence_;
}

/**
 * @brief rscservice::stride_length, in CM
 * @return
 */
uint16_t rscservice::stride_length() const
{
    return stride_length_;
}

/**
 * @brief rscservice::distance, in meters
 * @return
 */
double rscservice::distance() const
{
    return distance_;
}

void rscservice::process_service_notify_data(const uuid& chr, const uint8_t* data, size_t size)
{
    switch(chr.uuid16bit())
    {
        case RSC_MEASUREMENT:
        {
            int offset(1);
            memset(&flags_,0,sizeof(flags_));
            memcpy(&flags_,data,sizeof(flags_));

            uint16_t speed_mask(0);
            memcpy(&speed_mask, data+offset, sizeof(speed_mask));
            speed_ = ((double)speed_mask/256.0)*3.6;
            offset += sizeof(speed_mask);
            cadence_ = *(data + offset);
            offset += sizeof(uint8_t);
            stride_length_ = 0;
            distance_ = 0;
            if( flags_.stride_length_present_ )
            {
                memcpy(&stride_length_, data+offset, sizeof(stride_length_));
                offset += sizeof(stride_length_);
            }
            if( flags_.distance_present_ )
            {
                uint32_t distance_tmp(0);
                memcpy(&distance_tmp, data+offset, sizeof(distance_tmp));
                distance_ = ((double)distance_tmp*0.1);
            }
            break;
        }
    }
}

void rscservice::process_service_value_read(const uuid &chr, const uint8_t *data, size_t size, const error &err)
{

}

void rscservice::reset()
{

}


