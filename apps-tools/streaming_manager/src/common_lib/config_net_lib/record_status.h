#ifndef CONFIG_NET_LIB_RECORD_STATUS_H
#define CONFIG_NET_LIB_RECORD_STATUS_H

#include <cstdint>
#include <string>

// A recording of one stream on the board (the ADC, the GPIO input), of its current or last run:
// the counters are reset by its start and kept after its stop.
struct StreamRecordStatus {
    enum Mode : uint8_t { NONE = 0, NETWORK = 1, SD = 2 };
    uint8_t mode = NONE;     // where the run sends its data
    bool running = false;
    uint64_t samples = 0;    // per channel: in the file (SD), given by the FPGA (network)
    uint64_t limit = 0;      // the sample limit of the file, 0 - none
    uint64_t written = 0;    // bytes in the file
    double writeRate = 0;    // bytes a second the SD card takes, 0 - not measured yet
    double queue = 0;        // the fill of the write queue in memory, 0..1
    uint64_t lostBoard = 0;  // samples per channel lost on the board (no free buffer)
    uint64_t lostQueue = 0;  // samples per channel the file did not take (its queue was full)
    std::string file;        // the path on the board

    auto operator==(const StreamRecordStatus&) const -> bool = default;
};

// The recordings of a board, sent with CS_RESPONSE_RECORD_STATUS: what the SD card of the board
// takes and how far the files are, for the clients that do not get the data themselves.
struct RecordStatus {
    StreamRecordStatus adc;
    StreamRecordStatus gpioIn;
    uint64_t sdFree = 0;   // bytes free on the card of the files
    uint64_t sdTotal = 0;  // its size

    auto toJson() const -> std::string;
    // False on a document that is not a status (including an empty one); _status is then
    // unchanged. Absent keys keep their defaults: a newer server may add fields.
    static auto fromJson(const std::string& _json, RecordStatus& _status) -> bool;

    auto operator==(const RecordStatus&) const -> bool = default;
};

#endif
