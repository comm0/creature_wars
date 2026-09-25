#include "game_scheduler.h"

#include <stdexcept>
#include <utility>

/*!
    \class game_scheduler_t
    \inmodule CreatureWars
    \brief Holds events planned for a point in time, in time and scheduling order.
*/

game_scheduler_t::event_id_t game_scheduler_t::schedule(
    time_point_t time_p,
    event_t event_p
)
{
    if (!event_p) {
        throw std::invalid_argument("Scheduled event must be callable.");
    }

    const auto id = next_id_++;
    events_.emplace(event_key_t{time_p, id}, std::move(event_p));
    times_by_id_.emplace(id, time_p);
    return id;
}

bool game_scheduler_t::cancel(event_id_t id_p)
{
    const auto time = times_by_id_.find(id_p);

    if (time == times_by_id_.end()) {
        return false;
    }

    events_.erase(event_key_t{time->second, id_p});
    times_by_id_.erase(time);
    return true;
}

void game_scheduler_t::clear() noexcept
{
    events_.clear();
    times_by_id_.clear();
}

void game_scheduler_t::delay_all(std::chrono::steady_clock::duration delay_p)
{
    std::map<event_key_t, event_t> delayed_events;

    for (auto& [key, event] : events_) {
        delayed_events.emplace(event_key_t{key.time_ + delay_p, key.id_}, std::move(event));
        times_by_id_[key.id_] = key.time_ + delay_p;
    }

    events_ = std::move(delayed_events);
}

std::optional<game_scheduler_t::time_point_t> game_scheduler_t::next_time() const noexcept
{
    if (events_.empty()) {
        return std::nullopt;
    }

    return events_.begin()->first.time_;
}

std::vector<std::function<void()>> game_scheduler_t::take_due(time_point_t now_p)
{
    std::vector<std::function<void()>> due_events;

    while (!events_.empty() && events_.begin()->first.time_ <= now_p) {
        auto event = events_.extract(events_.begin());
        const auto time = event.key().time_;
        times_by_id_.erase(event.key().id_);
        due_events.push_back([due_event = std::move(event.mapped()), time]() {
            due_event(time);
        });
    }

    return due_events;
}
