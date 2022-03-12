cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "80")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KIGO_BASE_VERSION once new features are added
set(KIGO_BASE_VERSION "0.6")
set(KIGO_VERSION "${KIGO_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(kigo VERSION ${KIGO_VERSION})

set (QT_MIN_VERSION "5.15.0")
set (KF5_MIN_VERSION "5.90.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Svg)
find_package(Qt${QT_MAJOR_VERSION}QuickWidgets ${REQUIRED_QT_VERSION} CONFIG)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
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

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055800
    -DKNEWSTUFF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054F00 # KNS3::DownloadDialog
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
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
