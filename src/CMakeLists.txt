include_directories(${KDE4_INCLUDES} ${KDEGAMES_INCLUDE_DIR})

set(kgo_SRCS
    game/goengine.cpp
    gui/graphicsview/themerenderer.cpp
    gui/graphicsview/gamescene.cpp
    gui/graphicsview/gameview.cpp
    gui/screens/messagescreen.cpp
    gui/screens/setupscreen.cpp
    gui/screens/gamescreen.cpp
    gui/config.cpp
    gui/mainwindow.cpp
    main.cpp 
)

kde4_add_ui_files(kgo_SRCS 
    gui/screens/messagescreen.ui
    gui/screens/setupscreen.ui
    gui/screens/gamescreen.ui
    gui/config.ui
)
kde4_add_kcfg_files(kgo_SRCS preferences.kcfgc)
kde4_add_app_icon(kgo_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-kgo.png")
kde4_add_executable(kgo ${kgo_SRCS})

target_link_libraries(kgo ${KDE4_KDEUI_LIBS} ${KDEGAMES_LIBRARY} ${KDE_KNEWSTUFF2_LIBS} ${KGGZGAMES_LIBS})

install(TARGETS kgo DESTINATION ${BIN_INSTALL_DIR})
install(FILES gui/kgo.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES gui/kgoui.rc DESTINATION ${DATA_INSTALL_DIR}/kgo)
install(FILES kgo.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kgo.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

#register_ggz_module(module.dsc)
