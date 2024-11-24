add_subdirectory(functions)
add_subdirectory(parser)
add_subdirectory(config)

set(EXTENSION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp ${EXTENSION_SOURCES}
    PARENT_SCOPE)
