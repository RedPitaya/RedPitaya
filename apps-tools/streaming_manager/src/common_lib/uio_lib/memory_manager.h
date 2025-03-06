#ifndef UIO_LIB_MEMORY_MANAGER_H
#define UIO_LIB_MEMORY_MANAGER_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace uio_lib {

enum MemoryTAG { MM_NONE = 0, MM_ADC_RESERVE_SKIP = 1, MM_ADC = 2, MM_DAC = 3, MM_GPIO = 4 };

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

    auto setMemoryBlockSize(uint32_t _size) -> void;
    auto getMemoryBlockSize() -> uint32_t;

    auto getMinRAMSize(MemoryTAG _tag) -> uint32_t;
    auto getMinRAMSize(uint32_t _blockSize, MemoryTAG _tag) -> uint32_t;
    auto getMinRequiredRAM() -> uint32_t;
    auto getMaxBlockSize() -> uint32_t;

    auto setReserverdMemory(MemoryTAG _tag, uint32_t size) -> bool;
    auto getReserverdMemory(MemoryTAG _tag) -> uint32_t;
    auto getRAMSize() -> uint32_t;
    auto isValidSize(MemoryTAG _tag) -> bool;

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
};

}  // namespace uio_lib
#endif
