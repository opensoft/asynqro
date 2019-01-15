#include "thread_pool.hpp"

#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

using namespace tp;

#ifndef CONCURRENCY
#    define CONCURRENCY 4
#endif

#ifndef JOBS_COUNT
#    define JOBS_COUNT 1000000
#endif

#ifndef CORES
#    define CORES 8
#endif

static std::atomic_llong USEFUL{0};

struct RepostJob
{
    ThreadPool *thread_pool;

    volatile size_t counter;
    long long int begin_count;
    std::promise<void> *waiter;

    RepostJob(ThreadPool *thread_pool, std::promise<void> *waiter)
        : thread_pool(thread_pool), counter(0), waiter(waiter)
    {
        begin_count = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    }

    void operator()()
    {
        if (++counter < JOBS_COUNT) {
            thread_pool->post(*this);
        } else {
            long long int end_count = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::cout << "reposted " << counter << " in " << (double)(end_count - begin_count) / (double)1000000
                      << " ms" << std::endl;
            waiter->set_value();
        }
    }
};

int main(int, const char *[])
{
    std::cout << "Benchmark job repost (empty): " << CONCURRENCY << "/" << JOBS_COUNT << std::endl;
    {
        std::cout << "***thread pool cpp***" << std::endl;

        std::promise<void> waiters[CONCURRENCY];
        ThreadPool thread_pool;
        for (auto &waiter : waiters)
            thread_pool.post(RepostJob(&thread_pool, &waiter));

        for (auto &waiter : waiters)
            waiter.get_future().wait();
    }
    return 0;
}
