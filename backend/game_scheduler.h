#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

class game_scheduler_t
{
public:
    using time_point_t = std::chrono::steady_clock::time_point;
    using event_id_t = std::uint64_t;
    using event_t = std::function<void(time_point_t)>;

    event_id_t schedule(time_point_t time_p, event_t event_p);
    bool cancel(event_id_t id_p);
    void clear() noexcept;
    void delay_all(std::chrono::steady_clock::duration delay_p);
    std::optional<time_point_t> next_time() const noexcept;
    std::optional<time_point_t> time_of(event_id_t id_p) const noexcept;
    std::vector<std::function<void()>> take_due(time_point_t now_p);

    std::size_t size() const noexcept
    {
        return events_.size();
    }

private:
    struct event_key_t
    {
        time_point_t time_;
        event_id_t id_;

        bool operator<(const event_key_t& other_p) const noexcept
        {
            return time_ != other_p.time_ ? time_ < other_p.time_ : id_ < other_p.id_;
        }
    };

    std::map<event_key_t, event_t> events_;
    std::unordered_map<event_id_t, time_point_t> times_by_id_;
    event_id_t next_id_ = 1;
};
