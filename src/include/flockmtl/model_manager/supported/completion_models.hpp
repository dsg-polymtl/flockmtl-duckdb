#pragma once

#include "flockmtl/model_manager/supported/providers.hpp"

namespace flockmtl {

const std::string GPT_4O = "gpt-4o";
const std::string GPT_4O_MINI = "gpt-4o-mini";

enum SupportedModels {
    FLOCKMTL_GPT_4o,
    FLOCKMTL_GPT_4o_MINI,
    FLOCKMTL_OLLAMA_MODEL,
    FLOCKMTL_UNSUPPORTED_MODEL,
    FLOCKMTL_SUPPORTED_MODELS_COUNT
};

inline SupportedModels GetModelType(std::string model, std::string provider) {
    std::transform(provider.begin(), provider.end(), provider.begin(), [](unsigned char c) { return std::tolower(c); });
    if (provider == OLLAMA)
        return FLOCKMTL_OLLAMA_MODEL;
    std::transform(model.begin(), model.end(), model.begin(), [](unsigned char c) { return std::tolower(c); });
    if (model == GPT_4O)
        return FLOCKMTL_GPT_4o;
    if (model == GPT_4O_MINI)
        return FLOCKMTL_GPT_4o_MINI;

    return FLOCKMTL_UNSUPPORTED_MODEL;
}

} // namespace flockmtl
