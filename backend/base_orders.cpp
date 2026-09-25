#include "base_orders.h"

#include <utility>

/*!
    \class base_orders_t
    \inmodule CreatureWars
    \brief Runs timed base orders; each order key has its own queue.
*/

base_orders_t::base_orders_t(
    game_scheduler_t& scheduler_p,
    completion_t completion_p,
    change_t change_p
)
    : scheduler_(scheduler_p)
    , completion_(std::move(completion_p))
    , change_(std::move(change_p))
{
}

void base_orders_t::enqueue(
    const std::string& key_p,
    std::chrono::milliseconds duration_p,
    game_scheduler_t::time_point_t now_p
)
{
    auto& queue = queues_[key_p];
    queue.duration_ = duration_p;
    ++queue.queued_;

    if (!queue.event_.has_value()) {
        schedule_completion(key_p, now_p + duration_p);
    }

    change_();
}

order_progress_t base_orders_t::progress(const std::string& key_p) const
{
    const auto queue = queues_.find(key_p);

    if (queue == queues_.end()) {
        return {0, std::chrono::milliseconds{0}, std::nullopt};
    }

    std::optional<game_scheduler_t::time_point_t> finish_time;

    if (queue->second.event_.has_value()) {
        finish_time = scheduler_.time_of(queue->second.event_.value());
    }

    return {queue->second.queued_, queue->second.duration_, finish_time};
}

void base_orders_t::clear()
{
    for (const auto& [key, queue] : queues_) {
        if (queue.event_.has_value()) {
            scheduler_.cancel(*queue.event_);
        }
    }

    queues_.clear();
}

void base_orders_t::schedule_completion(
    const std::string& key_p,
    game_scheduler_t::time_point_t time_p
)
{
    queues_[key_p].event_ = scheduler_.schedule(
        time_p,
        [this, key_p](game_scheduler_t::time_point_t scheduled_time_p) {
            complete(key_p, scheduled_time_p);
        }
    );
}

void base_orders_t::complete(
    const std::string& key_p,
    game_scheduler_t::time_point_t time_p
)
{
    auto& queue = queues_[key_p];
    queue.event_.reset();

    if (!completion_(key_p)) {
        schedule_completion(key_p, time_p + retry_interval);
        return;
    }

    --queue.queued_;

    if (queue.queued_ > 0) {
        schedule_completion(key_p, time_p + queue.duration_);
    }

    change_();
}
