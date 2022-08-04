
cmake_minimum_required( VERSION 2.8 )

project( minghttpd )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
link_libraries(pthread)
add_executable( httpserver mainfunc.cpp httpd.cpp)
