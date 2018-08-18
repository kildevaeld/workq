
add_library(thpool c-thread-pool/thpool.c)
target_link_libraries(thpool pthread)
