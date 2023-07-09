cmake_minimum_required(VERSION 3.0)
project(testserver)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_executable(server testserver.cc)
target_link_libraries(server mymuduo)