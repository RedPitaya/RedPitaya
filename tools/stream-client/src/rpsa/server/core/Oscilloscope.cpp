#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "rpsa/server/core/Oscilloscope.h"

namespace
{
//!
//!@brief Map register using UIO.
//!
//!@param _fd File descriptor.
//!@param _size The register map size.
//!@param _number The register map number.
//!@return The memory map pointer.
//!
void * MmapNumber(int _fd, size_t _size, size_t _number) {
    const size_t offset = _number * getpagesize();
    return mmap(nullptr, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, offset);
}
}

COscilloscope::Ptr COscilloscope::Create(const UioT &_uio, unsigned _channel,uint32_t _dec_factor)
{
    // Validation
    if ((_channel > 2) || (_uio.mapList.size() < 2))
    {
        // Error: validation.
        std::cerr << "Error: UIO validation." << std::endl;
        return COscilloscope::Ptr();
    }

    if (_uio.mapList[1].size < (osc_buf_size * 4))
    {
        // Error: buffer size.
        std::cerr << "Error: buffer size." << std::endl;
        return COscilloscope::Ptr();
    }

    // Open file
    std::string path("/dev/" + _uio.name);
    int fd = open(path.c_str(), O_RDWR);

    if (fd == -1)
    {
        // Error: open file.
        std::cerr << "Error: open file." << std::endl;
        return COscilloscope::Ptr();
    }

    // Map
    void *regset = MmapNumber(fd, _uio.mapList[0].size, 0);

    if (regset == MAP_FAILED)
    {
        // Error: mmap
        std::cerr << "Error: mmap regset." << std::endl;
        close(fd);
        return COscilloscope::Ptr();
    }

    void *buffer = MmapNumber(fd, _uio.mapList[1].size, 1);

    if (buffer == MAP_FAILED)
    {
        // Error: mmap
        std::cerr << "Error: mmap buffer." << std::endl;
        munmap(regset, _uio.mapList[0].size);
        close(fd);
        return COscilloscope::Ptr();
    }

   
    return std::make_shared<COscilloscope>(_channel, fd, regset, _uio.mapList[0].size, buffer, _uio.mapList[1].size, _uio.mapList[1].addr,_dec_factor);
}

COscilloscope::COscilloscope(unsigned _channel, int _fd, void *_regset, size_t _regsetSize, void *_buffer, size_t _bufferSize, uintptr_t _bufferPhysAddr,uint32_t _dec_factor) :
    m_Channel(_channel),
    m_Fd(_fd),
    m_Regset(_regset),
    m_RegsetSize(_regsetSize),
    m_Buffer(_buffer),
    m_BufferSize(_bufferSize),
    m_BufferPhysAddr(_bufferPhysAddr),
    m_OscMap(nullptr),
    m_OscBuffer(nullptr),
    m_OscBufferNumber(0),
    m_dec_factor(_dec_factor)
{
    uintptr_t oscMap = reinterpret_cast<uintptr_t>(m_Regset) + (_channel == 0 ? osc0_baseaddr : osc1_baseaddr);
    m_OscMap = reinterpret_cast<OscilloscopeMapT *>(oscMap);
    m_OscBuffer = static_cast<uint8_t *>(m_Buffer) + osc_buf_size * _channel * 2;
}

COscilloscope::~COscilloscope()
{
    munmap(m_Regset, m_RegsetSize);
    munmap(m_Buffer, m_BufferSize);
    close(m_Fd);
}

void COscilloscope::prepare()
{
    stop();

    // Buffer
    m_OscMap->dma_buf_size = osc_buf_size;
    m_OscMap->dma_dst_addr1 = m_BufferPhysAddr + osc_buf_size * (m_Channel * 2);
    m_OscMap->dma_dst_addr2 = m_BufferPhysAddr + osc_buf_size * (m_Channel * 2 + 1);

    // Filter bypass
    m_OscMap->filt_bypass = UINT32_C(0x00000001);

    // Trigger mask
    m_OscMap->trig_mask = UINT32_C(0x00000004);

    // Trigger low level
    m_OscMap->trig_low_level = -4;

    // Trigger high level
    m_OscMap->trig_high_level = 4;

    // Trigger edge
    m_OscMap->trig_edge = UINT32_C(0x00000000);

    // Trigger pre samples
    m_OscMap->trig_pre_samp = osc_buf_pre_samp;

    // Trigger post samples
    m_OscMap->trig_post_samp = osc_buf_post_samp;

    // Decimate factor
    m_OscMap->dec_factor = m_dec_factor;

    // DMA start
    m_OscMap->dma_ctrl = UINT32_C(0x00000101);

    // Event
    m_OscMap->event_sel = (m_Channel == 0) ? osc0_event_id : osc1_event_id;

    // Control reset
    m_OscMap->event_sts = UINT32_C(0x00000001);

    // Control strart
    m_OscMap->event_sts = UINT32_C(0x00000002);
}

bool COscilloscope::next(uint8_t *&_buffer, size_t &_size)
{
    // Enable interrupt
    int32_t cnt = 1;
    constexpr size_t cnt_size = sizeof(cnt);
    ssize_t bytes = write(m_Fd, &cnt, cnt_size);

    if (bytes == cnt_size) {
        // Wait for interrupt
        bytes = read(m_Fd, &cnt, cnt_size);

        if (bytes == cnt_size) {
            // Interrupt ACQ
            m_OscMap->dma_ctrl |= UINT32_C(0x00000002);

            _buffer = m_OscBuffer + osc_buf_size * m_OscBufferNumber;
            _size = osc_buf_size;
            m_OscBufferNumber = (m_OscBufferNumber == 0) ? 1 : 0;
            return true;
        }
    }

    return false;
}

void COscilloscope::stop()
{
    // Control stop
    m_OscMap->event_sts = UINT32_C(0x00000004);
}
