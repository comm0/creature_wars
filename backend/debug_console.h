#pragma once

#include <chrono>
#include <cstddef>

struct debug_monitor_sample_t
{
    std::chrono::microseconds tick_duration_{};
    std::size_t creature_count_ = 0;
    std::size_t occupied_position_count_ = 0;
    std::size_t position_count_ = 0;
    std::size_t pending_event_count_ = 0;
    std::size_t peak_pending_event_count_ = 0;
    std::size_t dispatched_event_count_ = 0;
};

namespace debug_console
{
void initialize();
void shutdown();
void print_message(const char* message_p);
void print_warning(const char* message_p);
void update_monitor(const debug_monitor_sample_t& sample_p);
}
