#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <stdexcept>
#include <vector>

#include "game_scheduler.h"

namespace
{
const auto start_time = std::chrono::steady_clock::time_point{};

game_scheduler_t::time_point_t at(int milliseconds_p)
{
    return start_time + std::chrono::milliseconds{milliseconds_p};
}

void run_due(game_scheduler_t& scheduler_p, int milliseconds_p)
{
    for (auto& event : scheduler_p.take_due(at(milliseconds_p))) {
        event();
    }
}
}

// Verifies that an empty scheduler has nothing to wait for (e.g. a fresh game).
TEST(game_scheduler_t_test, has_no_next_time_when_empty)
{
    game_scheduler_t scheduler;

    EXPECT_FALSE(scheduler.next_time().has_value());
    EXPECT_TRUE(scheduler.take_due(at(1000)).empty());
}

// Verifies that events run in time order, not scheduling order (e.g. research before spawn).
TEST(game_scheduler_t_test, runs_events_in_time_order)
{
    game_scheduler_t scheduler;
    std::vector<int> execution_order;

    scheduler.schedule(at(300), [&execution_order](auto) { execution_order.push_back(3); });
    scheduler.schedule(at(100), [&execution_order](auto) { execution_order.push_back(1); });
    scheduler.schedule(at(200), [&execution_order](auto) { execution_order.push_back(2); });

    EXPECT_EQ(scheduler.next_time(), at(100));

    run_due(scheduler, 300);

    EXPECT_EQ(execution_order, (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(scheduler.size(), 0U);
}

// Verifies that events due at the same time keep their scheduling order (e.g. queued spawns).
TEST(game_scheduler_t_test, keeps_scheduling_order_for_equal_times)
{
    game_scheduler_t scheduler;
    std::vector<int> execution_order;

    scheduler.schedule(at(100), [&execution_order](auto) { execution_order.push_back(1); });
    scheduler.schedule(at(100), [&execution_order](auto) { execution_order.push_back(2); });

    run_due(scheduler, 100);

    EXPECT_EQ(execution_order, (std::vector<int>{1, 2}));
}

// Verifies that only events whose time has come are taken (e.g. a long research keeps waiting).
TEST(game_scheduler_t_test, takes_only_due_events)
{
    game_scheduler_t scheduler;
    std::vector<int> execution_order;

    scheduler.schedule(at(100), [&execution_order](auto) { execution_order.push_back(1); });
    scheduler.schedule(at(500), [&execution_order](auto) { execution_order.push_back(2); });

    run_due(scheduler, 200);

    EXPECT_EQ(execution_order, (std::vector<int>{1}));
    EXPECT_EQ(scheduler.next_time(), at(500));
}

// Verifies that a cancelled event never runs (e.g. a cancelled spawn in the queue).
TEST(game_scheduler_t_test, does_not_run_cancelled_event)
{
    game_scheduler_t scheduler;
    std::vector<int> execution_order;

    const auto cancelled_id = scheduler.schedule(
        at(100),
        [&execution_order](auto) { execution_order.push_back(1); }
    );
    scheduler.schedule(at(200), [&execution_order](auto) { execution_order.push_back(2); });

    EXPECT_TRUE(scheduler.cancel(cancelled_id));
    EXPECT_FALSE(scheduler.cancel(cancelled_id));

    run_due(scheduler, 200);

    EXPECT_EQ(execution_order, (std::vector<int>{2}));
}

// Verifies that clearing drops all planned events (e.g. starting a new match).
TEST(game_scheduler_t_test, clears_all_events)
{
    game_scheduler_t scheduler;

    scheduler.schedule(at(100), [](auto) {});
    scheduler.schedule(at(200), [](auto) {});
    scheduler.clear();

    EXPECT_EQ(scheduler.size(), 0U);
    EXPECT_FALSE(scheduler.next_time().has_value());
}

// Verifies that delaying keeps order and cancel ids (e.g. resuming a paused game).
TEST(game_scheduler_t_test, delays_all_events)
{
    game_scheduler_t scheduler;
    std::vector<int> execution_order;

    scheduler.schedule(at(100), [&execution_order](auto) { execution_order.push_back(1); });
    const auto cancelled_id = scheduler.schedule(
        at(200),
        [&execution_order](auto) { execution_order.push_back(2); }
    );

    scheduler.delay_all(std::chrono::milliseconds{1000});

    EXPECT_EQ(scheduler.next_time(), at(1100));
    EXPECT_TRUE(scheduler.cancel(cancelled_id));

    run_due(scheduler, 1000);
    EXPECT_TRUE(execution_order.empty());

    run_due(scheduler, 1100);
    EXPECT_EQ(execution_order, (std::vector<int>{1}));
}

// Verifies that an event learns its delayed time (e.g. income continues after a pause).
TEST(game_scheduler_t_test, passes_delayed_time_to_event)
{
    game_scheduler_t scheduler;
    game_scheduler_t::time_point_t received_time;

    scheduler.schedule(at(100), [&received_time](auto time_p) { received_time = time_p; });
    scheduler.delay_all(std::chrono::milliseconds{500});
    run_due(scheduler, 600);

    EXPECT_EQ(received_time, at(600));
}

// Verifies that an empty event is rejected (e.g. a missing handler).
TEST(game_scheduler_t_test, rejects_empty_event)
{
    game_scheduler_t scheduler;

    EXPECT_THROW(scheduler.schedule(at(100), {}), std::invalid_argument);
}
