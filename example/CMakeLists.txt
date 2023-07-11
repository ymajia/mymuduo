cmake_minimum_required(VERSION 3.0)
project(httpserver)

set(SRC_LIST main.cc HttpServer.cc HttpResponse.cc HttpParser.cc)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_executable(httpserver ${SRC_LIST})
target_link_libraries(httpserver mymuduo)