#ifndef BM_SEGMENTER_TEST_WINDOW_H
#define BM_SEGMENTER_TEST_WINDOW_H

#include <exception>
#include <thread>

#include "events.h"
#include <gtest/gtest.h>

TEST(Events, OneListener) {
    EventQueue& queue = EventQueue::getInstance();

    // Create a listener that increments a variable each time an event is created
    int num_listens = 0;
    Listener listener1{
        .filter = "events*", // Using the wildcard * to listen to multiple events
        .callback = [&num_listens] (Event event) {
            num_listens++;
        },
    };
    queue.subscribe(&listener1);

    queue.post(Event{.name = "events/1"});
    queue.post(Event{.name = "events/2"});
    queue.post(Event{.name = "event/3"}); // Purposefully post an event with a typo
    queue.post(Event{.name = "events/3"});

    queue.pollEvents();

    EXPECT_EQ(num_listens, 3) << "Listener did not listen to posted event";

    queue.unsubscribe(&listener1);
    queue.post(Event{.name = "events/4"});

    queue.pollEvents();

    EXPECT_EQ(num_listens, 3) << "Listener was not unsubscribed";
}


TEST(Events, MultipleListeners) {
    EventQueue& queue = EventQueue::getInstance();

    // Create two listeners that each increments the same variable each time an event is created
    int num_listens = 0;
    Listener listener1{
            .filter = "events*", // Using the wildcard * to listen to multiple events
            .callback = [&num_listens] (Event event) {
                num_listens++;
            },
    };
    queue.subscribe(&listener1);
    Listener listener2{
            .filter = "events*", // Using the wildcard * to listen to multiple events
            .callback = [&num_listens] (Event event) {
                num_listens++;
            },
    };
    queue.subscribe(&listener1);

    queue.post(Event{.name = "events/1"});
    queue.post(Event{.name = "events/2"});
    queue.post(Event{.name = "events/3"});

    queue.pollEvents();

    EXPECT_EQ(num_listens, 6) << "At least one listener was not notified";

    queue.unsubscribe(&listener1);
    queue.post(Event{.name = "events/4"});

    queue.pollEvents();

    EXPECT_EQ(num_listens, 7) << "Listener was not unsubscribed";
}



#endif