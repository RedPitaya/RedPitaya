/**
 * What CReaderController hands the DAC from every kind of source.
 *
 * Blocks are written straight into the memory given to readBlock(); samples come out signed as
 * the DAC takes them. The files are the ones that went wrong before: a TDMS recording of the
 * board (a time column first, the buffer rows after), a file with only ch2, unsigned TDMS
 * channels, 8 bit WAV (unsigned by the format) and WAV files with chunks around the samples.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "support.h"
#include "tdms_lib/tdms.h"

using namespace reader_test;
using Reader = CReaderController;

namespace {

constexpr uint32_t kBlock = 8;  // bytes per channel and block

auto Tdms(const std::string& path) -> Reader::Ptr {
    return Reader::Create(CStreamSettings::DataFormat::TDMS, path, CStreamSettings::DACRepeat::DAC_REP_OFF, 0, kBlock);
}

auto Wav(const std::string& path, CStreamSettings::DACRepeat repeat = CStreamSettings::DACRepeat::DAC_REP_OFF, uint32_t count = 0) -> Reader::Ptr {
    return Reader::Create(CStreamSettings::DataFormat::WAV, path, repeat, count, kBlock);
}

struct Drained {
    std::vector<uint8_t> ch[2];
    Reader::BufferResult last = Reader::BR_OK;
    uint8_t bits[2] = {0, 0};
    int blocks = 0;
};

// Every block until the end, the bytes of samples of each channel without the zero fill
auto Drain(Reader& r, int maxBlocks = 32) -> Drained {
    Drained d;
    std::vector<uint8_t> b1(kBlock), b2(kBlock);
    for (int i = 0; i < maxBlocks; i++) {
        Reader::Data data;
        d.last = r.readBlock({b1.data(), b2.data()}, data);
        if (data.size[0] || data.size[1])
            d.blocks++;
        for (int ch = 0; ch < 2; ch++) {
            const auto& b = ch ? b2 : b1;
            d.ch[ch].insert(d.ch[ch].end(), b.begin(), b.begin() + static_cast<std::ptrdiff_t>(data.real_size[ch]));
            if (data.bits[ch])
                d.bits[ch] = data.bits[ch];
        }
        if (d.last != Reader::BR_OK)
            break;
    }
    return d;
}

template <typename T>
auto Values(const std::vector<uint8_t>& bytes) -> std::vector<T> {
    std::vector<T> v(bytes.size() / sizeof(T));
    std::memcpy(v.data(), bytes.data(), v.size() * sizeof(T));
    return v;
}

auto WriteBytes(const std::string& path, const std::string& bytes) -> void {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

template <typename T>
auto Le(T v) -> std::string {
    return std::string(reinterpret_cast<const char*>(&v), sizeof(T));
}

// A WAV file: `before` and `after` are extra chunks around "data"
auto WavBytes(uint16_t format, uint16_t channels, uint16_t bits, const std::string& samples, const std::string& before = "", const std::string& after = "") -> std::string {
    std::string fmt = Le<uint16_t>(format) + Le<uint16_t>(channels) + Le<uint32_t>(1000) + Le<uint32_t>(1000u * channels * bits / 8) +
                      Le<uint16_t>(static_cast<uint16_t>(channels * bits / 8)) + Le<uint16_t>(bits);
    if (format == 0xFFFE) {  // extensible: cbSize, valid bits, channel mask, subformat GUID (PCM)
        fmt += Le<uint16_t>(22) + Le<uint16_t>(bits) + Le<uint32_t>(3) + Le<uint16_t>(1) + std::string("\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71", 14);
    }
    std::string body = "WAVE" + std::string("fmt ") + Le<uint32_t>(static_cast<uint32_t>(fmt.size())) + fmt + before + "data" +
                       Le<uint32_t>(static_cast<uint32_t>(samples.size())) + samples + (samples.size() & 1 ? std::string(1, '\0') : "") + after;
    return "RIFF" + Le<uint32_t>(static_cast<uint32_t>(body.size())) + body;
}

auto Chunk(const std::string& id, const std::string& content) -> std::string {
    return id + Le<uint32_t>(static_cast<uint32_t>(content.size())) + content + (content.size() & 1 ? std::string(1, '\0') : "");
}

template <typename T>
auto Raw(std::initializer_list<T> values) -> std::string {
    std::string out;
    for (T v : values)
        out += Le<T>(v);
    return out;
}

}  // namespace

// The layout the board records: per buffer the samples, after them a row of the group Buffers,
// and with the time column on a column "time" before the samples
TEST(Sources, ABoardRecordingPlaysItsChannelsOnly) {
    TempFile file("board.tdms");
    {
        TDMS::FileWriter w;
        ASSERT_TRUE(w.open(file.path()));
        for (int seg = 0; seg < 3; seg++) {
            TDMS::SegmentBuilder s;
            const std::vector<int64_t> time{seg * 10, seg * 10 + 1, seg * 10 + 2, seg * 10 + 3};
            const std::vector<int16_t> ch1{int16_t(seg * 4), int16_t(seg * 4 + 1), int16_t(seg * 4 + 2), int16_t(seg * 4 + 3)};
            const std::vector<int16_t> ch2{-1, -2, -3, -4};
            const std::vector<uint64_t> row{4};
            s.copySamples("Group", "time", time);
            s.copySamples("Group", "ch1", ch1);
            s.copySamples("Group", "ch2", ch2);
            s.copySamples("Buffers", "samples", row);
            w.write(s);
        }
    }
    auto r = Tdms(file.path());
    ASSERT_EQ(r->isOpen(), Reader::OR_OK);
    auto d = Drain(*r);
    EXPECT_EQ(d.last, Reader::BR_ENDED);
    EXPECT_EQ(Values<int16_t>(d.ch[0]), (std::vector<int16_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}));
    EXPECT_EQ(Values<int16_t>(d.ch[1]).size(), 12u);
    EXPECT_EQ(d.bits[0], 16);
}

TEST(Sources, OnlyCh2HasTheBitsOfItsSamples) {
    TempFile file("ch2.tdms");
    {
        TDMS::FileWriter w;
        ASSERT_TRUE(w.open(file.path()));
        TDMS::SegmentBuilder s;
        const std::vector<int8_t> ch2{1, -1, 2, -2};
        s.copySamples("Group", "ch2", ch2);
        w.write(s);
    }
    auto r = Tdms(file.path());
    ASSERT_EQ(r->isOpen(), Reader::OR_OK);
    dac_channels_t ch = {};
    r->getChannels(ch);
    EXPECT_FALSE(ch[DACChannels::DAC_CH1]);
    EXPECT_TRUE(ch[DACChannels::DAC_CH2]);
    auto d = Drain(*r);
    EXPECT_EQ(d.bits[1], 8) << "8 bit samples, not taken as 16 bit";
    EXPECT_EQ(Values<int8_t>(d.ch[1]), (std::vector<int8_t>{1, -1, 2, -2}));
    EXPECT_TRUE(d.ch[0].empty());
}

TEST(Sources, UnsignedTdmsSamplesBecomeSigned) {
    TempFile file("unsigned.tdms");
    {
        TDMS::FileWriter w;
        ASSERT_TRUE(w.open(file.path()));
        TDMS::SegmentBuilder s;
        const std::vector<uint16_t> ch1{32768, 0, 65535, 32769};
        s.copySamples("Group", "ch1", ch1);
        w.write(s);
    }
    auto r = Tdms(file.path());
    ASSERT_EQ(r->isOpen(), Reader::OR_OK);
    auto d = Drain(*r);
    EXPECT_EQ(d.bits[0], 16);
    EXPECT_EQ(Values<int16_t>(d.ch[0]), (std::vector<int16_t>{0, -32768, 32767, 1})) << "the middle of the range is zero";
}

TEST(Sources, TdmsChannelsTheDacCannotPlayAreRefused) {
    TempFile file("refused.tdms");
    auto write = [&](auto build) {
        TDMS::FileWriter w;
        ASSERT_TRUE(w.open(file.path()));
        TDMS::SegmentBuilder s;
        build(s);
        w.write(s);
    };
    write([](TDMS::SegmentBuilder& s) { s.copySamples("Group", "ch1", std::vector<float>{0.5f}); });
    EXPECT_EQ(Tdms(file.path())->isOpen(), Reader::OR_WRONG_DATA_TYPE) << "volts";
    write([](TDMS::SegmentBuilder& s) {
        s.copySamples("Group", "ch1", std::vector<int8_t>{1});
        s.copySamples("Group", "ch2", std::vector<int16_t>{1});
    });
    EXPECT_EQ(Tdms(file.path())->isOpen(), Reader::OR_WRONG_DATA_TYPE) << "two sample formats";
    write([](TDMS::SegmentBuilder& s) {
        s.copySamples("Group", "ch1", std::vector<int16_t>{1, 2});
        s.copySamples("Group", "ch2", std::vector<int16_t>{1});
    });
    EXPECT_EQ(Tdms(file.path())->isOpen(), Reader::OR_DATA_NOT_EQUAL);
    write([](TDMS::SegmentBuilder& s) { s.copySamples("Group", "time", std::vector<int64_t>{1}); });
    EXPECT_EQ(Tdms(file.path())->isOpen(), Reader::OR_MISSING_CHANNELS);
}

TEST(Sources, AnyGroupWithCh1Plays) {
    TempFile file("group.tdms");
    {
        TDMS::FileWriter w;
        ASSERT_TRUE(w.open(file.path()));
        TDMS::SegmentBuilder s;
        s.copySamples("Other", "x", std::vector<int16_t>{9});
        s.copySamples("Signals", "ch1", std::vector<int16_t>{1, 2});
        w.write(s);
    }
    auto r = Tdms(file.path());
    ASSERT_EQ(r->isOpen(), Reader::OR_OK);
    EXPECT_EQ(Values<int16_t>(Drain(*r).ch[0]), (std::vector<int16_t>{1, 2}));
}

TEST(Sources, An8BitWavIsUnsignedAndBecomesSigned) {
    TempFile file("u8.wav");
    WriteBytes(file.path(), WavBytes(1, 1, 8, std::string("\x80\x81\x7F\xFF\x00", 5)));
    auto r = Wav(file.path());
    ASSERT_EQ(r->isOpen(), Reader::OR_OK);
    auto d = Drain(*r);
    EXPECT_EQ(d.bits[0], 8);
    EXPECT_EQ(Values<int8_t>(d.ch[0]), (std::vector<int8_t>{0, 1, -1, 127, -128}));
}

TEST(Sources, AStereoWavIsSplitIntoTheTwoChannels) {
    TempFile file("stereo.wav");
    WriteBytes(file.path(), WavBytes(1, 2, 16, Raw<int16_t>({1, -1, 2, -2, 3, -3, 4, -4, 5, -5})));
    auto r = Wav(file.path());
    ASSERT_EQ(r->isOpen(), Reader::OR_OK);
    size_t s1 = 0, s2 = 0;
    r->getChannelsSize(&s1, &s2);
    EXPECT_EQ(s1, 10u);
    EXPECT_EQ(s2, 10u);
    auto d = Drain(*r);
    EXPECT_EQ(Values<int16_t>(d.ch[0]), (std::vector<int16_t>{1, 2, 3, 4, 5}));
    EXPECT_EQ(Values<int16_t>(d.ch[1]), (std::vector<int16_t>{-1, -2, -3, -4, -5}));
}

TEST(Sources, ChunksAroundTheSamplesAreSkipped) {
    TempFile file("chunks.wav");
    const std::string list = Chunk("LIST", "INFOISFT" + Le<uint32_t>(3) + std::string("ab\0", 3));  // odd size, padded
    WriteBytes(file.path(), WavBytes(1, 1, 16, Raw<int16_t>({7, 8, 9}), list, Chunk("id3 ", "tag")));
    auto r = Wav(file.path());
    ASSERT_EQ(r->isOpen(), Reader::OR_OK);
    EXPECT_EQ(Values<int16_t>(Drain(*r).ch[0]), (std::vector<int16_t>{7, 8, 9})) << "neither LIST nor the trailing chunk is played";
}

TEST(Sources, AnExtensibleWavOfPcmPlays) {
    TempFile file("ext.wav");
    WriteBytes(file.path(), WavBytes(0xFFFE, 1, 16, Raw<int16_t>({4, 5})));
    auto r = Wav(file.path());
    ASSERT_EQ(r->isOpen(), Reader::OR_OK);
    EXPECT_EQ(Values<int16_t>(Drain(*r).ch[0]), (std::vector<int16_t>{4, 5}));
}

TEST(Sources, WavFilesTheDacCannotPlayAreRefused) {
    TempFile file("bad.wav");
    WriteBytes(file.path(), WavBytes(3, 1, 32, Raw<float>({0.5f})));
    EXPECT_EQ(Wav(file.path())->isOpen(), Reader::OR_WRONG_DATA_TYPE) << "float";
    WriteBytes(file.path(), WavBytes(1, 4, 16, Raw<int16_t>({1, 2, 3, 4})));
    EXPECT_EQ(Wav(file.path())->isOpen(), Reader::OR_MISSING_CHANNELS) << "four channels";
    WriteBytes(file.path(), "RIFF....WAVEnot a fmt chunk at all");
    EXPECT_EQ(Wav(file.path())->isOpen(), Reader::OR_CLOSE);
    EXPECT_EQ(Wav("/nonexistent/x.wav")->isOpen(), Reader::OR_CLOSE);
}

TEST(Sources, TheLastBlockIsFilledWithZeros) {
    TempFile file("tail.wav");
    WriteBytes(file.path(), WavBytes(1, 1, 16, Raw<int16_t>({1, 2, 3, 4, 5})));
    auto r = Wav(file.path());
    std::vector<uint8_t> block(kBlock, 0xEE);
    Reader::Data data;
    EXPECT_EQ(r->readBlock({block.data(), nullptr}, data), Reader::BR_OK);
    EXPECT_EQ(data.real_size[0], 8u);
    std::fill(block.begin(), block.end(), 0xEE);
    EXPECT_EQ(r->readBlock({block.data(), nullptr}, data), Reader::BR_ENDED);
    EXPECT_EQ(data.size[0], kBlock);
    EXPECT_EQ(data.real_size[0], 2u);
    EXPECT_EQ(Values<int16_t>(block), (std::vector<int16_t>{5, 0, 0, 0}));
}

TEST(Sources, RepeatPlaysTheFileAgainInsideABlock) {
    TempFile file("repeat.wav");
    WriteBytes(file.path(), WavBytes(1, 1, 16, Raw<int16_t>({1, 2, 3})));
    auto twice = Wav(file.path(), CStreamSettings::DACRepeat::DAC_REP_ON, 2);
    auto d = Drain(*twice);
    EXPECT_EQ(d.last, Reader::BR_ENDED);
    EXPECT_EQ(Values<int16_t>(d.ch[0]), (std::vector<int16_t>{1, 2, 3, 1, 2, 3}));

    auto endless = Wav(file.path(), CStreamSettings::DACRepeat::DAC_REP_INF);
    auto e = Drain(*endless, 3);
    EXPECT_EQ(e.last, Reader::BR_OK);
    EXPECT_EQ(Values<int16_t>(e.ch[0]), (std::vector<int16_t>{1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3}));
}

TEST(Sources, AFileWithoutSamplesIsEmpty) {
    TempFile file("empty.wav");
    WriteBytes(file.path(), WavBytes(1, 1, 16, ""));
    auto r = Wav(file.path(), CStreamSettings::DACRepeat::DAC_REP_INF);
    ASSERT_EQ(r->isOpen(), Reader::OR_OK);
    std::vector<uint8_t> block(kBlock);
    Reader::Data data;
    EXPECT_EQ(r->readBlock({block.data(), nullptr}, data), Reader::BR_EMPTY) << "and no endless repeat";
}

TEST(Sources, MemoryIsCopiedIntoTheBlocks) {
    std::vector<int16_t> samples{1, 2, 3, 4, 5, 6};
    auto* in = new Reader::DataIn();
    in->ch[0] = reinterpret_cast<uint8_t*>(samples.data());
    in->size[0] = samples.size() * 2;
    in->bits = 16;
    Reader r(in, CStreamSettings::DACRepeat::DAC_REP_OFF, 0, kBlock);
    ASSERT_EQ(r.isOpen(), Reader::OR_OK);
    auto d = Drain(r);
    EXPECT_EQ(Values<int16_t>(d.ch[0]), samples);
    EXPECT_EQ(d.last, Reader::BR_ENDED);
}

TEST(Sources, ACallbackFillsTheBlocksItself) {
    auto* sink = new Reader::MemoryStreamSink();
    sink->memoryStreamBits = 16;
    sink->channels = dac_channels_t();
    sink->channels[DACChannels::DAC_CH1] = true;
    std::vector<uint8_t*> seen;
    int calls = 0;
    sink->callback = [&](uint8_t bits, std::array<uint8_t*, MAX_DAC_CHANNELS>& buffers, const uint32_t size) {
        seen.push_back(buffers[0]);
        EXPECT_EQ(bits, 16);
        EXPECT_EQ(size, kBlock);
        EXPECT_EQ(buffers[1], nullptr) << "no channel 2";
        std::memset(buffers[0], 0x11, size);
        return ++calls == 2;
    };
    Reader r(sink, kBlock);
    std::vector<uint8_t> block(kBlock), block2(kBlock);
    Reader::Data data;
    EXPECT_EQ(r.readBlock({block.data(), block2.data()}, data), Reader::BR_OK);
    EXPECT_EQ(data.size[0], kBlock);
    EXPECT_EQ(r.readBlock({block.data(), block2.data()}, data), Reader::BR_ENDED);
    ASSERT_EQ(seen.size(), 2u);
    EXPECT_EQ(seen[0], block.data()) << "the callback gets the memory of the block, no copy";
    EXPECT_EQ(block[0], 0x11);
}

// readBlock writes into the given memory and not a byte beyond the block
TEST(Sources, TheBlockIsWrittenInPlaceAndNoFurther) {
    TempFile file("place.wav");
    WriteBytes(file.path(), WavBytes(1, 2, 16, Raw<int16_t>({1, -1, 2, -2, 3, -3, 4, -4, 5, -5})));
    auto r = Wav(file.path());
    std::vector<uint8_t> a(kBlock + 4, 0xEE), b(kBlock + 4, 0xEE);
    Reader::Data data;
    for (int i = 0; i < 2; i++) {
        r->readBlock({a.data(), b.data()}, data);
        for (size_t k = kBlock; k < a.size(); k++) {
            EXPECT_EQ(a[k], 0xEE) << "block " << i;
            EXPECT_EQ(b[k], 0xEE) << "block " << i;
        }
    }
}
