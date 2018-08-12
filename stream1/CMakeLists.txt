cmake_minimum_required(VERSION 3.10)
project(stream1)
set(CMAKE_CXX_STANDARD 14)

set(SRCS
        main.cpp
        H264Encoder.cpp H264Encoder.h
        LiveSourceWithx264.cpp LiveSourceWithx264.h
        GoblinMediaSession.cpp GoblinMediaSession.h
        )

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME}
        ${FFMPEG_LIBRARIES}
        ${OpenCV_LIBS}
        ${LIVE555_LIBRARIES}
        )
