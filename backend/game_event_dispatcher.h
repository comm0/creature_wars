#pragma once

#include <deque>
#include <functional>

class game_event_dispatcher_t
{
public:
    void enqueue(std::function<void()> event_p);
    void dispatch_pending();

private:
    std::deque<std::function<void()>> events_;
    bool dispatching_ = false;
};
