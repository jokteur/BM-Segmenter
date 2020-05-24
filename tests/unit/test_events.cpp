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
        .callback = [&num_listens] (std::shared_ptr<Event> event) {
            num_listens++;
        },
    };
    queue.subscribe(&listener1);

    queue.post(Event_ptr(new Event("events/1")));
    queue.post(Event_ptr(new Event("events/2")));
    queue.post(Event_ptr(new Event("event/2"))); // Purposefully post an event with a typo
    queue.post(Event_ptr(new Event("events/3")));

    queue.pollEvents();

    EXPECT_EQ(num_listens, 3) << "Listener did not listen to posted event";

    queue.unsubscribe(&listener1);
    queue.post(Event_ptr(new Event("events/4")));

    queue.pollEvents();

    EXPECT_EQ(num_listens, 3) << "Listener was not unsubscribed";
}


TEST(Events, MultipleListeners) {
    EventQueue& queue = EventQueue::getInstance();

    // Create two listeners that each increments the same variable each time an event is created
    int num_listens = 0;
    Listener listener1{
            .filter = "events*", // Using the wildcard * to listen to multiple events
            .callback = [&num_listens] (Event_ptr event) {
                num_listens++;
            },
    };
    queue.subscribe(&listener1);
    Listener listener2{
            .filter = "events*", // Using the wildcard * to listen to multiple events
            .callback = [&num_listens] (Event_ptr event) {
                num_listens++;
            },
    };
    queue.subscribe(&listener2);

    queue.post(Event_ptr(new Event("events/1")));
    queue.post(Event_ptr(new Event("events/2")));
    queue.post(Event_ptr(new Event("events/3")));

    queue.pollEvents();

    EXPECT_EQ(num_listens, 6) << "At least one listener was not notified";

    queue.unsubscribe(&listener1);
    queue.post(Event_ptr(new Event("events/4")));

    queue.pollEvents();

    EXPECT_EQ(num_listens, 7) << "Listener was not unsubscribed";
    queue.unsubscribe(&listener2);
}

TEST(Events, AcknowledgableEvents) {
    EventQueue& queue = EventQueue::getInstance();

    int num_acknowledges = 0;

    auto post = [&queue, &num_acknowledges] () {
        // Get the number of listeners for the event this function will post
        // This number should decrease once the events are acknowledged
        num_acknowledges = queue.getNumSubscribers({"events/1"}) + queue.getNumSubscribers({"events/2", "events/3"})*2;
        EXPECT_EQ(num_acknowledges, 4) << "The expect num_acknowledges is not correct";

        // To the poster knowledge, there should be 2 listeners to the event he will post

        // When the events are posted, the listener is still subscribed
        queue.post(Event_ptr(new Event("events/1", true)));
        queue.post(Event_ptr(new Event("events/2", true)));
        queue.post(Event_ptr(new Event("events/3", true)));
    };

    // First listener listens to all "events"
    Listener listener1{
            .filter = "events*", // Using the wildcard * to listen to multiple events
            .callback = [&num_acknowledges] (Event_ptr event) {
                //here the acknowledge would be simulated by a simple decrement of the num_acknowledges
                num_acknowledges--;
            },
    };
    queue.subscribe(&listener1);

    // Second listener only listens to events/1
    Listener listener2{
            .filter = "events/1", // Using the wildcard * to listen to multiple events
            .callback = [&num_acknowledges] (Event_ptr event) {
                //here the acknowledge would be simulated by a simple decrement of the num_acknowledges
                num_acknowledges--;
            },
    };
    queue.subscribe(&listener2);

    post();


    // In theory, once a listener is unsubscribed, he would not be called in the next pollEvents()
    // but as the events still need to be acknowledge, the unsubscribe should happen after pollEvents()
    queue.unsubscribe(&listener1);
    queue.unsubscribe(&listener2);
    queue.pollEvents();

    EXPECT_EQ(num_acknowledges, 0) << "Not all events have been acknowledged by the listeners";
}

#endif