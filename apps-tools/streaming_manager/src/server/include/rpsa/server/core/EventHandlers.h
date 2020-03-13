//
// Created by user on 02.04.19.
//
#include <functional>
#include <map>
#include <vector>

#ifndef PROJECT_EVENTHANDLERS_H
#define PROJECT_EVENTHANDLERS_H


template <typename T>
class EventList {
private:
    typedef std::function<void(T)> EventHandler;
    typedef std::vector<EventHandler> EventHandlerList;
    std::map<int, EventHandlerList> m_handlers;

public:


    void addListener(int eventId, EventHandler listener){
        if(hasEvent(eventId)){
            EventHandlerList& list = m_handlers.at(eventId);
            list.push_back(listener);
        }else{
           std::vector<EventHandler> list;
           list.push_back(listener);
           m_handlers.insert(std::make_pair(eventId, list));
        }

    }

    void emitEvent(int eventId,T value){
        if(hasEvent(eventId)){
            EventHandlerList& list = m_handlers.at(eventId);
            for(auto & func : list){
                func(value);
            }
        }
    }


    bool hasEvent(int eventId){
        return m_handlers.find(eventId)!=m_handlers.end();
    }
};

template <typename T, typename E>
class EventList2 {
private:
    typedef std::function<void(T,E)> EventHandler;
    typedef std::vector<EventHandler> EventHandlerList;
    std::map<int, EventHandlerList> m_handlers;

public:
    void addListener(int eventId, EventHandler listener){
        if(hasEvent(eventId)){
            EventHandlerList& list = m_handlers.at(eventId);
            list.push_back(listener);
        }else{
            std::vector<EventHandler> list;
            list.push_back(listener);
            m_handlers.insert(std::make_pair(eventId, list));
        }
    }

    void emitEvent(int eventId,T param1,E param2){
        if(hasEvent(eventId)){
            EventHandlerList& list = m_handlers.at(eventId);
            for(auto & func : list){
                func(param1,param2);
            }
        }
    }


    bool hasEvent(int eventId){
        return m_handlers.find(eventId)!=m_handlers.end();
    }
};

template <typename T, typename E,typename F>
class EventList3 {
private:
    typedef std::function<void(T,E,F)> EventHandler;
    typedef std::vector<EventHandler> EventHandlerList;
    std::map<int, EventHandlerList> m_handlers;

public:
    void addListener(int eventId, EventHandler listener){
        if(hasEvent(eventId)){
            EventHandlerList& list = m_handlers.at(eventId);
            list.push_back(listener);
        }else{
            std::vector<EventHandler> list;
            list.push_back(listener);
            m_handlers.insert(std::make_pair(eventId, list));
        }
    }

    void emitEvent(int eventId,T param1,E param2,F param3){
        if(hasEvent(eventId)){
            EventHandlerList& list = m_handlers.at(eventId);
            for(auto & func : list){
                func(param1,param2,param3);
            }
        }
    }


    bool hasEvent(int eventId){
        return m_handlers.find(eventId)!=m_handlers.end();
    }
};

#endif //PROJECT_EVENTHANDLERS_H
