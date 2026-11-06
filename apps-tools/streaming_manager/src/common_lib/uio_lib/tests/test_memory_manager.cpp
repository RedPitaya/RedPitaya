// The block size check of the memory manager (the host build's heap manager)

#include <gtest/gtest.h>

#include "data_lib/network_header.h"
#include "uio_lib/memory_manager.h"

using uio_lib::CMemoryManager;

namespace {

class MemoryBlockSize : public ::testing::Test {
   protected:
    void SetUp() override { m_before = CMemoryManager::instance()->getMemoryBlockSize(); }
    void TearDown() override {
        // the instance is the process's; leave the block size it had
        CMemoryManager::instance()->setMemoryBlockSize(m_before - DataLib::sizeHeader());
    }
    uint32_t m_before = 0;
};

}  // namespace

TEST_F(MemoryBlockSize, TakesASizeTheRAMHolds) {
    auto mm = CMemoryManager::instance();
    ASSERT_TRUE(mm->setMemoryBlockSize(4096));
    EXPECT_EQ(mm->getMemoryBlockSize(), 4096 + DataLib::sizeHeader());
    EXPECT_TRUE(mm->reallocateBlocks());
}

TEST_F(MemoryBlockSize, RefusesZeroAndKeepsTheSize) {
    auto mm = CMemoryManager::instance();
    ASSERT_TRUE(mm->setMemoryBlockSize(4096));
    EXPECT_FALSE(mm->setMemoryBlockSize(0));
    EXPECT_EQ(mm->getMemoryBlockSize(), 4096 + DataLib::sizeHeader());
}

TEST_F(MemoryBlockSize, RefusesASizeThatWrapsWithTheHeader) {
    auto mm = CMemoryManager::instance();
    ASSERT_TRUE(mm->setMemoryBlockSize(4096));
    // 0xFFFFFF80 + the 128 byte header was 0 in 32 bits: a division by zero in reallocateBlocks
    EXPECT_FALSE(mm->setMemoryBlockSize(0xFFFFFF80u));
    EXPECT_FALSE(mm->setMemoryBlockSize(0xFFFFFFFFu));
    EXPECT_EQ(mm->getMemoryBlockSize(), 4096 + DataLib::sizeHeader());
    EXPECT_TRUE(mm->reallocateBlocks());
}

TEST_F(MemoryBlockSize, RefusesBlocksTheRAMDoesNotHold) {
    auto mm = CMemoryManager::instance();
    const uint32_t ram = mm->getRAMSize();
    // 12 blocks are the least a stream needs
    EXPECT_FALSE(mm->setMemoryBlockSize(ram / 12));
    EXPECT_TRUE(mm->setMemoryBlockSize(ram / 12 - DataLib::sizeHeader()));
}
