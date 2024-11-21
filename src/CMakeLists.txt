add_subdirectory(core)
add_subdirectory(prompt_manager)

set(EXTENSION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/flockmtl_extension.cpp ${EXTENSION_SOURCES}
    PARENT_SCOPE)
