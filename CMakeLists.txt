cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "08")
set (RELEASE_SERVICE_VERSION_MICRO "2")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KIGO_BASE_VERSION once new features are added
set(KIGO_BASE_VERSION "0.6")
set(KIGO_VERSION "${KIGO_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(kigo VERSION ${KIGO_VERSION})

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")

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

include(InternalMacros)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Svg
    Widgets
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Config
    Crash
    DBusAddons
    DocTools
    I18n
    ConfigWidgets
    TextWidgets
    WidgetsAddons
    XmlGui
    KIO
    NewStuff
    IconThemes
)

find_package(KDEGames6 6.0.0 REQUIRED)

find_package(GNUGo QUIET)
set_package_properties(GNUGo PROPERTIES
    DESCRIPTION "A go-playing program (engine)"
    URL "https://www.gnu.org/software/gnugo/"
    PURPOSE "Used as backend (or any other compatible with the GTP protocol)"
    TYPE RUNTIME
)

ecm_set_disabled_deprecation_versions(
    QT 6.9.0
    KF 6.5
    KDEGAMES 6.0
)

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(doc)

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
