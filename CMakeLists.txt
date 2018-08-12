cmake_minimum_required(VERSION 3.10)
project(live555_fun)

#file(COPY Jump.mp3 DESTINATION .)

set(CMAKE_CXX_STANDARD 14)
#add_definitions("-D__STDC_CONSTANT_MACROS")

# OpenCV
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )

# PkgConfig
find_package(PkgConfig REQUIRED)

# ffmpeg
pkg_search_module(AVCODEC required libavcodec)
pkg_search_module(AVUTIL required libavutil)
include_directories(${AVCODEC_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS})
set(FFMPEG_LIBRARIES ${AVCODEC_LIBRARIES} ${AVUTIL_LIBRARIES})
message("FFMPEG_LIBRARIES = ${FFMPEG_LIBRARIES}")

# live555
pkg_search_module(LIVE555 live555)
include_directories(${LIVE555_INCLUDE_DIRS})


add_subdirectory(stream1)
add_subdirectory(client0)
add_subdirectory(client1)
