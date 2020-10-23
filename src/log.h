#ifndef BM_SEGMENTER_LOG_H
#define BM_SEGMENTER_LOG_H

#include <string>
#include <utility>

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


#endif //BM_SEGMENTER_LOG_H
