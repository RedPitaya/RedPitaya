/**
 * @file    map_handler.hpp
 * @author  Pavel A. Bobrov <p.bobrov(at)integrasources.com>
 * @date    15.06.2021
 */

#ifndef MAP_HANDLER_HPP
#define MAP_HANDLER_HPP

#include <cstdio>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/serialization.hpp>

#include "asio.hpp"


namespace asionet {

template<typename Key, typename Value>
class MapHandler {
public:
    static void serialize(const std::map<Key, Value>& _map, asio::streambuf& _out_streambuf)
    {
        std::printf("Serialization...\n");

        std::ostream os {&_out_streambuf};
        boost::archive::text_oarchive oar(os);
        oar << _map;

        std::printf("Serialization... Done\n");
    }

    static void deserialize(asio::streambuf& _in_streambuf, std::map<Key, Value>& _map)
    {
        std::printf("Deserialization...\n");

        std::istream is {&_in_streambuf};
        boost::archive::text_iarchive iar(is);
        iar >> _map;

        std::printf("Deserialization... Done\n");
    }

    static Value safeAt(const std::map<Key, Value>& _map, Key _key) {

       Value v {};

       try {
           v = _map.at(_key);
       }
       catch (const std::out_of_range& ex) {
           std::printf("Exception: %s", ex.what());
       }

       return v;
   }

private:
    friend class boost::serialization::access;
};

} // namespace asionet

#endif // MAP_HANDLER_HPP
