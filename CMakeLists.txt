cmake_minimum_required(VERSION 2.8)

project(gpxtools)

add_executable(gpxls gpxls.cpp XMLParser.cpp)
target_link_libraries(gpxls)

add_executable(gpxrm gpxrm.cpp XMLParser.cpp)
target_link_libraries(gpxrm)

add_executable(gpxsim gpxsim.cpp XMLParser.cpp)
target_link_libraries(gpxsim)

add_executable(gpxjson gpxjson.cpp XMLParser.cpp)
target_link_libraries(gpxjson)

add_executable(gpxformat gpxformat.cpp)
target_link_libraries(gpxformat)

add_executable(gpxcat gpxcat.cpp XMLParser.cpp)
target_link_libraries(gpxcat)
