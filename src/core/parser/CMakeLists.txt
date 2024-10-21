add_subdirectory(llm_response)
add_subdirectory(scalar)
add_subdirectory(query)

set(EXTENSION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/query_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tokenizer.cpp ${EXTENSION_SOURCES}
    PARENT_SCOPE)
