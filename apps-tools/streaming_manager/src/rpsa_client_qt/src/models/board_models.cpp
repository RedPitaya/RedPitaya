#include "board_models.h"
#include <QRegularExpression>

namespace {

// rp_HPeModels_t
const char* const NAMES[] = {
    "STEMlab 125-10 v1.0",           // 0
    "STEMlab 125-14 v1.0",           // 1
    "STEMlab 125-14 v1.1",           // 2
    "SDRlab 122-16 v1.0",            // 3
    "SDRlab 122-16 v1.1",            // 4
    "STEMlab 125-14 LN v1.1",        // 5
    "STEMlab 125-14 Z7020 v1.0",     // 6
    "STEMlab 125-14 LN-Z7020 v1.1",  // 7
    "STEMlab 125-14 4-Input v1.0",   // 8
    "STEMlab 125-14 4-Input v1.2",   // 9
    "STEMlab 125-14 4-Input v1.3",   // 10
    "SIGNALlab 250-12 v1.0",         // 11
    "SIGNALlab 250-12 v1.1",         // 12
    "SIGNALlab 250-12 v1.2",         // 13
    "SIGNALlab 250-12/120",          // 14
    "SIGNALlab 250-12 v1.2a",        // 15
    "SIGNALlab 250-12 v1.2b",        // 16
    "STEMlab 125-14 LN BO v1.1",     // 17
    "STEMlab 125-14 LN CE1 v1.1",    // 18
    "STEMlab 125-14 LN CE2 v1.1",    // 19
    "STEMlab 125-14 v2.0",           // 20
    "STEMlab 125-14 Pro v2.0",       // 21
    "STEMlab 125-14-Z7020 Pro v2.0", // 22
    "STEMlab 125-14 Ind v2.0",       // 23
    "STEMlab 125-14-Z7020 Pro v1.0", // 24
    "STEMlab 125-14 LL v1.1",        // 25
    "STEMlab 65-16 LL v1.1",         // 26
    "STEMlab 125-14 LL v1.2",        // 27
    "STEMlab 125-14 TI v1.3",        // 28
    "STEMlab 65-16 TI v1.3",         // 29
    "STEMlab 125-14 4-Input BO v1.3",  // 30
    "STEMlab 125-14 BO v2.0",          // 31
    "STEMlab 125-14 Pro BO v2.0",      // 32
    "STEMlab 125-14-Z7020 Pro BO v2.0" // 33
};

}  // namespace

auto modelCaps(uint8_t model) -> ModelCaps {
    ModelCaps c;
    if (model >= sizeof(NAMES) / sizeof(NAMES[0]))
        return c;
    c.known = true;
    c.name = NAMES[model];
    switch (model) {
        case 0:
            c.adcBits = 10;
            break;
        case 3:
        case 4:
            c.adcRate = 122880000;
            c.dacRate = 122880000;
            c.adcBits = 16;
            c.adcRanges = false;
            c.fixedRange = "±0.25 V";
            break;
        case 8:
        case 9:
        case 10:
        case 30:
            c.adcChannels = 4;
            c.dacChannels = 0;
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            c.adcRate = 250000000;
            c.dacRate = 250000000;
            c.adcBits = 12;
            c.adcACDC = true;
            c.dacGain = true;
            c.extTriggerLevel = true;
            break;
        case 26:
        case 29:
            c.adcRate = 62500000;
            c.adcBits = 16;
            break;
        default:
            break;
    }
    return c;
}

auto hostNameFromMac(const QString& mac) -> QString {
    static const QRegularExpression re("^([0-9A-Fa-f]{2}[:-]){5}[0-9A-Fa-f]{2}$");
    if (!re.match(mac).hasMatch())
        return "";
    QString hex = mac;
    hex.remove(':').remove('-');
    return "rp-" + hex.right(6).toLower();
}
