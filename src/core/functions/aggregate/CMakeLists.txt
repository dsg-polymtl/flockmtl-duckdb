set(EXTENSION_SOURCES
        ${EXTENSION_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/llm_first.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/llm_last.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/llm_agg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/llm_rerank.cpp
        PARENT_SCOPE)
