project(kigo)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

if(NOT KDE4_FOUND)
    find_package(KDE4 REQUIRED)
    find_package(LibKDEGames REQUIRED)
    include(KDE4Defaults)
    #include(GGZ)
endif(NOT KDE4_FOUND)

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(docs)

# Packaging section
set(CPACK_GENERATOR "STGZ;TGZ;TZ;DEB")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Go board game for KDE")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "README")
set(CPACK_PACKAGE_VENDOR "Sascha Peilicke <sasch.pe@gmx.de>")
set(CPACK_PACKAGE_CONTACT "Sascha Peilicke <sasch.pe@gmx.de>")
#set(CPACK_PACKAGE_EXECUTABLES "kigo")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "5")
set(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "kdebase-runtime (>= 4:4.1.2), kdelibs5 (>= 4:4.1.3), libacl1 (>= 2.2.11-1), libattr1 (>= 2.4.41-1), libbz2-1.0, libc6 (>= 2.4), libgcc1 (>= 1:4.1.1), libice6 (>= 1:1.0.0), libkdegames4, libqt4-dbus (>= 4.4.3), libqt4-network (>= 4.4.3), libqt4-svg (>= 4.4.3), libqt4-xml (>= 4.4.3), libqtcore4 (>= 4.4.3), libqtgui4 (>= 4.4.3), libsm6, libstdc++6 (>= 4.1.1), libstreamanalyzer0 (>= 0.5.11), libstreams0 (>= 0.5.11), libx11-6, libxau6, libxcursor1 (> 1.1.2), libxdmcp6, libxext6, libxfixes3 (>= 1:4.0.1), libxft2 (> 2.1.1), libxrender1, libxtst6, zlib1g (>= 1:1.1.4)")

include(CPack)
