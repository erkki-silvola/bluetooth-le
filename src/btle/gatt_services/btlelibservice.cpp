
#include "btle/gatt_services/btlelibservice.h"
#include "btle/gatt_services/gattserviceregisterer.h"
#include "btle/log.h"

#include <assert.h>
#include <cstring>
#include <sstream>

using namespace btle::gatt_services;

namespace {
    #if 0
    gattserviceregisterer<btlelibservice> registration;
    #endif
}

btlelibservice::btlelibservice()
: gattservicebase(),
  out_(),
  in_(),
  in_ctx_(),
  out_ctx_(),
  in_mutex_(),
  out_mutex_(),
  in_cond_(),
  out_cond_()
{
    service_ = uuid(BTLE_SERVICE);
    mandatory_notifications_.push_back(uuid(BTLE_MTU));
    included_characteristics_.push_back(uuid(BTLE_VERSION));
    included_characteristics_.push_back(uuid(BTLE_MTU));
}

void btlelibservice::process_service_notify_data(const uuid& chr, const uint8_t* data, size_t size)
{
    if( chr == BTLE_MTU )
    {
        msg_ack type = {0};
        memcpy(&type,data,sizeof(type));
        if(type.ack)
        {
            in_ack_mutex_.lock();
            in_ack_.push_back(std::string((const char*)data,size));
            in_ack_cond_.notify_all();
            in_ack_mutex_.unlock();
        }
        else
        {
            in_queue_.push_notify(std::string((const char*)data,size));
            /*in_mutex_.lock();
            in_.push_back(std::string((const char*)data,size));
            in_cond_.notify_all();
            in_mutex_.unlock();*/
        }
    }
}

void btlelibservice::process_service_value_read(const uuid& chr, const uint8_t* data, size_t size, const error& err)
{
    if( chr == BTLE_VERSION )
    {
        // TODO
    }
}

void btlelibservice::reset()
{
    
}

int btlelibservice::write_service_value(const uuid& chr, uint8_t id, const std::string& data, device *dev, gattservicetx* tx)
{
    assert(chr == BTLE_MTU);

    out_mutex_.lock();
    out_.push_back(std::make_pair(id,data));
    out_cond_.notify_all();
    out_mutex_.unlock();

    return 0;
}

void btlelibservice::packet_in(const std::string& data)
{
    msg_ack type = {0};
    memcpy(&type,data.c_str(),sizeof(type));
    in_mutex_.lock();
    if(type.ack) in_ack_.push_back(data);
    else         in_.push_back(data);
    in_cond_.notify_all();
    in_mutex_.unlock();
}

bool btlelibservice::is_empty(std::deque<std::string>& array, std::mutex& mutex)
{
    bool empty(false);
    mutex.lock();
    empty = array.size();
    mutex.unlock();
    return empty;
}

std::string btlelibservice::take_front(std::deque<std::string>& array, std::mutex& mutex)
{
    std::string msg;
    mutex.lock();
    msg = array.front();
    array.pop_front();
    mutex.unlock();
    return msg;
}

std::string btlelibservice::take_last_message() const
{
    return "";
}

void btlelibservice::out_queue()
{
    func_log

    do{
        size_t out_size(0);
        {
            std::unique_lock<std::mutex> lock(out_mutex_);
            out_cond_.wait(lock);
            out_size = out_.size();
        }
        if( out_size ){
            uint8_t rc(0);
            out_mutex_.lock();
            std::pair<uint8_t,std::string> message(out_.front());
            size_t total_size(message.second.size());
            out_.pop_front();
            out_mutex_.unlock();
            std::stringstream ss(message.second);
            // make first air packet
            first_air_packet sof = {0};
            sof.more = true;
            sof.first = true;
            sof.rc = rc++;
            sof.file_size = message.second.size();
            ss.read((char*)sof.payload[0],SOF_MAX_PAYLOAD);
            size_t count(ss.gcount());
            tx_->write_btle_ftp(*origin_,std::string((const char*)&sof,ss.gcount()+3));
            // TODO inform in main context
            listener_->out_progress(origin_,message.first,(count/total_size)*100.0);
            if(!ss.eof())
            {
                do{
                    // ok to start pump packets
                    msg_payload payload={0};
                    payload.first = false;
                    payload.more = true;
                    payload.rc = rc == 0x10 ? rc=0 : rc++;
                    ss.read((char*)&payload.payload[0],19);
                    count += ss.gcount();
                    if(!ss.eof())
                    {
                        tx_->write_btle_ftp(*origin_,std::string((const char*)&payload,ss.gcount()+1));
                        listener_->out_progress(origin_,message.first,(count/total_size)*100.0);
                        if( rc == 0x0F )
                        {
                            // read ack
                            if(is_empty(in_ack_,in_ack_mutex_))
                            {
                                // TODO wait with timeout!
                                std::unique_lock<std::mutex> lock(in_ack_mutex_);
                                in_ack_cond_.wait(lock);
                            }
                            std::string data = take_front(in_ack_,in_ack_mutex_);
                            msg_ack ack={0};
                            memcpy(&ack,data.c_str(),sizeof(ack));
                            assert(ack.ack);
                            if(ack.abort)
                            {
                                // abort streaming
                                btle::error err(-1);
                                listener_->out_complete(origin_,message.first,err);
                                break;
                            }
                        }
                    }
                    else
                    {
                        // last packet
                        payload.more = false;
                        tx_->write_btle_ftp(*origin_,std::string((const char*)&payload,ss.gcount()+1));
                        listener_->out_progress(origin_,message.first,(count/total_size)*100.0);
                        btle::error err(0);
                        listener_->out_complete(origin_,message.first,err);
                        // read ack
                        if(is_empty(in_ack_,in_ack_mutex_))
                        {
                            // TODO wait with timeout!
                            std::unique_lock<std::mutex> lock(in_ack_mutex_);
                            in_ack_cond_.wait(lock);
                        }
                        std::string data = take_front(in_ack_,in_ack_mutex_);
                        msg_ack ack={0};
                        memcpy(&ack,data.c_str(),sizeof(ack));

                        break;
                    }
                }while(!ss.eof());
            }
            else
            {
                // first and last packet
            }
        }
    }while (true);
}

void btlelibservice::in_queue()
{
    func_log

    do{
        {
            //std::unique_lock<std::mutex> lock(in_mutex_);
            //in_cond_.wait(lock);
        }
        //if(in_.size())
        {
            std::string packet = in_queue_.pop_wait();//take_front(in_,in_mutex_);
            first_air_packet sof={0};
            memcpy(&sof,packet.c_str(),packet.size());
            listener_->in_progress(origin_,sof.identifier,((packet.size()-3)/sof.file_size)*100.0);
            assert(sof.first);
            if(sof.more)
            {
                msg_payload payload={0};
                do{
                    if(is_empty(in_,in_mutex_))
                    {
                        // TODO wait with timeout!
                        std::unique_lock<std::mutex> lock(in_mutex_);
                        in_cond_.wait(lock);
                    }
                    packet = take_front(in_,in_mutex_);
                    memcpy(&payload,packet.c_str(),sizeof(payload));
                    if(payload.rc == 0x0F )
                    {
                        //
                        msg_ack ack={0};
                        ack.abort=false;
                        ack.ack=true;
                        ack.retransmit=false;
                        ack.reserved=0;
                        tx_->write_btle_ftp(*origin_,std::string((const char*)&ack,sizeof(ack)));
                    }
                }while(payload.more);
            }
            else
            {
                // first and last packet respond with ack
                msg_ack ack={0};
                ack.abort=false;
                ack.ack=true;
                ack.retransmit=false;
                ack.reserved=0;
                tx_->write_btle_ftp(*origin_,std::string((const char*)&ack,sizeof(ack)));
            }
        }
    }while(true);
}

