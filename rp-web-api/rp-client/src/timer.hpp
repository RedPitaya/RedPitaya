/**
 * $Id$
 *
 * @brief Red Pitaya Web module
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#pragma once
#include <chrono>

class Timer {
    bool clear = false;

public:

    ~Timer() {stop();}

    void setTimeout(auto function, int delay) {
        this->clear = false;
        std::thread t([=,this]() {
            if(this->clear) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            if(this->clear) return;
            function();
        });
        t.detach();
    }

    void setInterval(auto function, int interval) {
        this->clear = false;
        std::thread t([=,this]() {
            while(true) {
                if(this->clear) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if(this->clear) return;
                function();
            }
        });
        t.detach();
    }

    void stop() {
        this->clear = true;
    }
};