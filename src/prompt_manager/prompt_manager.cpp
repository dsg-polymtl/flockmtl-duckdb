#include "flockmtl/prompt_manager/prompt_manager.hpp"

namespace flockmtl {

template <>
std::string PromptManager::ToString<PromptSection>(PromptSection section) {
    switch (section) {
    case PromptSection::USER_PROMPT:
        return "{{USER_PROMPT}}";
    case PromptSection::TUPLES:
        return "{{TUPLES}}";
    case PromptSection::RESPONSE_FORMAT:
        return "{{RESPONSE_FORMAT}}";
    case PromptSection::INSTRUCTIONS:
        return "{{INSTRUCTIONS}}";
    default:
        return "";
    }
}

std::string PromptManager::ReplaceSection(const std::string& prompt_template, const PromptSection section,
                                          const std::string& section_content) {
    auto replace_string = PromptManager::ToString(section);
    return PromptManager::ReplaceSection(prompt_template, replace_string, section_content);
}

std::string PromptManager::ReplaceSection(const std::string& prompt_template, const std::string& replace_string,
                                          const std::string& section_content) {
    auto prompt = prompt_template;
    auto replace_string_size = replace_string.size();
    auto replace_pos = prompt.find(replace_string);

    while (replace_pos != std::string::npos) {
        prompt.replace(replace_pos, replace_string_size, section_content);
        replace_pos = prompt.find(replace_string, replace_pos + section_content.size());
    }

    return prompt;
}

std::string PromptManager::ConstructMarkdownHeader(const nlohmann::json& tuple) {
    std::string header_markdown = "|";
    for (const auto& key : tuple.items()) {
        header_markdown += key.key() + " | ";
    }
    header_markdown += "\n";
    for (auto i = 0; i < tuple.size(); i++) {
        header_markdown += "|---";
    }
    header_markdown += "|\n";
    return header_markdown;
}

std::string PromptManager::ConstructMarkdownSingleTuple(const nlohmann::json& tuple) {
    std::string tuple_markdown = "|";
    for (const auto& key : tuple.items()) {
        tuple_markdown += key.value().dump() + " | ";
    }
    tuple_markdown += "\n";
    return tuple_markdown;
}

std::string PromptManager::ConstructMarkdownArrayTuples(const nlohmann::json& tuples) {
    std::string tuples_markdown = "";
    tuples_markdown += PromptManager::ConstructMarkdownHeader(tuples[0]);
    for (const auto& tuple : tuples) {
        tuples_markdown += PromptManager::ConstructMarkdownSingleTuple(tuple);
    }
    return tuples_markdown;
}

PromptDetails PromptManager::CreatePromptDetails(const nlohmann::json& prompt_details_json) {
    PromptDetails prompt_details;

    if (prompt_details_json.contains("prompt_name")) {
        if (!prompt_details_json.contains("version") && prompt_details_json.size() > 1) {
            throw std::runtime_error("The prompt details struct should contain a single key value pair of prompt or "
                                     "prompt_name with prompt version");
        } else if (prompt_details_json.contains("version") && prompt_details_json.size() > 2) {
            throw std::runtime_error("The prompt details struct should contain a single key value pair of prompt or "
                                     "prompt_name with prompt version");
        }
        prompt_details.prompt_name = prompt_details_json["prompt_name"];
        std::string prompt_details_query = duckdb_fmt::format(" SELECT prompt "
                                                              "   FROM flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE "
                                                              "  WHERE prompt_name = '{}'",
                                                              prompt_details.prompt_name);
        std::string error_message = duckdb_fmt::format("The provided `{}` prompt ", prompt_details.prompt_name);
        if (prompt_details_json.contains("version")) {
            prompt_details.version = std::stoi(prompt_details_json["version"].get<std::string>());
            prompt_details_query += duckdb_fmt::format(" AND version = {}", prompt_details.version);
            error_message += duckdb_fmt::format("with version {} not found", prompt_details.version);
        } else {
            prompt_details_query += " ORDER BY version DESC LIMIT 1;";
            error_message += "not found";
        }
        auto con = Config::GetConnection();
        auto query_result = con.Query(prompt_details_query);
        if (query_result->RowCount() == 0) {
            throw std::runtime_error(error_message);
        }
        prompt_details.prompt = query_result->GetValue(0, 0).ToString();
    } else if (prompt_details_json.contains("prompt")) {
        prompt_details.prompt = prompt_details_json["prompt"];
    } else {
        throw std::runtime_error("The prompt details struct should contain a single key value pair of prompt or "
                                 "prompt_name with prompt version");
    }
    return prompt_details;
}

} // namespace flockmtl
