cmake_minimum_required(VERSION 3.10)
project(client1)
set(CMAKE_CXX_STANDARD 14)

set(SRCS
        client1.cpp
        ElfRTSPClient.h
        ElfClientState.h
        ElfSink.cpp ElfSink.h
        VDecoder.cpp VDecoder.h
        )

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME}
        ${FFMPEG_LIBRARIES}
        ${OpenCV_LIBS}
        ${LIVE555_LIBRARIES}
        )
