add_subdirectory(llm_reduce)
add_subdirectory(llm_first_or_last)
add_subdirectory(llm_rerank)

set(EXTENSION_SOURCES
    ${EXTENSION_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/aggregate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/aggregate_state.cpp
    PARENT_SCOPE)
