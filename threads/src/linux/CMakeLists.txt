add_library(list list.c list.h)

add_executable(linux_threads main.c
        spin_lock.c spin_lock.h
        thread.c thread.h
        switch.S)
target_link_libraries(linux_threads list)
