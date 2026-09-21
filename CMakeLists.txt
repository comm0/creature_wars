cmake_minimum_required(VERSION 3.16)

project(creature_wars VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Felgo REQUIRED)
find_package(Threads REQUIRED)

set(PRODUCT_IDENTIFIER "com.example.creaturewars")
set(PRODUCT_VERSION_NAME "0.1.0")
set(PRODUCT_VERSION_CODE 1)
set(PRODUCT_STAGE "test")
set(PRODUCT_LICENSE_KEY "")

file(GLOB_RECURSE QmlFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} qml/*.qml qml/*.js)

set(BackendFiles
    backend/CreaturesModel.h
    backend/CreaturesModel.cpp
    backend/GameBackend.h
    backend/GameBackend.cpp
    backend/creature.h
    backend/creature.cpp
    backend/creatures.h
    backend/creatures.cpp
    backend/game_constants.h
    backend/game.h
    backend/game.cpp
    backend/game_event_dispatcher.h
    backend/game_event_dispatcher.cpp
)

qt_add_executable(appCreatureWars
    main.cpp
    ${BackendFiles}
    ${QmlFiles}
)

felgo_configure_executable(appCreatureWars)

deploy_resources("${QmlFiles}")

qt_add_qml_module(appCreatureWars
    URI CreatureWars
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
)

target_compile_definitions(appCreatureWars
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(appCreatureWars PRIVATE Felgo Threads::Threads)

find_program(QDOC_EXECUTABLE qdoc)

if(QDOC_EXECUTABLE)
    add_custom_target(docs
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/docs/html
        COMMAND ${QDOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/creature_wars.qdocconf
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
    )
endif()
