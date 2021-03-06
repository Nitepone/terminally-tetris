cmake_minimum_required (VERSION 3.0)

project(tetris-mint)

# Previously, we used add_definitions() to set the compile flags, but that is not always supported.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fcommon")

if (WIN32)
    # add our custom cmake modules (for curses)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
    set(ADDITIONAL_LIBS wsock32 ws2_32)
    # By passing the `-static` flag, MinGW / GCC will use static libraries when
    # available (instead of DLLs). This is especially important for dlls like
    # pthreads on Windows, which I do not expect end users to have.
    #
    # TODO We should be able to use the static flag on Linux as well, but I'm
    # having trouble getting ncurses to link statically.
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
endif()

find_package(Threads)
find_package(Curses)
include_directories(${CURSES_INCLUDE_DIRS})


list(APPEND tetrismint_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tetris_game.c
    ${CMAKE_CURRENT_LIST_DIR}/client_conn.c
    ${CMAKE_CURRENT_LIST_DIR}/controller.c
    ${CMAKE_CURRENT_LIST_DIR}/generic.c
    ${CMAKE_CURRENT_LIST_DIR}/list.c
    ${CMAKE_CURRENT_LIST_DIR}/message.c
    ${CMAKE_CURRENT_LIST_DIR}/offline.c
    ${CMAKE_CURRENT_LIST_DIR}/player.c
    ${CMAKE_CURRENT_LIST_DIR}/render.c
    ${CMAKE_CURRENT_LIST_DIR}/widgets.c
    ${CMAKE_CURRENT_LIST_DIR}/curses_text_entry.c
    ${CMAKE_CURRENT_LIST_DIR}/curses_combobox.c
    ${CMAKE_CURRENT_LIST_DIR}/terminal_size.c
    ${CMAKE_CURRENT_LIST_DIR}/log.c
    ${CMAKE_CURRENT_LIST_DIR}/os_compat.c
    ${CMAKE_CURRENT_LIST_DIR}/party.c
    ${CMAKE_CURRENT_LIST_DIR}/event.c
)

add_library(tetrismintlib OBJECT ${tetrismint_SOURCES})

# Solo main uses termios, which is *nix only. For now, just skip building
# solo main on Windows.
if (NOT WIN32)
    add_executable(solo_main solo_main.c tetris_game.c)
    target_link_libraries(solo_main ${CMAKE_THREAD_LIBS_INIT} )
endif()

add_executable(tetris-mint client.c $<TARGET_OBJECTS:tetrismintlib>)
add_executable(tetris-mint-server server.c $<TARGET_OBJECTS:tetrismintlib>)
add_executable(test_render test_render.c $<TARGET_OBJECTS:tetrismintlib>)
add_executable(test_client_conn test_client_conn.c $<TARGET_OBJECTS:tetrismintlib>)
add_executable(test_player test_player.c $<TARGET_OBJECTS:tetrismintlib>)

target_link_libraries(tetrismintlib ${CURSES_LIBRARIES} ${CURSES_INCLUDE_DIRS})
target_link_libraries(tetris-mint ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBS})
target_link_libraries(tetris-mint-server ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBS})
target_link_libraries(test_render ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBS})
target_link_libraries(test_client_conn ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBS})
target_link_libraries(test_player ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBS})

# install for cpack packaging
install(
    TARGETS tetris-mint
    RUNTIME DESTINATION bin
)
install(
    TARGETS tetris-mint-server
    RUNTIME DESTINATION bin
)
