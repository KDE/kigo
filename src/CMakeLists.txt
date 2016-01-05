
set(kigo_SRCS
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
)

ki18n_wrap_ui(kigo_SRCS 
    gui/config/generalconfig.ui
    gui/widgets/errorwidget.ui
    gui/widgets/gamewidget.ui
    gui/widgets/setupwidget.ui
)
kconfig_add_kcfg_files(kigo_SRCS preferences.kcfgc)
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../data/icons/*-apps-kigo.png")
ecm_add_app_icon(kigo_SRCS ICONS ${ICONS_SRCS})

add_executable(kigo ${kigo_SRCS})

target_link_libraries(kigo
   KF5::XmlGui
   KF5::KIOWidgets  # KUrlRequester
   KF5::TextWidgets # KPluralHandlingSpinBox
   KF5KDEGames
   KF5KDEGamesPrivate
   KF5::NewStuff
   KF5::DBusAddons
   Qt5::Svg
)

install(TARGETS kigo ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kigo.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kigo.knsrc DESTINATION ${KDE_INSTALL_CONFDIR})
install(FILES kigo-games.knsrc DESTINATION ${KDE_INSTALL_CONFDIR})
install(PROGRAMS gui/org.kde.kigo.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES gui/kigoui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kigo)
