cmake_minimum_required(VERSION 2.8)

project(gpxtools)

find_package(EXPAT REQUIRED)

add_executable(gpxls gpxls.cpp XMLParser.cpp)
target_link_libraries(gpxls ${EXPAT_LIBRARIES})

add_executable(gpxrm gpxrm.cpp XMLParser.cpp)
target_link_libraries(gpxrm ${EXPAT_LIBRARIES})
