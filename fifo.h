/**
 * @file fifo.h
 * @author Max Beddies (max dot beddies at t dash online dot de)
 * @brief Implementation of a blocking fifo queue for concurrent use
 * @version 0.1
 * @date 2022-08-24
 * 
 * This implementation was taken from my master's thesis implementation at KIT.
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <memory>
#include <mutex>
#include <queue>


/**
 * @brief fifo queue with synchronized access
 *
 * @tparam T value type of the queue
 */
template <typename T>
class blocking_fifo
{
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = value_type const&;
    using size_type = std::size_t;


    /**
     * @brief removes an element from the front of the queue
     *
     * @return the removed element
     */
    value_type
    pop()
    {
        std::lock_guard<std::mutex> lock {_mtx};

        value_type result {std::move(_container->front())};
        _container->pop();

        return result;
    }


    /**
     * @brief inserts an element at the end of the queue
     *
     * @param item the element to insert
     */
    void
    push(const_reference item)
    {
        std::lock_guard<std::mutex> lock {_mtx};
        _container->push(item);
    }


    /**
     * @brief query the front of the queue without removal
     *
     * @return first element of the queue
     */
    value_type
    front()
    {
        std::lock_guard<std::mutex> lock {_mtx};

        return _container->front();
    }


    /**
     * @brief checks whether the queue is empty
     *
     * @return true if the queue is empty, false otherwise
     */
    bool
    empty()
    {
        std::lock_guard<std::mutex> lock {_mtx};
        return _container->empty();
    }


    /**
     * @brief get the size of the queue
     *
     * @return size of the queue
     */
    size_type
    size()
    {
        std::lock_guard<std::mutex> lock {_mtx};
        return _container->size();
    }


    /**
     * @brief Construct a new blocking fifo
     *
     */
    explicit blocking_fifo()
    {
        _container = std::make_unique<std::queue<value_type>>();
    }

private:

    std::unique_ptr<std::queue<value_type>> _container;
    std::mutex _mtx;
};

template <typename T>
using fifo_ptr = std::unique_ptr<blocking_fifo<T>>;
