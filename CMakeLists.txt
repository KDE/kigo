cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "04")
set (RELEASE_SERVICE_VERSION_MICRO "2")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KIGO_BASE_VERSION once new features are added
set(KIGO_BASE_VERSION "0.6")
set(KIGO_VERSION "${KIGO_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(kigo VERSION ${KIGO_VERSION})

set (QT_MIN_VERSION "5.15.2")
set (KF_MIN_VERSION "5.92.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(ECMDeprecationSettings)
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Svg)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Config
    Crash
    DBusAddons
    DocTools
    I18n
    ConfigWidgets
    TextWidgets
    XmlGui
    KIO
    NewStuff
)

find_package(KF5KDEGames 7.3.0 REQUIRED)

find_package(GNUGo QUIET)
set_package_properties(GNUGo PROPERTIES
    DESCRIPTION "A go-playing program (engine)"
    URL "https://www.gnu.org/software/gnugo/"
    PURPOSE "Used as backend (or any other compatible with the GTP protocol)"
    TYPE RUNTIME
)

ecm_set_disabled_deprecation_versions(
    QT 6.4
    KF 5.103
    KDEGAMES 7.3
)


# added by KDE_COMPILERSETTINGS_LEVEL 5.85
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
)

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(doc)

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
