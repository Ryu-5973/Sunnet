cmake_minimum_required(VERSION 3.0)
project(sunnet)

include_directories(include)
aux_source_directory(./src DIR_SRCS)
add_definitions(-std=c++2a)

link_directories(./3rd/lua-5.3.5/src)
link_directories("${PROJECT_SOURCE_DIR}/3rd/spdlog/build")

######################## 3rd party ###############################
# head file
include_directories(./3rd/lua-5.3.5/src)
include_directories("${PROJECT_SOURCE_DIR}/3rd/spdlog/include")

# libraries
link_directories(./3rd/lua-5.3.5/src)
link_directories("${PROJECT_SOURCE_DIR}/3rd/spdlog/build")

######################## 3rd party ###############################

add_executable(sunnet ${DIR_SRCS})

target_link_libraries(sunnet liblua.a)
target_link_libraries(sunnet dl)

find_package(Threads)
target_link_libraries(sunnet ${cmake_THREAD_LIBS_INIT})