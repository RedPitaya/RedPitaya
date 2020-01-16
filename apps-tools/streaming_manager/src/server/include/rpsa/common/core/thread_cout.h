//
// Created by user on 09.10.18.
//
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

#ifndef PROJECT_PRINTTHREAD_H
#define PROJECT_PRINTTHREAD_H


static std::mutex mtx_cout;

// Asynchronous output
struct acout
{
    std::unique_lock<std::mutex> lk;
    acout()
            :
            lk(std::unique_lock<std::mutex>(mtx_cout))
    {

    }

    template<typename T>
    acout& operator<<(const T& _t)
    {
        std::cout << _t;
        return *this;
    }

    acout& operator<<(std::ostream& (*fp)(std::ostream&))
    {
        std::cout << fp;
        return *this;
    }
};

class StringBuilder {
private:
    std::string main;
    std::string scratch;

    const std::string::size_type ScratchSize = 1024;  // or some other arbitrary number

public:
    StringBuilder & append(const std::string & str) {
        scratch.append(str);
        if (scratch.size() > ScratchSize) {
            main.append(scratch);
            scratch.resize(0);
        }
        return *this;
    }

    const std::string & str() {
        if (scratch.size() > 0) {
            main.append(scratch);
            scratch.resize(0);
        }
        return main;
    }
};

#endif