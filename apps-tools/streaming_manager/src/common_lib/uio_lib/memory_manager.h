#ifndef UIO_LIB_MEMORY_MANAGER_H
#define UIO_LIB_MEMORY_MANAGER_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace uio_lib {

// the GPIO input and output stream independently, each in memory of its own
enum MemoryTAG { MM_NONE = 0, MM_ADC_RESERVE_SKIP = 1, MM_ADC = 2, MM_DAC = 3, MM_GPIO_IN = 4, MM_GPIO_OUT = 5 };

struct MemoryRegionT {
    MemoryTAG tag{MM_NONE};
    uint32_t start{0};
    uint32_t end{0};
    uint8_t* startMemory{0};
    size_t size;
    bool isFree{false};
};

class CMemoryManager {
   public:
    using Ptr = std::shared_ptr<CMemoryManager>;

    static auto instance() -> Ptr;

    CMemoryManager();
    ~CMemoryManager();

    auto reserveMemory(MemoryTAG tag, uint32_t blockCount, uint32_t channelsCount) -> uint32_t;
    auto releaseMemory(MemoryTAG tag) -> void;
    auto reallocateBlocks() -> bool;
    auto getRegions(MemoryTAG tag) -> std::vector<MemoryRegionT>;
    auto getFreeBlockCount() -> uint32_t;

    // _size - the data bytes of a block, without the header. Refused (the block size stays) for 0
    // or when the RAM does not hold the blocks the streams need.
    auto setMemoryBlockSize(uint32_t _size) -> bool;
    auto getMemoryBlockSize() -> uint32_t;

    auto getMinRAMSize(MemoryTAG _tag) -> uint32_t;
    auto getMinRAMSize(uint32_t _blockSize, MemoryTAG _tag) -> uint32_t;
    auto getMinRequiredRAM() -> uint32_t;
    auto getMaxBlockSize() -> uint32_t;

    auto setReserverdMemory(MemoryTAG _tag, uint32_t size) -> bool;
    auto getReserverdMemory(MemoryTAG _tag) -> uint32_t;
    auto getRAMSize() -> uint32_t;
    auto isValidSize(MemoryTAG _tag) -> bool;

    auto setDACMemoryStreamMode(bool enable) -> void;

    // The memory comes from the rp_resmem driver as pages the kernel can pin: the network can
    // send it with MSG_ZEROCOPY. False for a "no-map" region (and for the heap of the host build).
    auto isZeroCopy() const -> bool { return m_zeroCopy; }

   private:
    CMemoryManager(const CMemoryManager&) = delete;
    CMemoryManager(CMemoryManager&&) = delete;
    CMemoryManager& operator=(const CMemoryManager&) = delete;
    CMemoryManager& operator=(const CMemoryManager&&) = delete;

    uint32_t m_lowReservedAddress;
    uint32_t m_highReservedAddress;

    std::mutex m_mtx;

    std::vector<MemoryRegionT> m_heap;
    int m_mem_fd;
    void* m_memory;
    uint32_t m_blockSize;
    uint32_t m_startRAMAddress;
    uint32_t m_ramSize;
    std::map<MemoryTAG, uint32_t> m_reservedMemory;
    bool m_dacStreamMode;
    bool m_zeroCopy = false;
};

}  // namespace uio_lib
#endif
