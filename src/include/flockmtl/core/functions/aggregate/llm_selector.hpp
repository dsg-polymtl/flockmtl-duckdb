#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "flockmtl/common.hpp"
#include "flockmtl/core/model_manager/model_manager.hpp"
#include "flockmtl/core/model_manager/tiktoken.hpp"
#include "flockmtl/core/module.hpp"

namespace flockmtl {
namespace core {

struct LlmSelectorState {
public:
    std::vector<nlohmann::json> value;

    void Initialize();
    void Update(const nlohmann::json &input);
    void Combine(const LlmSelectorState &source);
};

class LlmSelector {
public:
    std::string model;
    int model_context_size;
    std::string user_prompt;
    std::string llm_selector_template;
    int available_tokens;

    LlmSelector(std::string &model, int model_context_size, std::string &user_prompt,
                std::string &llm_selector_template);

    nlohmann::json GetElement(const nlohmann::json &tuples);
    nlohmann::json LlmSelectorCall(nlohmann::json &tuples);

private:
    int calculateFixedTokens() const;
};

} // namespace core
} // namespace flockmtl
