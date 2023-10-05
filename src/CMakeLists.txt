ecm_setup_version(${KIGO_VERSION}
    VARIABLE_PREFIX KIGO
    VERSION_HEADER kigo_version.h
)

add_executable(kigo)

target_sources(kigo PRIVATE
    game/game.cpp
    game/game.h
    game/move.cpp
    game/move.h
    game/player.cpp
    game/player.h
    game/score.cpp
    game/score.h
    game/stone.cpp
    game/stone.h
    gui/config/generalconfig.cpp
    gui/config/generalconfig.h
    gui/graphicsview/gamescene.cpp
    gui/graphicsview/gamescene.h
    gui/graphicsview/gameview.cpp
    gui/graphicsview/gameview.h
    gui/graphicsview/themerenderer.cpp
    gui/graphicsview/themerenderer.h
    gui/mainwindow.cpp
    gui/mainwindow.h
    gui/widgets/errorwidget.cpp
    gui/widgets/errorwidget.h
    gui/widgets/gamewidget.cpp
    gui/widgets/gamewidget.h
    gui/widgets/setupwidget.cpp
    gui/widgets/setupwidget.h
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
    KDEGames6
    KF6::XmlGui
    KF6::KIOWidgets  # KUrlRequester
    KF6::TextWidgets # KPluralHandlingSpinBox
    KF6::NewStuffWidgets
    KF6::DBusAddons
    KF6::Crash
    KF6::I18n
    Qt6::Svg
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
