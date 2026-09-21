cmake_minimum_required(VERSION 3.16)

project(creature_wars VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Felgo REQUIRED)

set(PRODUCT_IDENTIFIER "com.example.creaturewars")
set(PRODUCT_VERSION_NAME "0.1.0")
set(PRODUCT_VERSION_CODE 1)
set(PRODUCT_STAGE "test")
set(PRODUCT_LICENSE_KEY "")

file(GLOB_RECURSE QmlFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} qml/*.qml qml/*.js)

set(BackendFiles
    backend/GameBackend.h
    backend/GameBackend.cpp
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

target_link_libraries(appCreatureWars PRIVATE Felgo)
