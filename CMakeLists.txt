cmake_minimum_required(VERSION 3.16)

project(creature_wars VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CREATURE_WARS_BUILD_APP "Build the Felgo application." ON)
option(CREATURE_WARS_WARNINGS_AS_ERRORS "Treat project warnings as errors." OFF)
option(CREATURE_WARS_ENABLE_CLANG_TIDY "Run clang-tidy while building project targets." OFF)
option(CREATURE_WARS_ENABLE_CPPCHECK "Run cppcheck while building project targets." OFF)
option(CREATURE_WARS_USE_CCACHE "Use ccache when available." ON)
option(CREATURE_WARS_USE_LLD "Link with lld when available." ON)

if(CREATURE_WARS_ENABLE_CLANG_TIDY OR CREATURE_WARS_ENABLE_CPPCHECK)
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)
endif()

if(CREATURE_WARS_USE_CCACHE AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
    find_program(CCACHE_EXECUTABLE ccache)

    if(CCACHE_EXECUTABLE)
        set(CMAKE_CXX_COMPILER_LAUNCHER
            ${CMAKE_COMMAND} -E env CCACHE_SLOPPINESS=pch_defines,time_macros ${CCACHE_EXECUTABLE}
        )

        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-fpch-preprocess)
        endif()
    endif()
endif()

if(CREATURE_WARS_USE_LLD AND NOT MSVC)
    find_program(LLD_EXECUTABLE ld.lld)

    if(LLD_EXECUTABLE)
        add_link_options(-fuse-ld=lld)
    endif()
endif()

set(CREATURE_WARS_MAP_SIZE "64x32" CACHE STRING "Map size preset.")
set_property(CACHE CREATURE_WARS_MAP_SIZE PROPERTY STRINGS
    "40x20"
    "48x24"
    "56x28"
    "64x32"
)

if(CREATURE_WARS_MAP_SIZE STREQUAL "40x20")
    set(CREATURE_WARS_MAP_COLUMN_COUNT 40)
    set(CREATURE_WARS_MAP_ROW_COUNT 20)
elseif(CREATURE_WARS_MAP_SIZE STREQUAL "48x24")
    set(CREATURE_WARS_MAP_COLUMN_COUNT 48)
    set(CREATURE_WARS_MAP_ROW_COUNT 24)
elseif(CREATURE_WARS_MAP_SIZE STREQUAL "56x28")
    set(CREATURE_WARS_MAP_COLUMN_COUNT 56)
    set(CREATURE_WARS_MAP_ROW_COUNT 28)
elseif(CREATURE_WARS_MAP_SIZE STREQUAL "64x32")
    set(CREATURE_WARS_MAP_COLUMN_COUNT 64)
    set(CREATURE_WARS_MAP_ROW_COUNT 32)
else()
    message(FATAL_ERROR "Unsupported map size: ${CREATURE_WARS_MAP_SIZE}")
endif()

include(CTest)
include(FetchContent)
find_package(Threads REQUIRED)

set(JSON_BuildTests OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

add_library(creature_wars_warnings INTERFACE)

if(MSVC)
    target_compile_options(creature_wars_warnings INTERFACE /W4)

    if(CREATURE_WARS_WARNINGS_AS_ERRORS)
        target_compile_options(creature_wars_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(creature_wars_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
    )

    if(CREATURE_WARS_WARNINGS_AS_ERRORS)
        target_compile_options(creature_wars_warnings INTERFACE -Werror)
    endif()
endif()

set(CoreFiles
    backend/ai_profile.h
    backend/ai_profile.cpp
    backend/ai_planner.h
    backend/ai_planner.cpp
    backend/debug_console.h
    backend/debug_console.cpp
    backend/creature.h
    backend/creature.cpp
    backend/creature_type.h
    backend/creature_type.cpp
    backend/creature_type_registry.h
    backend/creature_type_registry.cpp
    backend/type_parsing.h
    backend/type_parsing.cpp
    backend/economy_settings.h
    backend/economy_settings.cpp
    backend/resource_cost.h
    backend/resource_reward.h
    backend/damage_contributions.h
    backend/tech_tree.h
    backend/tech_tree.cpp
    backend/base_action.h
    backend/base_controller.h
    backend/base_orders.h
    backend/base_orders.cpp
    backend/base_type.h
    backend/base_type_registry.h
    backend/base_type_registry.cpp
    backend/base.h
    backend/base.cpp
    backend/creatures.h
    backend/creatures.cpp
    backend/game_constants.h
    backend/game_map.h
    backend/game_map.cpp
    backend/pathfinder.h
    backend/pathfinder.cpp
    backend/visibility_system.h
    backend/visibility_system.cpp
    backend/game_observer.h
    backend/player_state.h
    backend/game_clock.h
    backend/game_clock.cpp
    backend/game.h
    backend/game.cpp
    backend/game_event_dispatcher.h
    backend/game_event_dispatcher.cpp
    backend/game_scheduler.h
    backend/game_scheduler.cpp
    backend/wildlife_spawner.h
    backend/wildlife_spawner.cpp
)

add_library(creature_wars_core STATIC ${CoreFiles})

target_include_directories(creature_wars_core PUBLIC backend)
target_compile_definitions(creature_wars_core PUBLIC
    CREATURE_WARS_MAP_COLUMN_COUNT=${CREATURE_WARS_MAP_COLUMN_COUNT}
    CREATURE_WARS_MAP_ROW_COUNT=${CREATURE_WARS_MAP_ROW_COUNT}
)
target_link_libraries(creature_wars_core
    PUBLIC Threads::Threads
    PRIVATE
        creature_wars_warnings
        nlohmann_json::nlohmann_json
)

if(WIN32)
    target_link_libraries(creature_wars_core PRIVATE $<$<CONFIG:Debug>:psapi>)
endif()

target_precompile_headers(creature_wars_core PRIVATE
    <algorithm>
    <chrono>
    <cstdint>
    <functional>
    <optional>
    <string>
    <string_view>
    <unordered_map>
    <utility>
    <vector>
    <nlohmann/json.hpp>
)

if(CREATURE_WARS_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE clang-tidy)

    if(NOT CLANG_TIDY_EXECUTABLE)
        message(FATAL_ERROR "clang-tidy was requested but was not found.")
    endif()

    set_target_properties(creature_wars_core PROPERTIES
        CXX_CLANG_TIDY
            "${CLANG_TIDY_EXECUTABLE};--checks=-*,clang-analyzer-*,bugprone-*,performance-*;--warnings-as-errors=*"
    )
endif()

if(CREATURE_WARS_ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXECUTABLE cppcheck)

    if(NOT CPPCHECK_EXECUTABLE)
        message(FATAL_ERROR "cppcheck was requested but was not found.")
    endif()

    set_target_properties(creature_wars_core PROPERTIES
        CXX_CPPCHECK
            "${CPPCHECK_EXECUTABLE};--enable=warning,performance,portability;--std=c++20;--suppress=missingIncludeSystem;--suppress=syntaxError:*/nlohmann/*;--error-exitcode=1"
    )
endif()

if(CREATURE_WARS_BUILD_APP)
    set(CMAKE_AUTOMOC ON)

    find_package(Felgo REQUIRED)
    find_package(Git QUIET)
    qt_policy(SET QTP0004 NEW)

    set(CREATURE_WARS_BUILD_BRANCH "unknown")

    if(Git_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --path-format=absolute --git-path HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE CREATURE_WARS_GIT_HEAD
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        if(CREATURE_WARS_GIT_HEAD)
            set_property(
                DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                ${CREATURE_WARS_GIT_HEAD}
            )
        endif()

        execute_process(
            COMMAND ${GIT_EXECUTABLE} branch --show-current
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE CREATURE_WARS_BUILD_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        if(NOT CREATURE_WARS_BUILD_BRANCH)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE CREATURE_WARS_BUILD_COMMIT
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            set(CREATURE_WARS_BUILD_BRANCH "detached@${CREATURE_WARS_BUILD_COMMIT}")
        endif()
    endif()

    set(PRODUCT_IDENTIFIER "com.example.creaturewars")
    set(PRODUCT_VERSION_NAME "0.1.0")
    set(PRODUCT_VERSION_CODE 1)
    set(PRODUCT_STAGE "test")
    set(PRODUCT_LICENSE_KEY "")

    file(
        GLOB_RECURSE QmlFiles
        CONFIGURE_DEPENDS
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        qml/*.qml
        qml/*.js
    )
    file(
        GLOB_RECURSE AssetFiles
        CONFIGURE_DEPENDS
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        assets/*.png
        assets/*.json
    )

    set(ApplicationFiles
        backend/BaseActionsModel.h
        backend/BaseActionsModel.cpp
        backend/BasesModel.h
        backend/BasesModel.cpp
        backend/CreaturesModel.h
        backend/CreaturesModel.cpp
        backend/CorpsesModel.h
        backend/CorpsesModel.cpp
        backend/GameBackend.h
        backend/GameBackend.cpp
        backend/GameObserver.h
        backend/GameObserver.cpp
    )

    qt_add_executable(appCreatureWars
        main.cpp
        ${ApplicationFiles}
        ${QmlFiles}
    )

    qt_add_resources(appCreatureWars creature_wars_data
        PREFIX "/"
        FILES
            data/creature_types.json
            data/base_types.json
            data/economy.json
            data/tech_tree.json
            data/ai_profiles.json
            ${AssetFiles}
    )

    find_package(Qt6 REQUIRED COMPONENTS ShaderTools)

    qt_add_shaders(appCreatureWars creature_wars_shaders
        PREFIX "/"
        FILES
            shaders/outline.frag
            shaders/tint.frag
    )

    felgo_configure_executable(appCreatureWars)

    deploy_resources("${QmlFiles}")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(appCreatureWars PROPERTIES
            QT_QMLCACHEGEN_ARGUMENTS "--only-bytecode"
        )
    endif()

    qt_add_qml_module(appCreatureWars
        URI CreatureWars
        VERSION 1.0
        NO_RESOURCE_TARGET_PATH
        QML_FILES ${QmlFiles}
    )

    target_compile_definitions(appCreatureWars
        PRIVATE
            $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
            CREATURE_WARS_BUILD_BRANCH="${CREATURE_WARS_BUILD_BRANCH}"
    )

    target_precompile_headers(appCreatureWars PRIVATE
        <QAbstractListModel>
        <QColor>
        <QHash>
        <QObject>
        <QString>
        <QVariant>
        <QtQml/qqml.h>
    )

    target_link_libraries(appCreatureWars
        PRIVATE
            creature_wars_core
            creature_wars_warnings
            Felgo
    )

    find_program(QDOC_EXECUTABLE qdoc)

    if(QDOC_EXECUTABLE)
        add_custom_target(docs
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/docs/html
            COMMAND ${QDOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/creature_wars.qdocconf
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
        )
    endif()
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
