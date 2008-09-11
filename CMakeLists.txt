project(kgo)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

if(NOT KDE4_FOUND)
    find_package(KDE4 REQUIRED)
    include(KDE4Defaults)
    #include(GGZ)
    macro_optional_find_package(LibKDEGames QUIET)
endif(NOT KDE4_FOUND)

add_subdirectory(src)
add_subdirectory(icons)
add_subdirectory(themes)
add_subdirectory(games)
add_subdirectory(sounds)
add_subdirectory(docs)
