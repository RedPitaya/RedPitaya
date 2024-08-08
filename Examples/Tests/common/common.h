#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <cstring>
#include <sstream>
#include <iostream>

#include "rp.h"
#include "rp_hw-profiles.h"
#include "rp_hw-calib.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef enum {
    RP_125_14,
    RP_250_12,
    RP_125_14_4CH,
    RP_122_16
} models_t;

using namespace std;

namespace Color {
    enum Code {
        FG_RED      = 31,
        FG_GREEN    = 32,
        FG_BLUE     = 34,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_BLUE     = 44,
        BG_DEFAULT  = 49
    };
    class Modifier {
        Code code;
    public:
        Modifier(Code pCode) : code(pCode) {}
        friend std::ostream&

        operator<<(std::ostream& os, const Modifier& mod) {
            return os << "\033[" << mod.code << "m";
        }

        operator std::string() const {
            std::ostringstream out;
            out << *this;
            return out.str();
        }

        auto color(string &str) -> string{
            Modifier def(FG_DEFAULT);
            std::ostringstream out;
            out << *this << str << def;
            return out.str();
        }

        auto color(const char *str) -> string{
            Modifier def(FG_DEFAULT);
            std::ostringstream out;
            out << *this << str << def;
            return out.str();
        }

    };
}

auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getADCRate() -> uint32_t;
auto getClock() -> double;
auto getModel() -> models_t;
auto getTrigName(rp_acq_trig_src_t tr) -> std::string;
auto printTestResult(list<string>  &_result, string _testName,bool result) -> void;
auto printAllResult(list<string>  &_result) -> void;
auto printBuffer(buffers_t *data,int indexOffset,int size) -> void;
auto printBufferF(buffers_t *data,int indexOffset,int size) -> void;
auto getTrigNameDir(rp_acq_trig_src_t tr) -> float;
auto getDACGainCh1() -> float;