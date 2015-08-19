#ifndef ATOMICLIST_HPP
#define ATOMICLIST_HPP

template <typename T>
atomiclist<T>::atomiclist()
: list_(),
  mutex_()
{
}

template <typename T>
void atomiclist<T>::push(const T& object)
{
    std::unique_lock<std::mutex> lock(mutex_);
    list_.push_back(object);
}

template <typename T>
void atomiclist<T>::push_notify(const T& object)
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.notify_all();
    list_.push_back(object);
}

template <typename T>
size_t atomiclist<T>::size()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return list_.size();
}

template <typename T>
T atomiclist<T>::pop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    T ret = list_.front();
    list_.pop_front();
    return ret;
}

template <typename T>
T atomiclist<T>::pop_wait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if(!list_.size()) cond_.wait(lock);
    T ret = list_.front();
    list_.pop_front();
    return ret;
}

template <typename T>
T atomiclist<T>::pop_wait(int seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if(!list_.size()) cond_.wait_for(lock,std::chrono::seconds(seconds));
    T ret= list_.front();
    list_.pop_front();
    return ret;
}



#endif // ATOMICLIST_HPP

