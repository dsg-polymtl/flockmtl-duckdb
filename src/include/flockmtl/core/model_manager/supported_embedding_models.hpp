#pragma once
#include <string>
#include "supported_providers.hpp"

const std::string TEXT_EMBEDDING_3_SMALL = "text-embedding-3-small";
const std::string TEXT_EMBEDDING_3_LARGE = "text-embedding-3-large";

enum SupportedEmbeddingModels {
    FLOCKMTL_TEXT_EMBEDDING_3_SMALL = 0,
    FLOCKMTL_TEXT_EMBEDDING_3_LARGE,
    FLOCKMTL_OLLAMA_EMBEDDING_MODEL,
    FLOCKMTL_UNSUPPORTED_EMBEDDING_MODEL,
    FLOCKMTL_SUPPORTED_EMBEDDING_MODELS_COUNT
};

inline SupportedEmbeddingModels GetEmbeddingModelType(std::string model, std::string provider) {
    std::transform(provider.begin(), provider.end(), provider.begin(), [](unsigned char c) { return std::tolower(c); });
    if (provider == OLLAMA)
        return FLOCKMTL_OLLAMA_EMBEDDING_MODEL;

    std::transform(model.begin(), model.end(), model.begin(), [](unsigned char c) { return std::tolower(c); });
    if (model == TEXT_EMBEDDING_3_SMALL)
        return FLOCKMTL_TEXT_EMBEDDING_3_SMALL;
    if (model == TEXT_EMBEDDING_3_LARGE)
        return FLOCKMTL_TEXT_EMBEDDING_3_LARGE;

    return FLOCKMTL_UNSUPPORTED_EMBEDDING_MODEL;
}
