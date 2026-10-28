/**
 * The way to the file: FileChunk (blocks written from where they are), the queue with its memory
 * limit, and the file thread of FileQueueManager, which waits for chunks instead of spinning.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "writer_lib/file_queue_manager.h"

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
