
set(SOURCES
    queue.cpp
    work-queue.cpp
)

add_library(workq++ ${SOURCES})
target_link_libraries(workq++ thpool)
target_include_directories(workq++ PUBLIC ${PROJECT_SOURCE_DIR}/include)

if (WORKQ_DEBUG)
    target_compile_definitions(workq++ PRIVATE WORKQ_DEBUG=1)
endif(WORKQ_DEBUG)