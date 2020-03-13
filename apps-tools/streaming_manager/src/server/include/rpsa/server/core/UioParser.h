#pragma once

#include <cstdint>
#include <string>
#include <vector>

//!
//! \brief The register map description.
//!
struct UioMapT
{
    std::string name; //!< The register map name.
    uintptr_t addr; //!< The register map address.
    uintptr_t offset; //!< The register map offset.
    uintptr_t size; //!< The register map size.

    UioMapT(const std::string &_name, uintptr_t _addr, uintptr_t _offset, uintptr_t _size);
};

//!
//! \brief The UIO device description.
//!
struct UioT
{
    std::string name; //!< UIO name (uioX).
    std::string nodeName; //!< The device-tree node name.
    std::vector<UioMapT> mapList; //!< The register map list.

    UioT(const std::string &_name, const std::string &_nodeName, const std::vector<UioMapT> &_mapList);
};

//!
//! \brief Get the UIO list.
//!
//! \return std::vector<UioT> UIO list.
//!
std::vector<UioT> GetUioList();
