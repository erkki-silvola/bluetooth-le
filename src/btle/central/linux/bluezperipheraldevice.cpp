
#include "btle/central/linux/bluezperipheraldevice.h"

using namespace btle;
using namespace btle::central;
using namespace btle::central::linux_platform;

namespace {
    void routine(void* context)
    {
        bluezperipheraldevice* parent(reinterpret_cast<bluezperipheraldevice*>(context));
        parent->message_thread();
    }
}

bluezperipheraldevice::bluezperipheraldevice(const bda &addr)
: device(addr),
  thread_(),
  queue_(),
  q_mutex_(),
  q_condition_(),
  started_(),
  messages_(),
  att_socket_()
{
    thread_ = std::thread(::routine,this);
    std::unique_lock<std::mutex> lock(q_mutex_);
    started_.wait(lock);
}

void bluezperipheraldevice::push(messagebase* message)
{
    /*std::unique_lock<std::mutex> lock(q_mutex_);
    queue_.push_back(message);
    q_condition_.notify_all();*/
    messages_.push_notify(message);
}

void bluezperipheraldevice::message_thread()
{
    q_mutex_.lock();
    started_.notify_all();
    q_mutex_.unlock();
    do{
        /*{
            std::unique_lock<std::mutex> lock(q_mutex_);
            q_condition_.wait(lock);
            msg = queue_.front();
            queue_.pop_front();
        }*/
        messagebase* msg(messages_.pop_wait());
        if(msg)
        {
            msg->process(this);
            delete msg;
        }
    }while(true);
}
