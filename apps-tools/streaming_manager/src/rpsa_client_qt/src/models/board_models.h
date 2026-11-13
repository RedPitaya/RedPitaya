#ifndef BOARD_MODELS_H
#define BOARD_MODELS_H

#include <QString>
#include <cstdint>

// What the client knows of a model (rp_HPeModels_t) without asking the board: its name, and the
// capabilities used until the board tells its own (CS_REQUEST_BOARD_INFO; an older server does not).
struct ModelCaps {
    QString name = "Unknown board";
    uint8_t adcChannels = 2;
    uint32_t adcRate = 125000000;
    uint8_t adcBits = 14;
    uint8_t dacChannels = 2;
    uint32_t dacRate = 125000000;
    bool adcRanges = true;  // 1:1 and 1:20
    bool adcACDC = false;
    bool dacGain = false;
    // the external trigger input has a level (250-12)
    bool extTriggerLevel = false;
    // the full scale when the inputs have one range only
    QString fixedRange = "±1 V";
    bool known = false;
};

auto modelCaps(uint8_t model) -> ModelCaps;

// "rp-f0f140" from "00:26:32:F0:F1:40"; empty when the MAC is not one
auto hostNameFromMac(const QString& mac) -> QString;

#endif
