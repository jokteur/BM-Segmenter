#include "events.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <set>

void EventQueue::subscribe(Listener *listener) {
    std::lock_guard<std::mutex> guard(listeners_mutex_);
    listeners_.push_back(listener);
}

void EventQueue::unsubscribe(Listener *listener) {
    std::lock_guard<std::mutex> guard(listeners_mutex_);
    for (auto it = listeners_.begin(); it != listeners_.end(); it++) {
        if (*it == listener) {
            listeners_.erase(it);
            break;
        }
    }
}

void EventQueue::post(const Event &event) {
    std::lock_guard<std::mutex> guard(event_mutex_);
    event_queue_.push(event);
    glfwPostEmptyEvent();
}

void EventQueue::pollEvents() {
    std::lock_guard<std::mutex> event_guard(event_mutex_);
    while(!event_queue_.empty()) {
        Event& event = event_queue_.front();

        std::lock_guard<std::mutex> listener_guard(listeners_mutex_);
        for(const auto &listener : listeners_) {
            bool filter_ok = is_listener(listener->filter, event.name);

            if(filter_ok) {
                listener->callback(event);
            }
        }
        event_queue_.pop();
    }
}

int EventQueue::getNumSubscribers(std::vector<std::string> event_names) {
    std::set<Listener*> listener_set;
    std::lock_guard<std::mutex> listener_guard(listeners_mutex_);
    for(const auto &event_name : event_names) {
        for(const auto listener : listeners_) {
            if (is_listener(listener->filter, event_name)) {
                listener_set.insert(listener);
            }
        }
    }
    return listener_set.size();
}

bool EventQueue::is_listener(const std::string &filter, const std::string &event_name) {
    bool filter_ok = true;

    int i = 0;
    for(;i < filter.size() && i < event_name.size();i++) {
        if(filter[i] == '*') {
            break;
        }
        else if(event_name[i] != filter[i]){
            filter_ok = false;
            break;
        }
    }
    if(i + 1 == filter.size()) {
        if(*(filter.end() - 1) != '*') {
            filter_ok = false;
        }
    }
    else if(i + 1 < filter.size()) {
        filter_ok = false;
    }
    return filter_ok;
}

