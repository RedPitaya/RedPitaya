#include "file_writer.h"

#include <filesystem>
#include <system_error>

using namespace TDMS;

FileWriter::~FileWriter() {
    close();
}

auto FileWriter::open(const std::string& path, bool append) -> bool {
    close();
    m_error.clear();
    m_path = path;
    m_size = 0;
    if (append) {
        std::error_code ec;
        const auto size = std::filesystem::file_size(path, ec);
        if (!ec)
            m_size = size;
    }
    m_file.open(path, std::ios::binary | std::ios::out | (append ? std::ios::app : std::ios::trunc));
    if (!m_file.is_open()) {
        m_error = "cannot open " + path + " for writing";
        return false;
    }
    return true;
}

auto FileWriter::close() -> bool {
    if (!m_file.is_open())
        return m_error.empty();
    m_file.close();
    if (m_file.fail() && m_error.empty())
        m_error = "cannot write " + m_path;
    m_file.clear();
    return m_error.empty();
}

auto FileWriter::isOpen() const -> bool {
    return m_file.is_open();
}

auto FileWriter::error() const -> const std::string& {
    return m_error;
}

auto FileWriter::write(SegmentBuilder& segment) -> bool {
    if (!m_file.is_open()) {
        m_error = "the file is not open";
        return false;
    }
    for (const auto& part : segment.parts()) {
        m_file.write(reinterpret_cast<const char*>(part.data), static_cast<std::streamsize>(part.size));
        if (!m_file) {
            m_error = "cannot write " + m_path;
            return false;
        }
        m_size += part.size;
    }
    return true;
}

auto FileWriter::flush() -> bool {
    if (!m_file.is_open())
        return false;
    m_file.flush();
    if (!m_file) {
        m_error = "cannot write " + m_path;
        return false;
    }
    return true;
}

auto FileWriter::size() const -> std::uint64_t {
    return m_size;
}
