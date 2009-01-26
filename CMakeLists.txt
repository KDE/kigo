project(kigo)

if(NOT KDE4_FOUND)
    find_package(KDE4 REQUIRED)
    find_package(LibKDEGames REQUIRED)
    include(KDE4Defaults)
    #include(GGZ)
endif(NOT KDE4_FOUND)

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(docs)
