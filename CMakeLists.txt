cmake_minimum_required(VERSION 3.10.2)
project(Elite_Aim)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fvisibility=hidden -Os -fno-rtti -fno-exceptions")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(dobby SHARED IMPORTED)
set_target_properties(dobby PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libdobby.so)

add_library(elite_aim SHARED src/main.cpp)

target_link_libraries(elite_aim dobby log android)
