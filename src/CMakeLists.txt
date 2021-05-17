ecm_setup_version(${KIGO_VERSION}
    VARIABLE_PREFIX KIGO
    VERSION_HEADER kigo_version.h
)

add_executable(kigo)

target_sources(kigo PRIVATE
    game/game.cpp
    game/move.cpp
    game/player.cpp
    game/score.cpp
    game/stone.cpp
    gui/config/generalconfig.cpp
    gui/mainwindow.cpp
    gui/graphicsview/gamescene.cpp
    gui/graphicsview/gameview.cpp
    gui/graphicsview/themerenderer.cpp
    gui/widgets/errorwidget.cpp
    gui/widgets/gamewidget.cpp
    gui/widgets/setupwidget.cpp
    main.cpp

    kigo.qrc
)

ki18n_wrap_ui(kigo
    gui/config/generalconfig.ui
    gui/widgets/errorwidget.ui
    gui/widgets/gamewidget.ui
    gui/widgets/setupwidget.ui
)

ecm_qt_declare_logging_category(kigo
    HEADER kigo_debug.h
    IDENTIFIER KIGO_LOG
    CATEGORY_NAME org.kde.kdegames.kigo
    DESCRIPTION "kigo (kdegames)"
    EXPORT KIGO
)

kconfig_add_kcfg_files(kigo preferences.kcfgc)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../data/icons/*-apps-kigo.png")
ecm_add_app_icon(kigo ICONS ${ICONS_SRCS})

target_link_libraries(kigo
    KF5KDEGames
    KF5KDEGamesPrivate
    KF5::XmlGui
    KF5::KIOWidgets  # KUrlRequester
    KF5::TextWidgets # KPluralHandlingSpinBox
    KF5::NewStuff
    KF5::DBusAddons
    KF5::Crash
    Qt5::Svg
)

install(TARGETS kigo ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kigo.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kigo.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(FILES kigo-games.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(PROGRAMS gui/org.kde.kigo.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES gui/org.kde.kigo.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ecm_qt_install_logging_categories(
    EXPORT KIGO
    FILE kigo.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
