#ifndef CONFIG_NET_LIB_BOARD_INFO_H
#define CONFIG_NET_LIB_BOARD_INFO_H

#include <cstdint>
#include <string>

// What a board is and what its streaming application can do, sent with CS_RESPONSE_BOARD_INFO.
// A client that found the board by its broadcast knows only the model; one that was given the
// address of the board (another subnet, no broadcast) knows nothing else.
struct BoardInfoMessage {
    uint8_t model = 0;  // rp_HPeModels_t, as the broadcast carries it
    std::string mac;
    bool master = true;  // false: a daisy chain slave
    uint8_t adcChannels = 2;
    uint8_t dacChannels = 2;
    uint32_t adcRate = 125000000;  // Hz, before the decimation
    uint32_t dacRate = 125000000;  // Hz
    uint8_t adcBits = 14;
    bool adcRanges = false;  // 1:1 and 1:20 inputs
    bool adcACDC = false;
    bool dacGain = false;  // x1/x5
    // the blocks of the FPGA image
    bool adc = true;
    bool adcBlockMode = false;
    bool dac = true;
    bool dacTriggerOutputs = false;
    bool gpio = false;
    bool extTrigger = false;
    // the DMA memory the streams share and the largest block (with its header), bytes; 0 unknown
    uint32_t memorySize = 0;
    uint32_t memoryMaxBlock = 0;

    auto toJson() const -> std::string;
    // False on a document that is not a board info (including an empty one); _info is then unchanged.
    static auto fromJson(const std::string& _json, BoardInfoMessage& _info) -> bool;

    auto operator==(const BoardInfoMessage&) const -> bool = default;
};

#endif
