add_library(list list.c list.h)

add_executable(linux_threads main.c switch.S)
target_link_libraries(linux_threads list)
