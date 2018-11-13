cmake_minimum_required (VERSION 2.6)
set (CMAKE_CXX_STANDARD 14)

project (Gossip)

file(GLOB pull_src
    "src/pull/*"
)

file(GLOB push_src
    "src/push/*"
)

add_executable(pull ${pull_src})
add_executable(push ${push_src})

target_link_libraries (push pthread)
target_link_libraries (pull pthread)

include_directories("./include")