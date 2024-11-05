#pragma once
#include <string>

enum SupportedModels {
    FLOCKMTL_GPT_4o = 0,
    FLOCKMTL_GPT_4o_MINI,
    FLOCKMTL_OLLAMA_MODEL,
    FLOCKMTL_UNSUPPORTED_MODEL,
    FLOCKMTL_SUPPORTED_MODELS_COUNT
};

inline SupportedModels GetModelType(std::string model, std::string provider) {
    std::transform(provider.begin(), provider.end(), provider.begin(), [](unsigned char c) { return std::tolower(c); });
    if (provider == "ollama")
        return FLOCKMTL_OLLAMA_MODEL;
    std::transform(model.begin(), model.end(), model.begin(), [](unsigned char c) { return std::tolower(c); });
    if (model == "gpt-4o")
        return FLOCKMTL_GPT_4o;
    if (model == "gpt-4o-mini")
        return FLOCKMTL_GPT_4o_MINI;

    return FLOCKMTL_UNSUPPORTED_MODEL;
}
