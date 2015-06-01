
#ifndef btle_btlelibservice_h
#define btle_btlelibservice_h

#include "btle/gatt_services/gattservicebase.h"
#include "btle/gatt_services/gattservicetx.h"
#include "btle/central/centralbtleftptransferinterface.h"


#include <deque>

// c++ 11
#include <thread>
#include <mutex>
#include <condition_variable>

namespace btle {
    namespace gatt_services{

        /**
         * @brief message_pair,first unique identifier and message string
         */
        typedef std::pair<int,std::string> message_pair;

        /**
         * btle library service for data transfer between two devices running the library,
         * payload will be max 20 bytes, this is for backwards compatibility, NOTE this feature 
         * is coming in 2.x.x version
         */
        #define BTLE_SERVICE "\x42\x54\x4c\x45\x20\x4c\x49\x42\x20\x53\x45\x52\x56\x49\x43\x45"
        #define BTLE_MTU     "\x42\x54\x4c\x45\x20\x43\x48\x52\x20\x4d\x54\x55\x20\x31\x32\x38"
        #define BTLE_VERSION "\x42\x54\x4c\x45\x20\x43\x48\x52\x20\x46\x45\x41\x54\x55\x52\x45"
        
        struct first_air_packet{
            uint8_t more:1; // true more to come, false, first and last air packet
            uint8_t first:1; // true
            uint8_t out:1;
            uint8_t reserved:1;
            uint8_t rc:4; // zero
            uint16_t file_size; // upcoming/incoming file size
            uint8_t payload[17]; // payload
        };
        
        /**
         * message out packet, 
         * more, true = more is coming , false = last packet eof
         * first = false
         * reserved = 0
         * rc = UPWARDS ( it is stupid to do downwards rc! ) counting ring counter
         * payload
         */
        struct msg_payload{
            uint8_t more:1;
            uint8_t first:1;
            uint8_t reserved:2;
            uint8_t rc:4;
            uint8_t payload[19];
        };
        
        /**
         * message ack packet, after
         * ack = true ok to send send more, false stream cancel
         * retransmit = true send last frame again
         */
        struct msg_ack
        {
            uint8_t ack:1;
            uint8_t retransmit:1;
            uint8_t reserved:6;
        };
        
        class btlelibservicetransferlistener{
        public:
            virtual void out_progress(device* dev, int id, double progress)=0;
            virtual void in_progress(device* dev, double progress)=0;
        };

        class btlelibservice: public gattservicebase{
        public:
            btlelibservice();
            
        public:
            
            void process_service_notify_data(const uuid& chr, const uint8_t* data, size_t size);
            void process_service_value_read(const uuid& chr, const uint8_t* data, size_t size, const error& err);
            void reset();

            int write_service_value(const uuid& chr, const std::string& data, device* dev, gattservicetx *tx);
            void packet_in(const std::string& data);

        public: // API
            
            std::string take_last_message() const;

        private: //

            void out_queue();
            void in_queue();

        private:
            
            std::deque<std::string> out_;
            std::deque<std::string> in_;
            int status_;
            std::thread in_ctx_;
            std::thread out_ctx_;
            std::mutex in_mutex_;
            std::mutex out_mutex_;
            std::condition_variable in_cond_;
            std::condition_variable out_cond_;
            //gattservicetx* tx_;
            central::centralbtleftptransferinterface* tx_;
            device* origin_;
            btlelibservicetransferlistener* listener_;
        };
    }
}

#endif
