include_directories(${KDE4_INCLUDES} ${KDEGAMES_INCLUDE_DIR} ../src)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
MACRO(KGO_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_unit_test(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${KDE4_KDEGAMES_LIBS} ${QT_QTTEST_LIBRARY} kdegames)
    ENDFOREACH(_testname)
ENDMACRO(KGO_UNIT_TESTS)
MACRO(KGO_EXECUTABLE_TESTS)
    FOREACH(_testname ${ARGN})
        set(${_testname}_SRCS ${_testname}.cpp)
        #if test has an ui file, use it
        IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_testname}.ui)
            kde4_add_ui_files(${_testname}_SRCS ${_testname}.ui)
        ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_testname}.ui)

        kde4_add_executable(${_testname} TEST ${${_testname}_SRCS})
        target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${KDE4_KDEGAMES_LIBS} ${QT_QTTEST_LIBRARY} kdegames)
    ENDFOREACH(_testname)
ENDMACRO(KGO_EXECUTABLE_TESTS)

KGO_EXECUTABLE_TESTS(
#     goengineexecutabletest
)

KGO_UNIT_TESTS(
    goenginetest
)
