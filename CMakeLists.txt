cmake_minimum_required(VERSION 2.8)

project(gpxtools)

add_executable(gpxls gpxls.cpp XMLParser.cpp)
target_link_libraries(gpxls)

add_executable(gpxrm gpxrm.cpp XMLParser.cpp)
target_link_libraries(gpxrm)
