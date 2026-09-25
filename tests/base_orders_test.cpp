#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <vector>

#include "base_orders.h"

namespace
{
const auto start_time = std::chrono::steady_clock::time_point{};

game_scheduler_t::time_point_t at(int milliseconds_p)
{
    return start_time + std::chrono::milliseconds{milliseconds_p};
}

void run_until(game_scheduler_t& scheduler_p, int milliseconds_p)
{
    for (
        auto events = scheduler_p.take_due(at(milliseconds_p));
        !events.empty();
        events = scheduler_p.take_due(at(milliseconds_p))
    ) {
        for (auto& event : events) {
            event();
        }
    }
}

struct orders_fixture_t
{
    game_scheduler_t scheduler;
    std::vector<std::string> completed;
    bool accept = true;
    int changes = 0;
    base_orders_t orders{
        scheduler,
        [this](const std::string& key_p) {
            if (accept) {
                completed.push_back(key_p);
            }

            return accept;
        },
        [this]() { ++changes; }
    };
};

const auto five_seconds = std::chrono::milliseconds{5000};
}

// Verifies that queued orders finish one after another (e.g. three orcs bought at once).
TEST(base_orders_t_test, completes_queue_in_sequence)
{
    orders_fixture_t fixture;

    fixture.orders.enqueue("spawn:orc", five_seconds, at(0));
    fixture.orders.enqueue("spawn:orc", five_seconds, at(0));
    fixture.orders.enqueue("spawn:orc", five_seconds, at(0));

    EXPECT_EQ(fixture.orders.progress("spawn:orc").queued_, 3);
    EXPECT_EQ(fixture.orders.progress("spawn:orc").finish_time_, at(5000));

    run_until(fixture.scheduler, 9999);
    EXPECT_EQ(fixture.completed.size(), 1U);
    EXPECT_EQ(fixture.orders.progress("spawn:orc").finish_time_, at(10000));

    run_until(fixture.scheduler, 15000);
    EXPECT_EQ(fixture.completed.size(), 3U);
    EXPECT_EQ(fixture.orders.progress("spawn:orc").queued_, 0);
    EXPECT_FALSE(fixture.orders.progress("spawn:orc").finish_time_.has_value());
}

// Verifies that different order keys run in parallel (e.g. a research during a spawn).
TEST(base_orders_t_test, runs_different_keys_in_parallel)
{
    orders_fixture_t fixture;

    fixture.orders.enqueue("spawn:orc", five_seconds, at(0));
    fixture.orders.enqueue("research:long_range", std::chrono::milliseconds{3000}, at(0));

    run_until(fixture.scheduler, 5000);

    EXPECT_EQ(
        fixture.completed,
        (std::vector<std::string>{"research:long_range", "spawn:orc"})
    );
}

// Verifies that a refused completion is retried later (e.g. no free tile around the base).
TEST(base_orders_t_test, retries_refused_completion)
{
    orders_fixture_t fixture;
    fixture.accept = false;

    fixture.orders.enqueue("spawn:orc", five_seconds, at(0));
    run_until(fixture.scheduler, 5000);

    EXPECT_EQ(fixture.orders.progress("spawn:orc").queued_, 1);
    EXPECT_EQ(
        fixture.orders.progress("spawn:orc").finish_time_,
        at(5000) + base_orders_t::retry_interval
    );

    fixture.accept = true;
    run_until(fixture.scheduler, 6000);

    EXPECT_EQ(fixture.completed, (std::vector<std::string>{"spawn:orc"}));
    EXPECT_EQ(fixture.orders.progress("spawn:orc").queued_, 0);
}

// Verifies that clearing drops pending orders (e.g. a new match starts).
TEST(base_orders_t_test, clear_cancels_pending_orders)
{
    orders_fixture_t fixture;

    fixture.orders.enqueue("upgrade", five_seconds, at(0));
    fixture.orders.clear();
    run_until(fixture.scheduler, 10000);

    EXPECT_TRUE(fixture.completed.empty());
    EXPECT_EQ(fixture.orders.progress("upgrade").queued_, 0);
}

// Verifies that listeners hear about enqueued and finished orders (e.g. the base menu refreshes).
TEST(base_orders_t_test, reports_changes)
{
    orders_fixture_t fixture;

    fixture.orders.enqueue("upgrade", five_seconds, at(0));
    EXPECT_EQ(fixture.changes, 1);

    run_until(fixture.scheduler, 5000);
    EXPECT_EQ(fixture.changes, 2);
}
