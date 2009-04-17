project(kigo)

if(NOT KDE4_FOUND)
    find_package(KDE4 REQUIRED)
    find_package(LibKDEGames REQUIRED)
    include(KDE4Defaults)
    #include(GGZ)
endif(NOT KDE4_FOUND)

add_subdirectory(src)
add_subdirectory(data)

kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en)
