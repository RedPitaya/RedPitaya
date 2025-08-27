/**
 * $Id: $
 *
 * @brief Red Pitaya library logic analizer api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_LA_H
#define __RP_LA_H

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

namespace rp_la {

struct OutputPacket {
    /* Line name according to the protocol for which the data was decoded.
       For example: RX, TX, MOSI, MISO, SDA.
       These lines must be specified in the decoder settings. Otherwise, they will not be here. */
    std::string line_name;

    /*
    Data identifier.
    This identifier can be used to get the annotation in the function:
        - getAnnotation
        - getAnnotationList
    */
    uint8_t control;

    /*
        Data contained in the protocol:
        - ID
        - DATA
        - CRC
        ...
        etc. */
    uint32_t data;

    // Number of recognized data bits. Multiple of 0.5.
    // 0, 0.5, 1, 1.5, 2, etc.
    float bitsInPack;

    /*
    Starting position in the data to be recognized in samples.
    0 = first sample
    1 = second sample
    etc
    The value is not an integer, since the bit width can be real depending on the protocol. */
    double sampleStart;

    // Length of the recognized block in samples
    double length;
};

typedef enum {
    // Enables the mode without an external logic analyzer block
    LA_BASIC = 0,
    // Must be installed when using an external unit.
    LA_PRO = 1
} la_Mode_t;

typedef enum { LA_DECODER_NONE = 0, LA_DECODER_CAN = 1, LA_DECODER_I2C = 2, LA_DECODER_SPI = 3, LA_DECODER_UART = 4 } la_Decoder_t;

/*
    Trigger settings for data capture.
    Settings: LA_RISING, LA_FALLING, LA_RISING_OR_FALLING cannot be used simultaneously on different lines.
    This setting can only be used on one line.
    Settings: LA_LOW, LA_HIGH can be used together with the settings for changing the signal edge.
    For example: In SPI, you can set line 1 (CS) to LA_LOW, and line 2 (CLK) to LA_RISING_OR_FALLING
*/
typedef enum { LA_ERROR = -1, LA_NONE = 0, LA_LOW = 1, LA_HIGH = 2, LA_RISING = 3, LA_FALLING = 4, LA_RISING_OR_FALLING = 5 } la_Trigger_Mode_t;

/**
 * Type representing digital input output pins.
 */
typedef enum {
    LA_T_CHANNEL_1 = 0,  //!< DIO0_P
    LA_T_CHANNEL_2 = 1,  //!< DIO1_P
    LA_T_CHANNEL_3 = 2,  //!< DIO2_P
    LA_T_CHANNEL_4 = 3,  //!< DIO3_P
    LA_T_CHANNEL_5 = 4,  //!< DIO4_P
    LA_T_CHANNEL_6 = 5,  //!< DIO5_P
    LA_T_CHANNEL_7 = 6,  //!< DIO6_P
    LA_T_CHANNEL_8 = 7,  //!< DIO7_P
} la_Trigger_Channel_t;

class CLAController;

class CLACallback {
   public:
    virtual ~CLACallback() {}
    // A callback function that is called after data is captured on the FPGA.
    // Called after the function: runAsync(uint32_t timeoutMs)
    virtual void captureStatus(CLAController* controller, bool isTimeout, uint32_t numBytes, uint64_t numSamples, uint64_t preTriggerSamples,
                               uint64_t postTriggerSamples) {}

    // A callback function that is called after a block of data uploaded by the user has been decoded.
    // Called after the function: loadFromFileAndDecode(std::string file, bool isRLE, uint64_t triggerSamplePosition)
    virtual void decodeStatus(CLAController* controller, uint32_t numBytes, uint64_t numSamples, uint64_t preTriggerSamples, uint64_t postTriggerSamples) {}

    // A callback function that is called after each of the configured decoders has been decoded.
    virtual void decodeDone(CLAController* controller, std::string name) {}
};

class CLAController {

   public:
    CLAController();
    ~CLAController();

    /*
        Initializes the FPGA registers and the reserved memory area.
    */
    auto initFpga() -> bool;

    /*
        Sets the operating mode with or without an external unit.
        Affects only data capture from the FPGA.
    */
    auto setMode(la_Mode_t mode) -> void;

    // Sets the trigger for the FPGA. Valid channels are 0 to 7
    auto setTrigger(la_Trigger_Channel_t channel, la_Trigger_Mode_t mode) -> void;

    // Returns the trigger setting for the specified channel. Valid channels are 0 to 7
    auto getTrigger(la_Trigger_Channel_t channel) -> la_Trigger_Mode_t;

    // Returns True if triggers on all channels are set to LA_NONE mode
    auto isNoTriggers() -> bool;

    // Sets triggers on all channels to LA_NONE mode
    auto resetTriggers() -> void;

    /*
        Sends a trigger signal to the FPGA.
        The command makes sense only during data capture on the FPGA and requires the user to stop waiting for the trigger.
    */
    auto softwareTrigger() -> bool;

    /*
        Sets the data recording mode to FPGA in RLE format, otherwise the data will be recorded in DMA without compression.
        Decoders work only with compressed data in RLE format
        RLE: data[i] -> Sample count + 1, data[i+1] -> Data from GPIO (8 bit = 8 lines)
        NoRLE: data[i] -> Sample #i
    */
    auto setEnableRLE(bool enable) -> bool;

    // Returns whether RLE mode is enabled
    auto isRLEEnable() -> bool;

    // Adds a decoder by the specified name.
    auto addDecoder(std::string name, la_Decoder_t decoder) -> bool;

    // Returns a list of decoder names that the user has added.
    auto getDecoders() -> std::vector<std::string>;

    // Returns the decoded data for the specified decoder.
    auto getDecodedData(std::string name) -> std::vector<rp_la::OutputPacket>;

    // Enables or disables the decoder.
    // After capturing data on the FPGA or after loading data from an array, if the decoder is disabled, it will not be used to decode data.
    auto setDecoderEnable(std::string name, bool enable) -> void;

    // Returns the decoder status. Enabled or disabled
    auto getDecoderEnable(std::string name) -> bool;

    // Returns the decoder type for the specified name.
    auto getDecoderType(std::string name) -> la_Decoder_t;

    // Checks if the specified decoder is installed
    auto isDecoderExist(std::string name) -> bool;

    // Removes the specified decoder.
    auto removeDecoder(std::string name) -> bool;

    // Removes all decoders
    auto removeAllDecoders() -> void;

    // Sets the settings in JSON format for the specified decoder.
    auto setDecoderSettings(std::string name, std::string json) -> bool;

    // Returns the settings in JSON format for the specified decoder.
    auto getDecoderSettings(std::string name) -> std::string;

    // Returns default settings for the specified decoder.
    // Depends on the type of decoder created. Each decoder has its own set of keys in the settings
    auto getDefaultSettings(la_Decoder_t decoder) -> std::string;

    // Sets the settings for the decoder by the specified key.
    // Keys can be taken from the JSON settings, if the key is not found the function returns false
    auto setDecoderSettingsUInt(std::string name, std::string key, uint32_t value) -> bool;
    auto setDecoderSettingsFloat(std::string name, std::string key, float value) -> bool;

    // Returns the value for the specified key, if the key is not found the function returns false
    auto getDecoderSettingsUInt(std::string name, std::string key, uint32_t* value) -> bool;
    auto getDecoderSettingsFloat(std::string name, std::string key, float* value) -> bool;

    // Sets decimation when capturing data via FPGA
    auto setDecimation(uint32_t decimation) -> void;
    auto getDecimation() -> uint32_t;

    // Sets the number of samples the FPGA must write before triggering.
    auto setPreTriggerSamples(uint32_t value) -> void;
    auto getPreTriggerSamples() -> uint32_t;

    // Sets the amount of data after the trigger that the FPGA will record.
    auto setPostTriggerSamples(uint32_t value) -> void;
    auto getPostTriggerSamples() -> uint32_t;

    /* Returns the size of reserved memory in DMA in bytes
       Without compression, 1 FPGA sample is equal to 1 byte
       The amount of data that can be written when compressed in RLE may depend on the data itself.
       For example, with 1MB memory size, we can write 512k samples in the worst case, 131M samples in the best case.
       This must be taken into account when setting the number of samples before and after the trigger, since the buffer is cyclic and data can overlap each other.
    */
    auto getDMAMemorySize() -> uint32_t;

    // Returns the number of bytes captured by the FPGA or loaded from the user data array.
    auto getCapturedDataSize() -> uint32_t;

    /*
        Returns the number of samples captured by the FPGA or loaded from the user data array.
        Also calculates from RLE.
    */
    auto getCapturedSamples() -> uint64_t;

    // Sets an object with callback functions.
    auto setDelegate(CLACallback* callbacks) -> void;
    auto removeDelegate() -> void;

    // Shows whether data capture is running or not.
    auto isCaptureRun() -> bool;

    // Starts asynchronously capturing data from the FPGA, and also decodes this data.
    auto runAsync(uint32_t timeoutMs) -> void;

    // Asynchronously decodes user data
    auto runAsync(std::vector<uint8_t> data, bool isRLE, uint64_t triggerSamplePosition) -> void;

    // Asynchronously decodes data that was captured via the FPGA or loaded using the function: loadFromFile
    auto decodeAsync() -> void;

    // Waits for an asynchronous function to complete or times out
    // All asynchronous functions cannot work
    auto wait(uint32_t timeoutMs, bool* isTimeout) -> void;

    // Synchronous decoding of data via the specified decoder.
    // A copy of the decoder is created and the decoder settings are used, the data in the decoder itself is not changed.
    // Callback functions are not called
    auto decode(std::string name) -> std::vector<rp_la::OutputPacket>;

    // Decodes data with the specified decoder and settings from a numpy-enabled data buffer
    // Decoding is synchronous and no callback functions are called.
    auto decodeNP(la_Decoder_t decoder, std::string json_settings, uint8_t* np_buffer, int size) -> std::vector<rp_la::OutputPacket>;

    // Saves data from the buffer to a file.
    auto saveCaptureDataToFile(std::string file) -> bool;

    // Loads data into the analyzer buffer from a file, without decoding.
    auto loadFromFile(std::string file, bool isRLE, uint64_t triggerSamplePosition) -> bool;

    // Loads data into the analyzer buffer from a file, with decoding.
    // Decoding and loading are asynchronous
    auto loadFromFileAndDecode(std::string file, bool isRLE, uint64_t triggerSamplePosition) -> bool;

    // Returns data from a buffer with the ability to use numpy
    auto getDataNP(uint8_t* np_buffer, int size) -> uint32_t;

    // Returns unpacked RLE data from a buffer with the ability to use numpy
    // Otherwise nothing will be returned
    auto getUnpackedRLEDataNP(uint8_t* np_buffer, int size) -> uint64_t;

    auto printRLE(bool useHex) -> void;
    auto printRLENP(uint8_t* np_buffer, int size, bool useHex) -> void;

    auto getAnnotationList(la_Decoder_t decoder) -> std::map<uint8_t, std::string>;
    auto getAnnotation(la_Decoder_t decoder, uint8_t control) -> std::string;

   private:
    CLAController(const CLAController&) = delete;
    CLAController(CLAController&&) = delete;
    CLAController& operator=(const CLAController&) = delete;
    CLAController& operator=(const CLAController&&) = delete;

    struct Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
};

}  // namespace rp_la

#endif  // __RP_LA_H
