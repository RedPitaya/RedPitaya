#include <cinttypes>
#include <exception>
#include <fstream>
#include <dirent.h>
#include "rpsa/server/core/UioParser.h"

namespace
{
class CUioException : public std::exception
{
public:
    CUioException() :
        std::exception()
    {
    }
};

std::string GetUioNodeName(const std::string &_path)
{
    std::string nodeName;
    std::ifstream ifs(_path + "/name");

    if (ifs.is_open())
    {
        std::string tmpNodeName;
        std::getline(ifs, tmpNodeName);

        if (ifs.good())
        {
            nodeName = tmpNodeName;
        }
        else
        {
            ifs.close();
            throw CUioException();
        }

        ifs.close();
    }
    else
    {
        throw CUioException();
    }

    return nodeName;
}

uintptr_t UioReadUintPtr(const std::string &_path)
{
    FILE *file = fopen(_path.c_str(), "r");

    if (!file) {
        throw CUioException();
    }

    uintptr_t value;

    if (fscanf(file, "0x%" SCNxPTR "\n", &value) != 1) {
        // Error: scanf
        fclose(file);
        throw CUioException();
    }

    fclose(file);
    return value;
}

std::vector<UioMapT> GetUioMapList(const std::string &_path)
{
    std::vector<UioMapT> mapList;

    try
    {
        for (size_t i = 0;; ++i)
        {
            std::string mapPath = _path + "/maps/map" + std::to_string(i);

            // Name
            std::ifstream ifs(mapPath + "/name");

            if (!ifs.is_open())
            {
                throw CUioException();
            }

            std::string name;
            std::getline(ifs, name);

            if (!ifs.good())
            {
                ifs.close();
                throw CUioException();
            }

            ifs.close();

            uintptr_t addr = UioReadUintPtr(mapPath + "/addr");
            uintptr_t offset = UioReadUintPtr(mapPath + "/offset");
            uintptr_t size = UioReadUintPtr(mapPath + "/size");

            mapList.emplace_back(name, addr, offset, size);
        }
    }
    catch (const CUioException &)
    {
        // Parse error.
    }

    return mapList;
}
}

UioMapT::UioMapT(const std::string &_name, uintptr_t _addr, uintptr_t _offset, uintptr_t _size) :
    name(_name),
    addr(_addr),
    offset(_offset),
    size(_size)
{
}

UioT::UioT(const std::string &_name, const std::string &_nodeName, const std::vector<UioMapT> &_mapList) :
    name(_name),
    nodeName(_nodeName),
    mapList(_mapList)
{
}

std::vector<UioT> GetUioList()
{
    std::vector<UioT> uioList;

    static const std::string base_path = "/sys/class/uio/";

    // Enumerate UIO
    DIR *dir = opendir(base_path.c_str());

    if (dir)
    {
        dirent *next = nullptr;

        while ((next = readdir(dir)))
        {
            const std::string name = next->d_name;

            if ((name == ".") || (name == ".."))
            {
                continue;
            }

            try
            {
                std::string nodeName = GetUioNodeName(base_path + "/" + name);
                std::vector<UioMapT> mapList = GetUioMapList(base_path + "/" + name);
                uioList.emplace_back(name, nodeName, mapList);
            }
            catch (const CUioException &)
            {
                // Parse error.
            }
        }

        closedir(dir);
    }

    return uioList;
}
