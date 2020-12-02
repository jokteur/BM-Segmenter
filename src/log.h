#ifndef BM_SEGMENTER_LOG_H
#define BM_SEGMENTER_LOG_H

#include <string>
#include <utility>
#include <mutex>

#include "events.h"

/**
 * @brief Custom event class for sending some log info
 */
class LogEvent : public Event {
private:
    std::string message_;
public:
    explicit LogEvent(const std::string& name, std::string message) : message_(std::move(message)), Event(std::string("log/") + name) {}

    std::string& getMessage() { return message_; }
};
#define LOGEVENT_PTRCAST(job) (reinterpret_cast<LogEvent*>((job)))

class DebugLogger {
private:
    std::vector<std::string> logs_;
    std::string out_file_;
    bool print_std_;
    Listener event_listener_;

public:
    DebugLogger(const std::string& out, bool print_std = false);
    ~DebugLogger();

};

void debug_event(const std::string& file, const std::string& func, const std::string& str);

#ifdef LOG_DEBUG
#define DEBUG(str) debug_event(__FILE__, __FUNCSIG__, (str))
#else
#define DEBUG(str) ;
#endif

#endif //BM_SEGMENTER_LOG_H
