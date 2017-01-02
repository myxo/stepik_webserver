cmake_minimum_required(VERSION 2.8)

add_compile_options(-std=c++11)
# find_package( Threads )

add_executable(final server.cpp)

target_link_libraries( final pthread )