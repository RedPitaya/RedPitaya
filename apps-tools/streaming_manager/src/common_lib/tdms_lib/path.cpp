#include "path.h"

namespace {

auto quoted(const std::string& name) -> std::string {
    std::string out = "'";
    for (char c : name) {
        out += c;
        if (c == '\'')
            out += '\'';
    }
    return out + "'";
}

}  // namespace

auto TDMS::groupPath(const std::string& group) -> std::string {
    return "/" + quoted(group);
}

auto TDMS::channelPath(const std::string& group, const std::string& channel) -> std::string {
    return "/" + quoted(group) + "/" + quoted(channel);
}

auto TDMS::parsePath(const std::string& path, std::vector<std::string>& names) -> bool {
    names.clear();
    if (path == "/")
        return true;
    size_t i = 0;
    while (i < path.size()) {
        if (path[i] != '/' || i + 1 >= path.size() || path[i + 1] != '\'')
            return false;
        i += 2;
        std::string name;
        bool closed = false;
        while (i < path.size()) {
            if (path[i] == '\'') {
                if (i + 1 < path.size() && path[i + 1] == '\'') {
                    name += '\'';
                    i += 2;
                    continue;
                }
                closed = true;
                i++;
                break;
            }
            name += path[i++];
        }
        if (!closed)
            return false;
        names.push_back(name);
    }
    return !names.empty() && names.size() <= 2;
}
