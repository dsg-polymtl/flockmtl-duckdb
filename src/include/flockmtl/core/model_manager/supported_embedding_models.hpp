#pragma once
#include <string>
enum SupportedEmbeddingModels {
    FLOCKMTL_TEXT_EMBEDDING_3_SMALL = 0,
    FLOCKMTL_TEXT_EMBEDDING_3_LARGE,
    FLOCKMTL_OLLAMA_EMBEDDING_MODEL,
    FLOCKMTL_UNSUPPORTED_EMBEDDING_MODEL,
    FLOCKMTL_SUPPORTED_EMBEDDING_MODELS_COUNT
};

inline SupportedEmbeddingModels GetEmbeddingModelType(std::string model, std::string provider) {
    std::transform(provider.begin(), provider.end(), provider.begin(), [](unsigned char c) { return std::tolower(c); });
    if (provider == "ollama")
        return FLOCKMTL_OLLAMA_EMBEDDING_MODEL;

    std::transform(model.begin(), model.end(), model.begin(), [](unsigned char c) { return std::tolower(c); });
    if (model == "text-embedding-3-small")
        return FLOCKMTL_TEXT_EMBEDDING_3_SMALL;
    if (model == "text-embedding-3-large")
        return FLOCKMTL_TEXT_EMBEDDING_3_LARGE;

    return FLOCKMTL_UNSUPPORTED_EMBEDDING_MODEL;
}
