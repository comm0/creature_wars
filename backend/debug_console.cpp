#include "debug_console.h"

#ifndef NDEBUG

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <Psapi.h>

#include <cstdio>
#endif

namespace
{
std::size_t monitor_width = 0;
std::mutex output_mutex;

#ifdef _WIN32
std::uint64_t file_time_value(FILETIME time_p)
{
    ULARGE_INTEGER value;
    value.LowPart = time_p.dwLowDateTime;
    value.HighPart = time_p.dwHighDateTime;
    return value.QuadPart;
}

double process_cpu_usage()
{
    static auto previous_wall_time = std::chrono::steady_clock::now();
    static std::uint64_t previous_process_time = 0;

    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;

    if (GetProcessTimes(
        GetCurrentProcess(),
        &creation_time,
        &exit_time,
        &kernel_time,
        &user_time
    ) == 0) {
        return 0.0;
    }

    const auto wall_time = std::chrono::steady_clock::now();
    const auto process_time = file_time_value(kernel_time) + file_time_value(user_time);

    if (previous_process_time == 0) {
        previous_process_time = process_time;
        previous_wall_time = wall_time;
        return 0.0;
    }

    const auto process_delta = process_time - previous_process_time;
    const auto wall_delta = std::chrono::duration<double>(
        wall_time - previous_wall_time
    ).count();
    const auto processor_count = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);

    previous_process_time = process_time;
    previous_wall_time = wall_time;

    if (wall_delta <= 0.0 || processor_count == 0) {
        return 0.0;
    }

    constexpr auto file_time_units_per_second = 10'000'000.0;
    return static_cast<double>(process_delta)
        / file_time_units_per_second
        / wall_delta
        / static_cast<double>(processor_count)
        * 100.0;
}

double process_memory_megabytes()
{
    PROCESS_MEMORY_COUNTERS_EX counters{};

    if (GetProcessMemoryInfo(
        GetCurrentProcess(),
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),
        sizeof(counters)
    ) == 0) {
        return 0.0;
    }

    constexpr auto bytes_per_megabyte = 1024.0 * 1024.0;
    return static_cast<double>(counters.WorkingSetSize) / bytes_per_megabyte;
}
#endif

void clear_monitor()
{
    if (monitor_width == 0) {
        return;
    }

    std::cerr << '\r' << std::string(monitor_width, ' ') << '\r';
    monitor_width = 0;
}
}

void debug_console::initialize()
{
#ifdef _WIN32
    if (AllocConsole() == 0) {
        return;
    }

    FILE* error_stream = nullptr;
    freopen_s(&error_stream, "CONOUT$", "w", stderr);
    std::cerr.clear();
    SetConsoleTitleW(L"Creature Wars Debug");
#endif
}

void debug_console::shutdown()
{
    std::lock_guard<std::mutex> lock(output_mutex);
    clear_monitor();
    std::cerr << '\n';
}

void debug_console::print_message(const char* message_p)
{
    std::lock_guard<std::mutex> lock(output_mutex);
    clear_monitor();
    std::cerr << message_p << '\n';
}

void debug_console::print_warning(const char* message_p)
{
    std::lock_guard<std::mutex> lock(output_mutex);
    clear_monitor();
    std::cerr << "[warning] " << message_p << '\n';
}

void debug_console::update_monitor(const debug_monitor_sample_t& sample_p)
{
    std::lock_guard<std::mutex> lock(output_mutex);

    if (monitor_width == 0) {
        std::cerr << '\n';
    }

    std::ostringstream output;
    output << std::fixed << std::setprecision(2);

#ifdef _WIN32
    output << "CPU " << process_cpu_usage() << "%"
        << " | RAM " << process_memory_megabytes() << " MB | ";
#endif

    output << "tick "
        << std::chrono::duration<double, std::milli>(sample_p.tick_duration_).count()
        << " ms"
        << " | creatures " << sample_p.creature_count_
        << " | map " << sample_p.occupied_position_count_
        << '/' << sample_p.position_count_
        << " | dispatcher " << sample_p.pending_event_count_ << " queued"
        << ", " << sample_p.peak_pending_event_count_ << " peak"
        << ", " << sample_p.dispatched_event_count_ << " total";

    const auto message = output.str();
    const auto padding = monitor_width > message.size()
        ? monitor_width - message.size()
        : 0;

    std::cerr << '\r' << message << std::string(padding, ' ') << std::flush;
    monitor_width = message.size();
}

#else

void debug_console::initialize()
{
}

void debug_console::shutdown()
{
}

void debug_console::print_message(const char* message_p)
{
    static_cast<void>(message_p);
}

void debug_console::print_warning(const char* message_p)
{
    static_cast<void>(message_p);
}

void debug_console::update_monitor(const debug_monitor_sample_t& sample_p)
{
    static_cast<void>(sample_p);
}

#endif
