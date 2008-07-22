project(kgo)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})
include(KDE4Defaults)
#include(GGZ)

add_subdirectory(src)

add_subdirectory(icons)
add_subdirectory(themes)
add_subdirectory(games)
add_subdirectory(sounds)
add_subdirectory(docs)
