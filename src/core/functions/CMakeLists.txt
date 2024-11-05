add_subdirectory(scalar)
add_subdirectory(aggregate)

set(EXTENSION_SOURCES
    ${EXTENSION_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/batch_response_builder.cpp
    PARENT_SCOPE)
