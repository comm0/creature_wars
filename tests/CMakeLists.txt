include(FetchContent)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.18.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(googletest)

add_executable(game_event_dispatcher_tests
    game_event_dispatcher_test.cpp
)

target_link_libraries(game_event_dispatcher_tests
    PRIVATE
        creature_wars_core
        creature_wars_warnings
        GTest::gtest_main
)

add_test(
    NAME game_event_dispatcher_tests
    COMMAND game_event_dispatcher_tests
)
