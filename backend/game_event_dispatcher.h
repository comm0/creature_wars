#pragma once

#include <cstddef>
#include <deque>
#include <functional>

class game_event_dispatcher_t
{
public:
    void enqueue(std::function<void()> event_p);
    void dispatch_pending();

#ifndef NDEBUG
    std::size_t pending_event_count() const noexcept
    {
        return events_.size();
    }

    std::size_t peak_pending_event_count() const noexcept
    {
        return peak_pending_event_count_;
    }

    std::size_t dispatched_event_count() const noexcept
    {
        return dispatched_event_count_;
    }
#endif

private:
    std::deque<std::function<void()>> events_;
#ifndef NDEBUG
    std::size_t peak_pending_event_count_ = 0;
    std::size_t dispatched_event_count_ = 0;
#endif
    bool dispatching_ = false;
};
