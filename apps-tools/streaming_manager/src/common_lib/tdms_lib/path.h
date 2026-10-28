#ifndef TDMS_LIB_PATH_H
#define TDMS_LIB_PATH_H

#include <string>
#include <vector>

namespace TDMS {

// Object paths: "/" the file, "/'group'" a group, "/'group'/'channel'" a channel; a quote in a
// name is written twice.
auto groupPath(const std::string& group) -> std::string;
auto channelPath(const std::string& group, const std::string& channel) -> std::string;

// The names of a path: none for the file, one for a group, two for a channel.
// False for anything that is not a path of that form.
auto parsePath(const std::string& path, std::vector<std::string>& names) -> bool;

}  // namespace TDMS

#endif
