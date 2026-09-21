#include <gtest/gtest.h>

#include <functional>
#include <stdexcept>
#include <vector>

#include "game_event_dispatcher.h"

// Verifies that dispatching an empty queue is safe (e.g. no queued player actions).
TEST(game_event_dispatcher_t_test, dispatches_no_events)
{
    game_event_dispatcher_t dispatcher;

    EXPECT_NO_THROW(dispatcher.dispatch_pending());
}

// Verifies that events run in their arrival order (e.g. move before attack).
TEST(game_event_dispatcher_t_test, dispatches_events_in_fifo_order)
{
    game_event_dispatcher_t dispatcher;
    std::vector<int> execution_order;

    dispatcher.enqueue([&execution_order]() { execution_order.push_back(1); });
    dispatcher.enqueue([&execution_order]() { execution_order.push_back(2); });
    dispatcher.enqueue([&execution_order]() { execution_order.push_back(3); });

    dispatcher.dispatch_pending();

    EXPECT_EQ(execution_order, (std::vector<int>{1, 2, 3}));
}

// Verifies that an event cannot run more than once (e.g. one spawn request).
TEST(game_event_dispatcher_t_test, dispatches_each_event_once)
{
    game_event_dispatcher_t dispatcher;
    auto execution_count = 0;

    dispatcher.enqueue([&execution_count]() { ++execution_count; });

    dispatcher.dispatch_pending();
    dispatcher.dispatch_pending();

    EXPECT_EQ(execution_count, 1);
}

// Verifies that newly queued events wait behind existing events (e.g. a follow-up action).
TEST(game_event_dispatcher_t_test, dispatches_new_events_at_the_end_of_the_queue)
{
    game_event_dispatcher_t dispatcher;
    std::vector<int> execution_order;

    dispatcher.enqueue([&dispatcher, &execution_order]() {
        execution_order.push_back(1);
        dispatcher.enqueue([&execution_order]() { execution_order.push_back(3); });
    });
    dispatcher.enqueue([&execution_order]() { execution_order.push_back(2); });

    dispatcher.dispatch_pending();

    EXPECT_EQ(execution_order, (std::vector<int>{1, 2, 3}));
}

// Verifies that nested dispatch calls do not interrupt the active dispatch (e.g. an event requests dispatch).
TEST(game_event_dispatcher_t_test, ignores_reentrant_dispatch)
{
    game_event_dispatcher_t dispatcher;
    std::vector<int> execution_order;

    dispatcher.enqueue([&dispatcher, &execution_order]() {
        execution_order.push_back(1);
        dispatcher.enqueue([&execution_order]() { execution_order.push_back(4); });
        dispatcher.dispatch_pending();
        execution_order.push_back(2);
    });
    dispatcher.enqueue([&execution_order]() { execution_order.push_back(3); });

    dispatcher.dispatch_pending();

    EXPECT_EQ(execution_order, (std::vector<int>{1, 2, 3, 4}));
}

// Verifies that empty callbacks are rejected.
TEST(game_event_dispatcher_t_test, rejects_empty_events)
{
    game_event_dispatcher_t dispatcher;
    std::function<void()> empty_event;

    EXPECT_THROW(dispatcher.enqueue(empty_event), std::invalid_argument);
}

// Verifies that the queue remains usable after an event throws (e.g. an invalid action fails).
TEST(game_event_dispatcher_t_test, remains_usable_after_an_event_throws)
{
    game_event_dispatcher_t dispatcher;
    auto remaining_event_executed = false;

    dispatcher.enqueue([]() { throw std::runtime_error("Expected test exception."); });
    dispatcher.enqueue([&remaining_event_executed]() { remaining_event_executed = true; });

    EXPECT_THROW(dispatcher.dispatch_pending(), std::runtime_error);
    EXPECT_FALSE(remaining_event_executed);

    EXPECT_NO_THROW(dispatcher.dispatch_pending());
    EXPECT_TRUE(remaining_event_executed);
}
