/* Copyright 2019, Denis Kormalev
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of the copyright holders nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef ASYNQRO_SPINLOCK_H
#define ASYNQRO_SPINLOCK_H

#include <QThread>

#include <atomic>

namespace asynqro::detail {
class SpinLock
{
public:
    SpinLock() noexcept {}
    SpinLock(const SpinLock &) = delete;
    SpinLock(SpinLock &&) = delete;
    SpinLock &operator=(const SpinLock &) = delete;
    SpinLock &operator=(SpinLock &&) = delete;

    inline void lock()
    {
        while (!tryLock())
            QThread::msleep(1);
    }

    inline bool tryLock()
    {
        bool result = !m_lock.test_and_set(std::memory_order_acq_rel);
        for (size_t i = 0; !result && i < 1024; ++i)
            result = !m_lock.test_and_set(std::memory_order_acq_rel);

        return result;
    }

    inline void unlock() { m_lock.clear(std::memory_order_release); }

private:
    std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
};

class SpinLockHolder
{
public:
    SpinLockHolder(SpinLock *lock) noexcept : m_lock(lock)
    {
        m_lock = lock;
        if (m_lock)
            m_lock->lock();
    }
    SpinLockHolder(const SpinLockHolder &) = delete;
    SpinLockHolder(SpinLockHolder &&) = delete;
    SpinLockHolder &operator=(const SpinLockHolder &) = delete;
    SpinLockHolder &operator=(SpinLockHolder &&) = delete;
    ~SpinLockHolder() { unlock(); }

    inline void unlock()
    {
        if (m_lock)
            m_lock->unlock();
        m_lock = nullptr;
    }

private:
    SpinLock *m_lock = nullptr;
};

} // namespace asynqro::detail

#endif // ASYNQRO_SPINLOCK_H
