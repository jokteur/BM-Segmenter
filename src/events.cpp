#include "events.h"

#include <GLFW/glfw3.h>

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
            bool filter_ok = true;

            int i = 0;
            for(;i < listener->filter.size() && i < event.name.size();i++) {
                if(listener->filter[i] == '*') {
                    break;
                }
                else if(event.name[i] != listener->filter[i]){
                    filter_ok = false;
                    break;
                }
            }
            if(i + 1 == listener->filter.size()) {
                if(*(listener->filter.end() - 1) != '*') {
                    filter_ok = false;
                }
            }
            else if(i + 1 < listener->filter.size()) {
                filter_ok = false;
            }

            if(filter_ok) {
                listener->callback(event);
            }
        }
        event_queue_.pop();
    }
}

