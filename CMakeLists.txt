cmake_minimum_required(VERSION 2.8)

add_compile_options(-std=c++11 -g -Wall)
# add_compile_options(-g)
# add_compile_options(-Wall)
# find_package( Threads )

set(SRC_LIST fork/master_server.cpp fork/slave_server.cpp fork/main.cpp fork/fd_passing.cpp fork/log_utils.cpp fork/server_pool.cpp)
add_executable(final ${SRC_LIST})

# add_executable(final server.cpp)

target_link_libraries(final pthread)