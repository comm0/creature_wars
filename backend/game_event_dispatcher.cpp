#include "game_event_dispatcher.h"

#include <stdexcept>
#include <utility>

/*!
    \class game_event_dispatcher_t
    \inmodule CreatureWars
    \brief Processes game events in FIFO order.
*/

/*! Adds \a event_p to the end of the queue. */
void game_event_dispatcher_t::enqueue(std::function<void()> event_p)
{
    if (!event_p) {
        throw std::invalid_argument("Game event must be callable.");
    }

    events_.push_back(std::move(event_p));
}

/*! Processes queued events in arrival order. */
void game_event_dispatcher_t::dispatch_pending()
{
    if (dispatching_) {
        return;
    }

    dispatching_ = true;

    try {
        while (!events_.empty()) {
            auto event = std::move(events_.front());
            events_.pop_front();
            event();
        }
    } catch (...) {
        dispatching_ = false;
        throw;
    }

    dispatching_ = false;
}
