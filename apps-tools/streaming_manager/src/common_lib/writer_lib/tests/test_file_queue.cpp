/**
 * The way to the file: FileChunk (blocks written from where they are), the queue with its memory
 * limit, and the file thread of FileQueueManager, which waits for chunks instead of spinning.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <limits>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "writer_lib/file_queue_manager.h"
#include "writer_lib/time_limit_clock.h"

namespace {

auto TempPath(const std::string& name) -> std::string {
    auto dir = std::filesystem::temp_directory_path() / ("writer_test_" + std::to_string(::getpid()));
    std::filesystem::create_directories(dir);
    return (dir / name).string();
}

auto ReadFile(const std::string& path) -> std::string {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

auto Chunk(const std::string& text) -> FileChunk::Ptr {
    auto c = std::make_unique<FileChunk>();
    c->add(std::vector<uint8_t>(text.begin(), text.end()));
    return c;
}

// The CPU time of the whole process, which the file thread is part of
auto CpuTime() -> std::chrono::milliseconds {
    return std::chrono::milliseconds(std::clock() * 1000 / CLOCKS_PER_SEC);
}

class TestQueue : public Queue {
   public:
    using Queue::popQueue;
    using Queue::pushQueue;
};

}  // namespace

TEST(FileChunk, BlocksAreWrittenFromWhereTheyAre) {
    auto owner = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>{'a', 'b', 'c'});
    FileChunk c;
    c.add(owner, owner->data(), 2);
    std::vector<uint8_t> own{'x', 'y'};
    const uint8_t* ownData = own.data();
    c.add(std::move(own));
    c.add(owner, owner->data(), 0);  // nothing

    ASSERT_EQ(c.blocks().size(), 2u);
    EXPECT_EQ(c.blocks()[0].data, owner->data()) << "no copy of someone else's memory";
    EXPECT_EQ(c.blocks()[1].data, ownData) << "the vector moved in, not copied";
    EXPECT_EQ(c.size(), 4u);
    EXPECT_EQ(owner.use_count(), 2) << "the chunk keeps the owner alive";

    (*owner)[1] = 'B';  // written as it is at write time
    std::ostringstream out;
    EXPECT_TRUE(c.writeTo(out));
    EXPECT_EQ(out.str(), "aBxy");
}

TEST(FileQueue, TheQueueTakesChunksUpToItsMemoryLimit) {
    TestQueue q;
    EXPECT_TRUE(q.pushQueue(Chunk("1234"), 10));
    EXPECT_TRUE(q.pushQueue(Chunk("5678"), 10));
    EXPECT_TRUE(q.pushQueue(Chunk("9"), 10)) << "8 bytes held, under the limit";
    EXPECT_FALSE(q.pushQueue(Chunk("0"), 9)) << "9 bytes held";
    EXPECT_EQ(q.queueMemory(), 9u);
    EXPECT_EQ(q.queueSize(), 3);
    std::ostringstream out;
    while (auto c = q.popQueue())
        c->writeTo(out);
    EXPECT_EQ(out.str(), "123456789") << "in order";
    EXPECT_EQ(q.queueMemory(), 0u);
}

TEST(FileQueue, PopWaitsForAChunkOrTheTimeout) {
    TestQueue q;
    auto start = std::chrono::steady_clock::now();
    EXPECT_EQ(q.popQueue(std::chrono::milliseconds(50)), nullptr);
    EXPECT_GE(std::chrono::steady_clock::now() - start, std::chrono::milliseconds(45));

    std::thread pusher([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        q.pushQueue(Chunk("x"), 100);
    });
    start = std::chrono::steady_clock::now();
    auto c = q.popQueue(std::chrono::seconds(5));
    pusher.join();
    ASSERT_NE(c, nullptr);
    EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(1)) << "woken by the push";
}

TEST(FileQueueManager, ChunksAreWrittenInOrderAndAllOfThemOnAFlushingStop) {
    const auto path = TempPath("order.bin");
    FileQueueManager m;
    m.openFile(path, false);
    m.startWrite(CStreamSettings::DataFormat::BIN);
    std::string expected;
    for (int i = 0; i < 200; i++) {
        const std::string text = "chunk " + std::to_string(i) + ";";
        expected += text;
        ASSERT_TRUE(m.addChunkToWrite(Chunk(text)));
    }
    m.stopWrite(true);
    m.closeFile();
    EXPECT_EQ(ReadFile(path), expected);
    EXPECT_EQ(m.getWritedSize(), expected.size());
    EXPECT_FALSE(m.addChunkToWrite(Chunk("late"))) << "the thread has stopped";
}

// The write-back runs in windows of 8 MB (limitDirtyPages): a file of several windows, twice in
// one manager, comes out whole.
TEST(FileQueueManager, AFileOfSeveralWriteBackWindowsIsWrittenWhole) {
    FileQueueManager m;
    for (int run = 0; run < 2; run++) {
        const auto path = TempPath("windows" + std::to_string(run) + ".bin");
        m.openFile(path, false);
        m.startWrite(CStreamSettings::DataFormat::BIN);
        std::string expected;
        for (int i = 0; i < 37; i++) {  // 37 x 0.6 MB: the last window is a partial one
            std::string text(600 * 1024, static_cast<char>('a' + (i + run) % 26));
            expected += text;
            while (!m.addChunkToWrite(Chunk(text)))
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        m.stopWrite(true);
        m.closeFile();
        EXPECT_EQ(ReadFile(path).size(), expected.size());
        EXPECT_TRUE(ReadFile(path) == expected) << "run " << run;
        std::filesystem::remove(path);
    }
}

// Before the speed of the file is known the queue takes 32 MB at most; once measured it takes
// 20 s of writing, from 8 MB up to a quarter of the RAM.
TEST(FileQueueManager, TheQueueLimitFollowsTheMeasuredSpeed) {
    const auto path = TempPath("limit.bin");
    FileQueueManager m;
    m.openFile(path, false);
    EXPECT_GE(m.queueLimit(), 8u * 1024 * 1024);
    EXPECT_LE(m.queueLimit(), 32u * 1024 * 1024);
    EXPECT_EQ(m.writeRate(), 0);
    m.startWrite(CStreamSettings::DataFormat::BIN);
    // a speed sample takes 32 MB (or a second of writing)
    const std::string text(1024 * 1024, 'q');
    const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(20);
    while (m.writeRate() == 0 && std::chrono::steady_clock::now() < end) {
        m.addChunkToWrite(Chunk(text));
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    m.stopWrite(false);
    m.closeFile();
    std::filesystem::remove(path);
    ASSERT_GT(m.writeRate(), 0) << "no speed measured in 20 s";
    const double want = m.writeRate() * 20;
    const uint64_t limit = m.queueLimit();
    EXPECT_GE(limit, 8u * 1024 * 1024);
    if (want > 8.0 * 1024 * 1024 && limit > 8u * 1024 * 1024)
        EXPECT_LE(static_cast<double>(limit), want * 1.001) << "the queue takes no more than 20 s of writing";
}

TEST(FileQueueManager, TheFileThreadSleepsWhileThereIsNothingToWrite) {
    const auto path = TempPath("idle.bin");
    FileQueueManager m;
    m.openFile(path, false);
    m.startWrite(CStreamSettings::DataFormat::BIN);
    const auto cpu = CpuTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    const auto used = CpuTime() - cpu;
    m.stopWrite(false);
    m.closeFile();
    EXPECT_LT(used, std::chrono::milliseconds(50)) << "the idle file thread took " << used.count() << " ms of CPU in 500 ms";
}

TEST(FileQueueManager, AStopWithoutFlushDropsWhatIsQueued) {
    const auto path = TempPath("drop.bin");
    FileQueueManager m;
    m.openFile(path, false);
    m.startWrite(CStreamSettings::DataFormat::BIN);
    m.stopWrite(false);
    m.closeFile();
    EXPECT_EQ(m.queueSize(), 0);
}

// ---- the parts of a recording: a switch to the next file between two chunks

namespace {

// a WAV header of 44 bytes whose two sizes (RIFF at 4, data at 40) are `riff` and `data`
auto WavHeader(uint32_t riff, uint32_t data) -> std::string {
    std::string h(44, '\0');
    std::memcpy(&h[0], "RIFF", 4);
    std::memcpy(&h[4], &riff, 4);
    std::memcpy(&h[8], "WAVE", 4);
    std::memcpy(&h[36], "data", 4);
    std::memcpy(&h[40], &data, 4);
    return h;
}

auto U32At(const std::string& bytes, size_t at) -> uint32_t {
    uint32_t v = 0;
    std::memcpy(&v, bytes.data() + at, 4);
    return v;
}

}  // namespace

TEST(FileQueueManager, ASwitchClosesTheFileBetweenTwoChunksAndGoesOnInTheNext) {
    const auto a = TempPath("part_0001.bin"), b = TempPath("part_0002.bin"), c = TempPath("part_0003.bin");
    FileQueueManager m;
    m.openFile(a, false);
    m.startWrite(CStreamSettings::DataFormat::BIN);
    EXPECT_EQ(m.filesWritten(), 1);
    std::string first, second, third;
    for (int i = 0; i < 50; i++) {
        const std::string t = "a" + std::to_string(i) + ";";
        first += t;
        ASSERT_TRUE(m.addChunkToWrite(Chunk(t)));
    }
    ASSERT_TRUE(m.addFileSwitch(b));
    for (int i = 0; i < 30; i++) {
        const std::string t = "b" + std::to_string(i) + ";";
        second += t;
        ASSERT_TRUE(m.addChunkToWrite(Chunk(t)));
    }
    ASSERT_TRUE(m.addFileSwitch(c));
    third = "c;";
    ASSERT_TRUE(m.addChunkToWrite(Chunk(third)));
    m.stopWrite(true);
    m.closeFile();
    EXPECT_EQ(ReadFile(a), first);
    EXPECT_EQ(ReadFile(b), second);
    EXPECT_EQ(ReadFile(c), third);
    EXPECT_EQ(m.filesWritten(), 3);
    EXPECT_EQ(m.fileName(), c);
    EXPECT_EQ(m.getWritedSize(), first.size() + second.size() + third.size()) << "all the files of the run";
    EXPECT_EQ(m.fileSize(), third.size()) << "the file written now";
}

TEST(FileQueueManager, ASwitchIsQueuedEvenWhenTheQueueIsFull) {
    const auto a = TempPath("full_0001.bin"), b = TempPath("full_0002.bin");
    FileQueueManager m;
    m.openFile(a, false);
    m.startWrite(CStreamSettings::DataFormat::BIN);
    // more than the queue takes before the speed is known (32 MB): some chunks may be refused,
    // the switch never
    const std::string big(4 * 1024 * 1024, 'x');
    uint64_t taken = 0;
    for (int i = 0; i < 20; i++)
        if (m.addChunkToWrite(Chunk(big)))
            taken += big.size();
    ASSERT_TRUE(m.addFileSwitch(b));
    const bool after = m.addChunkToWrite(Chunk("after"));
    m.stopWrite(true);
    m.closeFile();
    EXPECT_EQ(std::filesystem::file_size(a), taken) << "what was taken before the switch, in the first file";
    EXPECT_EQ(ReadFile(b), after ? "after" : "");
    std::filesystem::remove(a);
    std::filesystem::remove(b);
}

TEST(FileQueue, ASwitchMarkerHasNoSizeAndPassesAFullQueue) {
    TestQueue q;
    ASSERT_TRUE(q.pushQueue(Chunk("1234567890"), 10));
    EXPECT_FALSE(q.pushQueue(Chunk("x"), 10)) << "full";
    auto marker = FileChunk::nextFile("next.bin");
    EXPECT_EQ(marker->size(), 0u);
    EXPECT_EQ(marker->nextFileName(), "next.bin");
    EXPECT_TRUE(q.pushQueue(std::move(marker), std::numeric_limits<uint64_t>::max()));
    EXPECT_EQ(q.queueMemory(), 10u);
    q.popQueue();
    auto popped = q.popQueue();
    ASSERT_NE(popped, nullptr);
    EXPECT_EQ(popped->nextFileName(), "next.bin") << "in its place in the order";
}

TEST(FileQueueManager, ANextFileThatCannotBeMadeStopsTheRecordingAsAFullDisk) {
    const auto a = TempPath("bad_0001.bin");
    FileQueueManager m;
    std::atomic<int> notified{0};
    m.outSpaceNotify.connect([&]() { notified++; });
    m.openFile(a, false);
    m.startWrite(CStreamSettings::DataFormat::BIN);
    ASSERT_TRUE(m.addChunkToWrite(Chunk("kept")));
    ASSERT_TRUE(m.addFileSwitch("/nonexistent-dir/bad_0002.bin"));
    m.addChunkToWrite(Chunk("lost"));
    const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (notified == 0 && std::chrono::steady_clock::now() < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    m.stopWrite(true);
    m.closeFile();
    EXPECT_EQ(notified, 1);
    EXPECT_TRUE(m.isOutOfSpace());
    EXPECT_FALSE(m.isWork());
    EXPECT_EQ(ReadFile(a), "kept") << "the file before is whole";
}

TEST(FileQueueManager, EveryWavFileCountsItsOwnChunksInItsHeader) {
    const auto a = TempPath("w_0001.wav"), b = TempPath("w_0002.wav");
    FileQueueManager m;
    m.openFile(a, false);
    m.startWrite(CStreamSettings::DataFormat::WAV);
    // the first chunk of a file carries its header (sizes for its own samples), the next ones
    // are added to them by the file thread
    ASSERT_TRUE(m.addChunkToWrite(Chunk(WavHeader(36 + 8, 8) + std::string(8, 'a'))));
    ASSERT_TRUE(m.addChunkToWrite(Chunk(std::string(100, 'a'))));
    ASSERT_TRUE(m.addChunkToWrite(Chunk(std::string(20, 'a'))));
    ASSERT_TRUE(m.addFileSwitch(b));
    ASSERT_TRUE(m.addChunkToWrite(Chunk(WavHeader(36 + 4, 4) + std::string(4, 'b'))));
    ASSERT_TRUE(m.addChunkToWrite(Chunk(std::string(6, 'b'))));
    m.stopWrite(true);
    m.closeFile();
    const std::string fa = ReadFile(a), fb = ReadFile(b);
    ASSERT_EQ(fa.size(), 44u + 128u);
    EXPECT_EQ(U32At(fa, 40), 128u);
    EXPECT_EQ(U32At(fa, 4), 36u + 128u);
    ASSERT_EQ(fb.size(), 44u + 10u);
    EXPECT_EQ(U32At(fb, 40), 10u) << "the second file starts from its own header";
    EXPECT_EQ(U32At(fb, 4), 36u + 10u);
}

TEST(FileQueueManager, AWavHeaderStopsAt4GBInsteadOfWrappingAround) {
    const auto a = TempPath("big.wav");
    FileQueueManager m;
    m.openFile(a, false);
    m.startWrite(CStreamSettings::DataFormat::WAV);
    ASSERT_TRUE(m.addChunkToWrite(Chunk(WavHeader(0xFFFFFFF0u, 0xFFFFFFF0u - 36))));
    ASSERT_TRUE(m.addChunkToWrite(Chunk(std::string(100, 'x'))));
    ASSERT_TRUE(m.addChunkToWrite(Chunk(std::string(100, 'x'))));
    m.stopWrite(true);
    m.closeFile();
    const std::string f = ReadFile(a);
    EXPECT_EQ(U32At(f, 4), 0xFFFFFFFFu) << "readers take it as: to the end of the file";
    EXPECT_EQ(U32At(f, 40), 0xFFFFFFFFu);
}

#ifdef __linux__
TEST(FileQueueManager, AWriteThatFailsStopsTheRecordingOnce) {
    if (!std::filesystem::exists("/dev/full"))
        GTEST_SKIP() << "no /dev/full";
    FileQueueManager m(false);
    std::atomic<int> notified{0};
    m.outSpaceNotify.connect([&]() { notified++; });
    m.openFile("/dev/full", true);
    m.startWrite(CStreamSettings::DataFormat::BIN);
    const std::string text(64 * 1024, 'f');
    for (int i = 0; i < 20; i++)
        m.addChunkToWrite(Chunk(text));
    const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (notified == 0 && std::chrono::steady_clock::now() < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    m.stopWrite(true);
    m.closeFile();
    EXPECT_EQ(notified, 1) << "once, whatever follows";
    EXPECT_TRUE(m.isOutOfSpace());
    EXPECT_FALSE(m.isWork());
}
#endif

TEST(FileQueueManager, AnAppendedFileKeepsWhatWasThere) {
    const auto a = TempPath("append.bin");
    {
        std::ofstream out(a, std::ios::binary | std::ios::trunc);
        out << "before;";
    }
    FileQueueManager m;
    m.openFile(a, true);
    m.startWrite(CStreamSettings::DataFormat::BIN);
    ASSERT_TRUE(m.addChunkToWrite(Chunk("after;")));
    m.stopWrite(true);
    m.closeFile();
    EXPECT_EQ(ReadFile(a), "before;after;");
    EXPECT_EQ(m.getWritedSize(), 6u) << "what this run wrote";
}

TEST(FileQueueManager, ASecondRunCountsFromZeroAndEndsWithAStopNotify) {
    FileQueueManager m;
    int stops = 0;
    m.stopNotify.connect([&]() { stops++; });
    for (int run = 0; run < 2; run++) {
        const auto a = TempPath("again_" + std::to_string(run) + ".bin"), b = TempPath("again_" + std::to_string(run) + "_2.bin");
        m.openFile(a, false);
        m.startWrite(CStreamSettings::DataFormat::BIN);
        ASSERT_TRUE(m.addChunkToWrite(Chunk("12345")));
        ASSERT_TRUE(m.addFileSwitch(b));
        ASSERT_TRUE(m.addChunkToWrite(Chunk("678")));
        m.stopWrite(true);
        m.closeFile();
        EXPECT_EQ(m.getWritedSize(), 8u) << "run " << run;
        EXPECT_EQ(m.filesWritten(), 2) << "run " << run;
        EXPECT_EQ(ReadFile(b), "678");
    }
    EXPECT_EQ(stops, 2);
    EXPECT_FALSE(m.addFileSwitch(TempPath("late.bin"))) << "the thread has stopped";
}

// ---- the time limit on the clock (streaming_improvements 4.11)

TEST(TimeLimitClock, IsDueAfterTheLimitTheLongestBufferAndTheMargin) {
    TimeLimitClock c;
    c.setMargin(1.0);
    c.start(10.0, 0.5);
    const auto t0 = TimeLimitClock::Clock::now();
    auto at = [&](double s) { return t0 + std::chrono::duration_cast<TimeLimitClock::Clock::duration>(std::chrono::duration<double>(s)); };
    EXPECT_FALSE(c.due(at(11.0)));
    EXPECT_TRUE(c.due(at(11.6))) << "10 + 0.5 + 1";
    c.buffer(3.0);  // a longer buffer came: the one of the limit may last that long
    EXPECT_FALSE(c.due(at(13.5)));
    EXPECT_TRUE(c.due(at(14.1)));
    c.buffer(1.0);  // a shorter one changes nothing
    EXPECT_TRUE(c.due(at(14.1)));
    c.off();
    EXPECT_FALSE(c.due(at(100.0))) << "off: never";
    TimeLimitClock none;
    none.start(0, 1);
    EXPECT_FALSE(none.due(at(1e6))) << "no limit";
}

// buffers that come late but come (a ring behind a slow network): the time line ends the limit
TEST(TimeLimitClock, IsNotDueWhileBuffersStillCome) {
    TimeLimitClock c;
    c.setMargin(1.0);
    c.start(10.0, 0.5);
    const auto t0 = TimeLimitClock::Clock::now();
    auto at = [&](double s) { return t0 + std::chrono::duration_cast<TimeLimitClock::Clock::duration>(std::chrono::duration<double>(s)); };
    for (double t = 0; t <= 20; t += 0.5)
        c.buffer(0.5, at(t));
    EXPECT_FALSE(c.due(at(20.2))) << "a buffer 0.2 s ago";
    EXPECT_FALSE(c.due(at(21.4)));
    EXPECT_TRUE(c.due(at(21.6))) << "none for the buffer and the margin";
    // without a hint, before the first buffer: never (a buffer may last minutes)
    TimeLimitClock unknown;
    unknown.start(1.0, 0);
    EXPECT_FALSE(unknown.due(at(1000)));
    unknown.buffer(0.1, at(1000));
    EXPECT_FALSE(unknown.due(at(1002.0))) << "0.1 + the margin of 2 s";
    EXPECT_TRUE(unknown.due(at(1002.2)));
}
