#ifndef ATOMICLIST_H
#define ATOMICLIST_H

#include <deque>
// C++ 11
#include <mutex>
#include <condition_variable>

namespace btle {
    template<typename T>
    /**
     * @brief The atomiclist class, general purpose atomic list for all atomic use
     */
    class atomiclist
    {
    public:
        typedef typename std::deque<T>::iterator list_iterator;
        typedef typename std::deque<T>::const_iterator list_iterator_const;

        atomiclist();
        void push(const T& object);
        void push_notify(const T& object);
        size_t size();
        T pop();
        T pop_wait();
        T pop_wait(int seconds);

    private:

        std::deque<T> list_;
        std::mutex mutex_;
        std::condition_variable cond_;
    };

    #include "atomiclist.hpp"
}

#endif // ATOMICLIST_H

