cmake_minimum_required(VERSION 3.16)

project(creature_wars VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CREATURE_WARS_BUILD_APP "Build the Felgo application." ON)
option(CREATURE_WARS_WARNINGS_AS_ERRORS "Treat project warnings as errors." OFF)
option(CREATURE_WARS_ENABLE_CLANG_TIDY "Run clang-tidy while building project targets." OFF)
option(CREATURE_WARS_ENABLE_CPPCHECK "Run cppcheck while building project targets." OFF)

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
    backend/debug_console.h
    backend/debug_console.cpp
    backend/creature.h
    backend/creature.cpp
    backend/creature_type.h
    backend/creature_type.cpp
    backend/creature_type_registry.h
    backend/creature_type_registry.cpp
    backend/creatures.h
    backend/creatures.cpp
    backend/game_constants.h
    backend/game_map.h
    backend/game_map.cpp
    backend/game.h
    backend/game.cpp
    backend/game_event_dispatcher.h
    backend/game_event_dispatcher.cpp
)

add_library(creature_wars_core STATIC ${CoreFiles})

target_include_directories(creature_wars_core PUBLIC backend)
target_link_libraries(creature_wars_core
    PUBLIC Threads::Threads
    PRIVATE
        creature_wars_warnings
        nlohmann_json::nlohmann_json
)

if(WIN32)
    target_link_libraries(creature_wars_core PRIVATE $<$<CONFIG:Debug>:psapi>)
endif()

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
            "${CPPCHECK_EXECUTABLE};--enable=warning,performance,portability;--std=c++20;--suppress=missingIncludeSystem;--error-exitcode=1"
    )
endif()

if(CREATURE_WARS_BUILD_APP)
    set(CMAKE_AUTOMOC ON)

    find_package(Felgo REQUIRED)
    qt_policy(SET QTP0004 NEW)

    set(PRODUCT_IDENTIFIER "com.example.creaturewars")
    set(PRODUCT_VERSION_NAME "0.1.0")
    set(PRODUCT_VERSION_CODE 1)
    set(PRODUCT_STAGE "test")
    set(PRODUCT_LICENSE_KEY "")

    file(GLOB_RECURSE QmlFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} qml/*.qml qml/*.js)

    set(ApplicationFiles
        backend/CreaturesModel.h
        backend/CreaturesModel.cpp
        backend/GameBackend.h
        backend/GameBackend.cpp
    )

    qt_add_executable(appCreatureWars
        main.cpp
        ${ApplicationFiles}
        ${QmlFiles}
    )

    qt_add_resources(appCreatureWars creature_wars_data
        PREFIX "/"
        FILES data/creature_types.json
    )

    felgo_configure_executable(appCreatureWars)

    deploy_resources("${QmlFiles}")

    qt_add_qml_module(appCreatureWars
        URI CreatureWars
        VERSION 1.0
        NO_RESOURCE_TARGET_PATH
        QML_FILES ${QmlFiles}
    )

    target_compile_definitions(appCreatureWars
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
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
