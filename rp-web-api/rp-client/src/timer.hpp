/**
 * $Id$
 *
 * @brief Red Pitaya Web module
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#pragma once
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

// The worker threads are detached and outlive the Timer object: they share the stop
// flag instead of reading it through `this`, so replacing or destroying a Timer stops
// its thread without a use after free.
class Timer {
    std::shared_ptr<std::atomic<bool>> m_clear = std::make_shared<std::atomic<bool>>(false);

   public:
    ~Timer() { stop(); }

    void setTimeout(auto function, int delay) {
        stop();
        m_clear = std::make_shared<std::atomic<bool>>(false);
        std::thread t([=, clear = m_clear]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            if (*clear)
                return;
            function();
        });
        t.detach();
    }

    void setInterval(auto function, int interval) {
        stop();
        m_clear = std::make_shared<std::atomic<bool>>(false);
        std::thread t([=, clear = m_clear]() {
            while (!*clear) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if (*clear)
                    return;
                function();
            }
        });
        t.detach();
    }

    void stop() { *m_clear = true; }
};
