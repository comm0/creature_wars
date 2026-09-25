#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <optional>
#include <string>

#include "game_scheduler.h"

struct order_progress_t
{
    int queued_;
    std::chrono::milliseconds duration_;
    std::optional<game_scheduler_t::time_point_t> finish_time_;
};

class base_orders_t
{
public:
    using completion_t = std::function<bool(const std::string&)>;
    using change_t = std::function<void()>;

    static constexpr std::chrono::milliseconds retry_interval{1000};

    base_orders_t(
        game_scheduler_t& scheduler_p,
        completion_t completion_p,
        change_t change_p
    );

    void enqueue(
        const std::string& key_p,
        std::chrono::milliseconds duration_p,
        game_scheduler_t::time_point_t now_p
    );
    order_progress_t progress(const std::string& key_p) const;
    void clear();

private:
    struct order_queue_t
    {
        int queued_ = 0;
        std::chrono::milliseconds duration_{0};
        std::optional<game_scheduler_t::event_id_t> event_;
    };

    void schedule_completion(
        const std::string& key_p,
        game_scheduler_t::time_point_t time_p
    );
    void complete(const std::string& key_p, game_scheduler_t::time_point_t time_p);

    game_scheduler_t& scheduler_;
    completion_t completion_;
    change_t change_;
    std::map<std::string, order_queue_t> queues_;
};
