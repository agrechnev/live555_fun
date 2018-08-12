cmake_minimum_required(VERSION 3.10)
project(client0)
set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} client.cpp)

target_link_libraries(${PROJECT_NAME}
        ${LIVE555_LIBRARIES}
)
