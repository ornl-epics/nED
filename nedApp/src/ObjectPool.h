/* ObjectPool.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include <cantProceed.h>
#include <epicsMutex.h>

#include <functional>
#include <list>
#include <memory>

/**
 * A generic pool-like container.
 *
 * Thread safe, dynamicly growing. When object is requested using get(),
 * an existing object is returned if available or a new object is allocated.
 * When object is no longer needed, it can be returned to pool using put().
 * Alternatively use getPtr() which returns shared_ptr object which is returned
 * to pool as soon as shared_ptrs' reference drops to 0.
 */
template <typename T>
class ObjectPool {
    public:
        ObjectPool(bool recycleSmaller)
        : m_recycleSmaller(recycleSmaller)
        {}

        /**
         * Retrieve object from queue.
         *
         * If big enough object is available, it's returned immediately.
         * If free objects exists but they're smaller than requested size,
         * they are automatically deleted if recycle is enabled.
         * When no more free objects, a new object of the requested size is
         * allocated.
         *
         * \param[in] minSize in bytes
         */
        T *get(size_t minSize=sizeof(T))
        {
            void *mem = 0;
            m_mutex.lock();
            if (m_recycleSmaller) {
                while (m_ready.size() > 0) {
                    Object &obj = m_ready.front();
                    m_ready.pop_front();
                    if (obj.size >= minSize) {
                        mem = obj.mem;
                        m_used.push_back(obj);
                        break;
                    }
                    free(obj.mem);
                }
            } else {
                for (auto it=m_ready.begin(); it!=m_ready.end(); it++) {
                    if (it->size >= minSize) {
                        mem = it->mem;
                        m_used.push_back(*it);
                        m_ready.erase(it);
                        break;
                    }
                }
            }
            m_mutex.unlock();

            if (mem == 0) {
                // Not found any suitable element, must allocate
                mem = mallocMustSucceed(minSize, "Can't allocate pool object");

                Object obj = { mem, minSize };
                m_mutex.lock();
                m_used.push_back(obj);
                m_mutex.unlock();
            }
            return reinterpret_cast<T*>(mem);
        }

        /**
         * Retrieve object from queue.
         *
         * Same as get() function, except the returned is a managed shared_ptr
         * that will automatically call put() function when reference counter
         * drops to 0.
         *
         * \param[in] minSize in bytes
         */
        std::shared_ptr<T> getPtr(size_t minSize)
        {
            auto deleter = std::bind(&ObjectPool<T>::put, this, std::placeholders::_1);
            return std::shared_ptr<T>(get(minSize), deleter);
        }

        /**
         * Put object back to pool and ready for reuse.
         *
         * If alian object (not allocated through ObjectPool), its' memory is
         * simply deleted.
         */
        void put(T* mem)
        {
            bool returned = false;

            m_mutex.lock();
            for (auto it=m_used.begin(); it!=m_used.end(); it++) {
                if (it->mem == mem) {
                    m_ready.push_back(*it);
                    m_used.erase(it);
                    returned = true;
                    break;
                }
            }
            m_mutex.unlock();

            if (!returned)
                free(mem);
        }

        /**
         * Return number of currently allocated and used objects.
         */
        size_t size()
        {
            size_t size = 0;
            m_mutex.lock();
            size = m_used.size();
            m_mutex.unlock();
            return size;
        }

        /**
         * Return number of all allocated objects.
         */
        size_t capacity()
        {
            size_t size = 0;
            m_mutex.lock();
            size = m_used.size() + m_ready.size();
            m_mutex.unlock();
            return size;
        }

    protected:

        typedef struct {
            void *mem;
            size_t size;
        } Object;

        std::list<Object> m_ready;
        std::list<Object> m_used;
        epicsMutex m_mutex;
        bool m_recycleSmaller;
};
