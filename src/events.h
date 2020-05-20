#ifndef BM_SEGMENTER_EVENTS_H
#define BM_SEGMENTER_EVENTS_H

#include <vector>
#include <queue>
#include <string>
#include <chrono>
#include <functional>
#include <mutex>

/**
 * The events naming convention should follow a similar POSIX-folder structure
 * e.g.:
 *  jobs/job_1
 *  jobs/job_2
 *  jobs/error_with_scheduler
 *
 *  When selecting events, one can filter as one would do on a bash console :
 *  `jobs*` will select everything that begins with jobs
 *
 *  The wildcard * can only be used at the end of strings
 */
struct Event {
    std::string name;
    std::chrono::system_clock::time_point time;
};

struct Listener {
    std::string filter;
    std::function<void (Event)> callback;
};

class EventQueue {
private:
    std::queue<Event> event_queue_;
    std::mutex event_mutex_;
    std::vector<Listener*> listeners_;
    std::mutex listeners_mutex_;

    bool is_listener(const std::string &filter, const std::string &event_name);

    EventQueue() {}
public:
    EventQueue(EventQueue const &) = delete;
    void operator=(EventQueue const &) = delete;

    static EventQueue& getInstance () {
        static EventQueue instance;
        return instance;
    }

    void subscribe(Listener* listener);
    void unsubscribe(Listener* listener);

    int getNumSubscribers(std::vector<std::string> event_names);

    void post(const Event &event);

    void pollEvents();

};


#endif //BM_SEGMENTER_EVENTS_H
