
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
kde4_add_app_icon(kigo_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../data/icons/hi*-app-kigo.png")
add_executable(kigo ${kigo_SRCS})

target_link_libraries(kigo
   KF5::KIOWidgets  # KUrlRequester
   KF5::TextWidgets # KPluralHandlingSpinBox
   KF5::KDELibs4Support # KFileDialog, KPixmapCache
   KF5KDEGames
   KF5KDEGamesPrivate
   KF5::NewStuff
   Qt5::Svg
)

install(TARGETS kigo ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kigo.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kigo.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
install(FILES kigo-games.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
install(PROGRAMS gui/org.kde.kigo.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES gui/kigoui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kigo)
