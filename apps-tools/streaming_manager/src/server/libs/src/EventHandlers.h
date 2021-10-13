#pragma once

#include <functional>
#include <map>
#include <vector>


template<typename... Args>
class EventList {
private:
    typedef std::function<void(Args...)> EventHandler;
    typedef std::vector<EventHandler> EventHandlerList;
    std::map<int, EventHandlerList> m_handlers {};

public:

    void addListener(int event_type, EventHandler listener) {
        if (hasEvent(event_type)) {
            EventHandlerList& list = m_handlers.at(event_type);
            list.push_back(listener);
        }
        else {
            std::vector<EventHandler> list;
            list.push_back(listener);
            m_handlers.insert(std::make_pair(event_type, list));
        }
    }

    void emitEvent(int event_type, Args... vals) {
        if (hasEvent(event_type)) {
            EventHandlerList& list = m_handlers.at(event_type);
            for (auto& func : list) {
                func(vals...);
            }
        }
    }

    bool hasEvent(int event_type) {
        return m_handlers.find(event_type) != m_handlers.end();
    }
};
