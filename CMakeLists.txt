cmake_minimum_required(VERSION 3.13)
project(rear_sight_clone_project)

set(CMAKE_CXX_STANDARD 20)

add_executable(rear_sight rear_sight_webrtc_frame_change.cpp
#add_executable(rear_sight   rear_sight_webrtc_frame_change_raspberry_embedded.cpp
                            rear_sight_processor/image_processing.h
                            rear_sight_processor/rear_sight_processor.h
                            rear_sight_processor/rear_sight_processor.cpp)
#add_executable(rear_sight web_sockets/WebServer.h
#                          web_sockets/WebServer.cpp)

#target_link_libraries(rear_sight seasocks "${ZLIB_LIBRARIES}")
#target_link_libraries(rear_sight pthread z)

#target_link_libraries(rear_sight curses)  #this one is used for keyboard handling (but it doesn't work correctly)
#target_link_libraries(rear_sight ncurses) #this one is used for keyboard handling (but it doesn't work correctly)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(rear_sight ${OpenCV_LIBS})

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(GLIB "glib-2.0")
    if (GLIB_FOUND)
        target_link_libraries(rear_sight ${GLIB_LIBRARIES})
        add_definitions(${GLIB_CFLAGS} ${GLIB_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(GST "gstreamer-1.0")
    if (GST_FOUND)
        target_link_libraries(rear_sight ${GST_LIBRARIES})
        add_definitions(${GST_CFLAGS} ${GST_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(SPD "gstreamer-sdp-1.0")
    if (SPD_FOUND)
        target_link_libraries(rear_sight ${SPD_LIBRARIES})
        add_definitions(${SPD_CFLAGS} ${SPD_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(WEBRTC "gstreamer-webrtc-1.0")
    if (WEBRTC_FOUND)
        target_link_libraries(rear_sight ${WEBRTC_LIBRARIES})
        add_definitions(${WEBRTC_CFLAGS} ${WEBRTC_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(JSON "json-glib-1.0")
    if (JSON_FOUND)
        target_link_libraries(rear_sight ${JSON_LIBRARIES})
        add_definitions(${JSON_CFLAGS} ${JSON_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(SOUP "libsoup-2.4")
    if (SOUP_FOUND)
        target_link_libraries(rear_sight ${SOUP_LIBRARIES})
        add_definitions(${SOUP_CFLAGS} ${SOUP_CFLAGS_OTHER})
    endif()
endif()

add_custom_command(TARGET rear_sight POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/web_src $<TARGET_FILE_DIR:rear_sight>/web_src)