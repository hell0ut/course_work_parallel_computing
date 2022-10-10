cmake_minimum_required(VERSION 3.23)
project(course_work_parallel_computing_server)
project(course_work_parallel_computing_client)

set(CMAKE_CXX_STANDARD 17)

add_executable(course_work_parallel_computing_server main.cpp)
if(WIN32)
    target_link_libraries(course_work_parallel_computing_server wsock32 ws2_32)
endif()
add_executable(course_work_parallel_computing_client client.cpp)
if(WIN32)
    target_link_libraries(course_work_parallel_computing_client wsock32 ws2_32)
endif()

