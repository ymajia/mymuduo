cmake_minimum_required(VERSION 3.0)
project(mymuduo)

file(GLOB SRC_LIST1 ${CMAKE_CURRENT_SOURCE_DIR}/base/*.cc)
file(GLOB SRC_LIST2 ${CMAKE_CURRENT_SOURCE_DIR}/net/*.cc)
set(SRC_LIST1 ${SRC_LIST1} ${SRC_LIST2})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/base)

add_library(mymuduo SHARED ${SRC_LIST1})
# target_link_libraries(mymuduo pthread)