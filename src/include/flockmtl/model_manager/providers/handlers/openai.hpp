// The MIT License (MIT)
//
// Copyright (c) 2023 Olrea, Florian Dang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef OPENAI_HPP_
#define OPENAI_HPP_

#if OPENAI_VERBOSE_OUTPUT
#pragma message("OPENAI_VERBOSE_OUTPUT is ON")
#endif

#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include "session.hpp"
#include <nlohmann/json.hpp> // nlohmann/json

namespace openai {

namespace _detail {

// Json alias
using Json = nlohmann::json;

// forward declaration for category structures
class OpenAI;

// https://platform.openai.com/docs/api-reference/models
// List and describe the various models available in the API. You can refer to
// the Models documentation to understand what models are available and the
// differences between them.
struct CategoryModel {
    Json list();
    Json retrieve(const std::string &model);

    CategoryModel(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/assistants
// Build assistants that can call models and use tools to perform tasks.
struct CategoryAssistants {
    Json create(Json input);
    Json retrieve(const std::string &assistants);
    Json modify(const std::string &assistants, Json input);
    Json del(const std::string &assistants);
    Json list();
    Json createFile(const std::string &assistants, Json input);
    Json retrieveFile(const std::string &assistants, const std::string &files);
    Json delFile(const std::string &assistants, const std::string &files);
    Json listFile(const std::string &assistants);

    CategoryAssistants(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/threads
// Create threads that assistants can interact with.
struct CategoryThreads {
    Json create();
    Json retrieve(const std::string &threads);
    Json modify(const std::string &threads, Json input);
    Json del(const std::string &threads);
    Json list();

    // https://platform.openai.com/docs/api-reference/messages
    // Create messages within threads
    Json createMessage(const std::string &threads, Json input);
    Json retrieveMessage(const std::string &threads, const std::string &messages);
    Json modifyMessage(const std::string &threads, const std::string &messages, Json input);
    Json listMessage(const std::string &threads);
    Json retrieveMessageFile(const std::string &threads, const std::string &messages, const std::string &files);
    Json listMessageFile(const std::string &threads, const std::string &messages);

    // https://platform.openai.com/docs/api-reference/runs
    // Represents an execution run on a thread.
    Json createRun(const std::string &threads, Json input);
    Json retrieveRun(const std::string &threads, const std::string &runs);
    Json modifyRun(const std::string &threads, const std::string &runs, Json input);
    Json listRun(const std::string &threads);
    Json submitToolOutputsToRun(const std::string &threads, const std::string &runs, Json input);
    Json cancelRun(const std::string &threads, const std::string &runs);
    Json createThreadAndRun(Json input);
    Json retrieveRunStep(const std::string &threads, const std::string &runs, const std::string &steps);
    Json listRunStep(const std::string &threads, const std::string &runs);

    CategoryThreads(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/completions
// Given a prompt, the model will return one or more predicted completions, and
// can also return the probabilities of alternative tokens at each position.
struct CategoryCompletion {
    Json create(Json input);

    CategoryCompletion(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/chat
// Given a prompt, the model will return one or more predicted chat completions.
struct CategoryChat {
    Json create(Json input);

    CategoryChat(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/audio
// Learn how to turn audio into text.
struct CategoryAudio {
    Json transcribe(Json input);
    Json translate(Json input);

    CategoryAudio(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/edits
// Given a prompt and an instruction, the model will return an edited version of
// the prompt.
struct CategoryEdit {
    Json create(Json input);

    CategoryEdit(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/images
// Given a prompt and/or an input image, the model will generate a new image.
struct CategoryImage {
    Json create(Json input);
    Json edit(Json input);
    Json variation(Json input);

    CategoryImage(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/embeddings
// Get a vector representation of a given input that can be easily consumed by
// machine learning models and algorithms.
struct CategoryEmbedding {
    Json create(Json input);
    CategoryEmbedding(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

struct FileRequest {
    std::string file;
    std::string purpose;
};

// https://platform.openai.com/docs/api-reference/files
// Files are used to upload documents that can be used with features like
// Fine-tuning.
struct CategoryFile {
    Json list();
    Json upload(Json input);
    Json del(const std::string &file); // TODO
    Json retrieve(const std::string &file_id);
    Json content(const std::string &file_id);

    CategoryFile(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/fine-tunes
// Manage fine-tuning jobs to tailor a model to your specific training data.
struct CategoryFineTune {
    Json create(Json input);
    Json list();
    Json retrieve(const std::string &fine_tune_id);
    Json content(const std::string &fine_tune_id);
    Json cancel(const std::string &fine_tune_id);
    Json events(const std::string &fine_tune_id);
    Json del(const std::string &model);

    CategoryFineTune(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// https://platform.openai.com/docs/api-reference/moderations
// Given a input text, outputs if the model classifies it as violating OpenAI's
// content policy.
struct CategoryModeration {
    Json create(Json input);

    CategoryModeration(OpenAI &openai) : openai_ {openai} {}

private:
    OpenAI &openai_;
};

// OpenAI
class OpenAI {
public:
    OpenAI(const std::string &token = "", const std::string &organization = "", bool throw_exception = true,
           const std::string &api_base_url = "", const std::string &beta = "")
        : session_ {"OpenAI", throw_exception}, token_ {token}, organization_ {organization},
          throw_exception_ {throw_exception} {
        if (token.empty()) {
            if (const char *env_p = std::getenv("OPENAI_API_KEY")) {
                token_ = std::string {env_p};
            }
        }
        if (api_base_url.empty()) {
            if (const char *env_p = std::getenv("OPENAI_API_BASE")) {
                base_url = std::string {env_p} + "/";
            } else {
                base_url = "https://api.openai.com/v1/";
            }
        } else {
            base_url = api_base_url;
        }
        session_.setUrl(base_url);
        session_.setToken(token_, organization_);
        session_.setBeta(beta);
    }

    OpenAI(const OpenAI &) = delete;
    OpenAI &operator=(const OpenAI &) = delete;
    OpenAI(OpenAI &&) = delete;
    OpenAI &operator=(OpenAI &&) = delete;

    void setToken(const std::string &token = "", const std::string &organization = "") {
        session_.setToken(token, organization);
    }

    static const char *get_openai_api_key() {
        static int check_done = -1;
        static const char *key = nullptr;

        if (check_done == -1) {
            key = std::getenv("OPENAI_API_KEY");
            check_done = 1;
        }

        if (key == nullptr) {
            throw std::runtime_error("OPENAI_API_KEY environment variable is not set.");
        }

        return key;
    }

    void setProxy(const std::string &url) { session_.setProxyUrl(url); }

    void setBeta(const std::string &beta) { session_.setBeta(beta); }

    // void change_token(const std::string& token) { token_ = token; };
    void setThrowException(bool throw_exception) { throw_exception_ = throw_exception; }

    void setMultiformPart(const std::pair<std::string, std::string> &filefield_and_filepath,
                          const std::map<std::string, std::string> &fields) {
        session_.setMultiformPart(filefield_and_filepath, fields);
    }

    Json post(const std::string &suffix, const std::string &data, const std::string &contentType) {
        setParameters(suffix, data, contentType);
        auto response = session_.postPrepare(contentType);
        if (response.is_error) {
            trigger_error(response.error_message);
        }

        Json json {};
        if (isJson(response.text)) {

            json = Json::parse(response.text);
            checkResponse(json);
        } else {
#if OPENAI_VERBOSE_OUTPUT
            std::cerr << "Response is not a valid JSON";
            std::cout << "<< " << response.text << "\n";
#endif
        }

        return json;
    }

    Json get(const std::string &suffix, const std::string &data = "") {
        setParameters(suffix, data);
        auto response = session_.getPrepare();
        if (response.is_error) {
            trigger_error(response.error_message);
        }

        Json json {};
        if (isJson(response.text)) {
            json = Json::parse(response.text);
            checkResponse(json);
        } else {
#if OPENAI_VERBOSE_OUTPUT
            std::cerr << "Response is not a valid JSON\n";
            std::cout << "<< " << response.text << "\n";
#endif
            json = Json {{"Result", response.text}};
        }
        return json;
    }

    Json post(const std::string &suffix, const Json &json, const std::string &contentType = "application/json") {
        return post(suffix, json.dump(), contentType);
    }

    Json del(const std::string &suffix) {
        setParameters(suffix, "");
        auto response = session_.deletePrepare();
        if (response.is_error) {
            trigger_error(response.error_message);
        }

        Json json {};
        if (isJson(response.text)) {
            json = Json::parse(response.text);
            checkResponse(json);
        } else {
#if OPENAI_VERBOSE_OUTPUT
            std::cerr << "Response is not a valid JSON\n";
            std::cout << "<< " << response.text << "\n";
#endif
        }
        return json;
    }

    std::string easyEscape(const std::string &text) { return session_.easyEscape(text); }

    void debug() const { std::cout << token_ << '\n'; }

    void setBaseUrl(const std::string &url) { base_url = url; }

    std::string getBaseUrl() const { return base_url; }

private:
    std::string base_url;

    void setParameters(const std::string &suffix, const std::string &data, const std::string &contentType = "") {
        auto complete_url = base_url + suffix;
        session_.setUrl(complete_url);

        if (contentType != "multipart/form-data") {
            session_.setBody(data);
        }

#if OPENAI_VERBOSE_OUTPUT
        std::cout << "<< request: " << complete_url << "  " << data << '\n';
#endif
    }

    void checkResponse(const Json &json) {
        if (json.count("error")) {
            auto reason = json["error"].dump();
            trigger_error(reason);

#if OPENAI_VERBOSE_OUTPUT
            std::cerr << ">> response error :\n" << json.dump(2) << "\n";
#endif
        }
    }

    // as of now the only way
    bool isJson(const std::string &data) {
        bool rc = true;
        try {
            auto json = Json::parse(data); // throws if no json
        } catch (std::exception &) {
            rc = false;
        }
        return (rc);
    }

    void trigger_error(const std::string &msg) {
        if (throw_exception_) {
            throw std::runtime_error(msg);
        } else {
            std::cerr << "[OpenAI] error. Reason: " << msg << '\n';
        }
    }

public:
    CategoryModel model {*this};
    CategoryAssistants assistant {*this};
    CategoryThreads thread {*this};
    CategoryCompletion completion {*this};
    CategoryEdit edit {*this};
    CategoryImage image {*this};
    CategoryEmbedding embedding {*this};
    CategoryFile file {*this};
    CategoryFineTune fine_tune {*this};
    CategoryModeration moderation {*this};
    CategoryChat chat {*this};
    CategoryAudio audio {*this};
    // CategoryEngine          engine{*this}; // Not handled since deprecated (use
    // Model instead)

private:
    Session session_;
    std::string token_;
    std::string organization_;
    bool throw_exception_;
};

inline std::string bool_to_string(const bool b) {
    std::ostringstream ss;
    ss << std::boolalpha << b;
    return ss.str();
}

inline OpenAI &start(const std::string &token = "", const std::string &organization = "", bool throw_exception = true,
                     const std::string &api_base_url = "") {
    static OpenAI instance {token, organization, throw_exception, api_base_url};
    return instance;
}

inline OpenAI &instance() { return start(); }

inline Json post(const std::string &suffix, const Json &json) { return instance().post(suffix, json); }

inline Json get(const std::string &suffix /*, const Json& json*/) { return instance().get(suffix); }

// Helper functions to get category structures instance()

inline CategoryModel &model() { return instance().model; }

inline CategoryAssistants &assistant() { return instance().assistant; }

inline CategoryThreads &thread() { return instance().thread; }

inline CategoryCompletion &completion() { return instance().completion; }

inline CategoryChat &chat() { return instance().chat; }

inline CategoryAudio &audio() { return instance().audio; }

inline CategoryEdit &edit() { return instance().edit; }

inline CategoryImage &image() { return instance().image; }

inline CategoryEmbedding &embedding() { return instance().embedding; }

inline CategoryFile &file() { return instance().file; }

inline CategoryFineTune &fineTune() { return instance().fine_tune; }

inline CategoryModeration &moderation() { return instance().moderation; }

// Definitions of category methods

// GET https://api.openai.com/v1/models
// Lists the currently available models, and provides basic information about
// each one such as the owner and availability.
inline Json CategoryModel::list() { return openai_.get("models"); }

// GET https://api.openai.com/v1/models/{model}
// Retrieves a model instance, providing basic information about the model such
// as the owner and permissioning.
inline Json CategoryModel::retrieve(const std::string &model) { return openai_.get("models/" + model); }

// POST https://api.openai.com/v1/assistants
// Create an assistant with a model and instructions.
inline Json CategoryAssistants::create(Json input) { return openai_.post("assistants", input); }

// GET https://api.openai.com/v1/assistants/{assistant_id}
// Retrieves an assistant.
inline Json CategoryAssistants::retrieve(const std::string &assistants) {
    return openai_.get("assistants/" + assistants);
}

// POST https://api.openai.com/v1/assistants/{assistant_id}
// Modifies an assistant.
inline Json CategoryAssistants::modify(const std::string &assistants, Json input) {
    return openai_.post("assistants/" + assistants, input);
}

// DELETE https://api.openai.com/v1/assistants/{assistant_id}
// Delete an assistant.
inline Json CategoryAssistants::del(const std::string &assistants) { return openai_.del("assistants/" + assistants); }

// GET https://api.openai.com/v1/assistants
// Returns a list of assistants.
inline Json CategoryAssistants::list() { return openai_.get("assistants"); }

// POST https://api.openai.com/v1/assistants/{assistant_id}/files
// Create an assistant file by attaching a File to an assistant.
inline Json CategoryAssistants::createFile(const std::string &assistants, Json input) {
    return openai_.post("assistants/" + assistants + "/files", input);
}

// GET https://api.openai.com/v1/assistants/{assistant_id}/files/{file_id}
// Retrieves an AssistantFile.
inline Json CategoryAssistants::retrieveFile(const std::string &assistants, const std::string &files) {
    return openai_.get("assistants/" + assistants + "/files/" + files);
}

// DELETE https://api.openai.com/v1/assistants/{assistant_id}/files/{file_id}
// Delete an assistant file.
inline Json CategoryAssistants::delFile(const std::string &assistants, const std::string &files) {
    return openai_.del("assistants/" + assistants + "/files/" + files);
}

// GET https://api.openai.com/v1/assistants/{assistant_id}/files
// Returns a list of assistant files.
inline Json CategoryAssistants::listFile(const std::string &assistants) {
    return openai_.get("assistants/" + assistants + "/files");
}

// POST https://api.openai.com/v1/threads
// Create a thread.
inline Json CategoryThreads::create() {
    Json input;
    return openai_.post("threads", input);
}

// GET https://api.openai.com/v1/threads/{thread_id}
// Retrieves a thread.
inline Json CategoryThreads::retrieve(const std::string &threads) { return openai_.get("threads/" + threads); }

// POST https://api.openai.com/v1/threads/{thread_id}
// Modifies a thread.
inline Json CategoryThreads::modify(const std::string &threads, Json input) {
    return openai_.post("threads/" + threads, input);
}

// DELETE https://api.openai.com/v1/threads/{thread_id}
// Delete a thread.
inline Json CategoryThreads::del(const std::string &threads) { return openai_.del("threads/" + threads); }

// POST https://api.openai.com/v1/threads/{thread_id}/messages
// Create a message.
inline Json CategoryThreads::createMessage(const std::string &threads, Json input) {
    return openai_.post("threads/" + threads + "/messages", input);
}

// GET https://api.openai.com/v1/threads/{thread_id}/messages/{message_id}
// Retrieve a message.
inline Json CategoryThreads::retrieveMessage(const std::string &threads, const std::string &messages) {
    return openai_.get("threads/" + threads + "/messages/" + messages);
}

// POST https://api.openai.com/v1/threads/{thread_id}/messages/{message_id}
// Modifies a message.
inline Json CategoryThreads::modifyMessage(const std::string &threads, const std::string &messages, Json input) {
    return openai_.post("threads/" + threads + "/messages/" + messages, input);
}

// GET https://api.openai.com/v1/threads/{thread_id}/messages
// Returns a list of messages for a given thread.
inline Json CategoryThreads::listMessage(const std::string &threads) {
    return openai_.get("threads/" + threads + "/messages");
}

// GET
// https://api.openai.com/v1/threads/{thread_id}/messages/{message_id}/files/{file_id}
// Retrieves a message file.
inline Json CategoryThreads::retrieveMessageFile(const std::string &threads, const std::string &messages,
                                                 const std::string &files) {
    return openai_.get("threads/" + threads + "/messages/" + messages + "/files/" + files);
}

// GET https://api.openai.com/v1/threads/{thread_id}/messages/{message_id}/files
// Returns a list of message files.
inline Json CategoryThreads::listMessageFile(const std::string &threads, const std::string &messages) {
    return openai_.get("threads/" + threads + "/messages/" + messages + "/files");
}

// POST https://api.openai.com/v1/threads/{thread_id}/runs
// Create a run.
inline Json CategoryThreads::createRun(const std::string &threads, Json input) {
    return openai_.post("threads/" + threads + "/runs", input);
}

// GET https://api.openai.com/v1/threads/{thread_id}/runs/{run_id}
// Retrieves a run.
inline Json CategoryThreads::retrieveRun(const std::string &threads, const std::string &runs) {
    return openai_.get("threads/" + threads + "/runs/" + runs);
}

// POST https://api.openai.com/v1/threads/{thread_id}/runs/{run_id}
// Modifies a run.
inline Json CategoryThreads::modifyRun(const std::string &threads, const std::string &runs, Json input) {
    return openai_.post("threads/" + threads + "/runs/" + runs, input);
}

// GET https://api.openai.com/v1/threads/{thread_id}/runs
// Returns a list of runs belonging to a thread.
inline Json CategoryThreads::listRun(const std::string &threads) { return openai_.get("threads/" + threads + "/runs"); }

// POST
// https://api.openai.com/v1/threads/{thread_id}/runs/{run_id}/submit_tool_outputs
// When a run has the status: "requires_action" and required_action.type is
// submit_tool_outputs, this endpoint can be used to submit the outputs from the
// tool calls once they're all completed. All outputs must be submitted in a
// single request.
inline Json CategoryThreads::submitToolOutputsToRun(const std::string &threads, const std::string &runs, Json input) {
    return openai_.post("threads/" + threads + "/runs/" + runs + "submit_tool_outputs", input);
}

// POST https://api.openai.com/v1/threads/{thread_id}/runs/{run_id}/cancel
// Cancels a run that is in_progress.
inline Json CategoryThreads::cancelRun(const std::string &threads, const std::string &runs) {
    Json input;
    return openai_.post("threads/" + threads + "/runs/" + runs + "/cancel", input);
}

// POST https://api.openai.com/v1/threads/runs
// Create a thread and run it in one request.
inline Json CategoryThreads::createThreadAndRun(Json input) { return openai_.post("threads/runs", input); }

// GET
// https://api.openai.com/v1/threads/{thread_id}/runs/{run_id}/steps/{step_id}
// Retrieves a run step.
inline Json CategoryThreads::retrieveRunStep(const std::string &threads, const std::string &runs,
                                             const std::string &steps) {
    return openai_.get("threads/" + threads + "/runs/" + runs + "/steps/" + steps);
}

// GET https://api.openai.com/v1/threads/{thread_id}/runs/{run_id}/steps
// Returns a list of run steps belonging to a run.
inline Json CategoryThreads::listRunStep(const std::string &threads, const std::string &runs) {
    return openai_.get("threads/" + threads + "/runs/" + runs + "/steps");
}

// POST https://api.openai.com/v1/completions
// Creates a completion for the provided prompt and parameters
inline Json CategoryCompletion::create(Json input) { return openai_.post("completions", input); }

// POST https://api.openai.com/v1/chat/completions
// Creates a chat completion for the provided prompt and parameters
inline Json CategoryChat::create(Json input) { return openai_.post("chat/completions", input); }

// POST https://api.openai.com/v1/audio/transcriptions
// Transcribes audio into the input language.
inline Json CategoryAudio::transcribe(Json input) {
    auto lambda = [input]() -> std::map<std::string, std::string> {
        std::map<std::string, std::string> temp;
        temp.insert({"model", input["model"].get<std::string>()});
        if (input.contains("language")) {
            temp.insert({"language", input["language"].get<std::string>()});
        }
        if (input.contains("prompt")) {
            temp.insert({"prompt", input["prompt"].get<std::string>()});
        }
        if (input.contains("response_format")) {
            temp.insert({"response_format", input["response_format"].get<std::string>()});
        }
        if (input.contains("temperature")) {
            temp.insert({"temperature", std::to_string(input["temperature"].get<float>())});
        }
        return temp;
    };
    openai_.setMultiformPart({"file", input["file"].get<std::string>()}, lambda());

    return openai_.post("audio/transcriptions", std::string {""}, "multipart/form-data");
}

// POST https://api.openai.com/v1/audio/translations
// Translates audio into into English..
inline Json CategoryAudio::translate(Json input) {
    auto lambda = [input]() -> std::map<std::string, std::string> {
        std::map<std::string, std::string> temp;
        temp.insert({"model", input["model"].get<std::string>()});
        if (input.contains("language")) {
            temp.insert({"language", input["language"].get<std::string>()});
        }
        if (input.contains("prompt")) {
            temp.insert({"prompt", input["prompt"].get<std::string>()});
        }
        if (input.contains("response_format")) {
            temp.insert({"response_format", input["response_format"].get<std::string>()});
        }
        if (input.contains("temperature")) {
            temp.insert({"temperature", std::to_string(input["temperature"].get<float>())});
        }
        return temp;
    };
    openai_.setMultiformPart({"file", input["file"].get<std::string>()}, lambda());

    return openai_.post("audio/translations", std::string {""}, "multipart/form-data");
}

// POST https://api.openai.com/v1/translations
// Creates a new edit for the provided input, instruction, and parameters
inline Json CategoryEdit::create(Json input) { return openai_.post("edits", input); }

// POST https://api.openai.com/v1/images/generations
// Given a prompt and/or an input image, the model will generate a new image.
inline Json CategoryImage::create(Json input) { return openai_.post("images/generations", input); }

// POST https://api.openai.com/v1/images/edits
// Creates an edited or extended image given an original image and a prompt.
inline Json CategoryImage::edit(Json input) {
    std::string prompt = input["prompt"].get<std::string>(); // required
    // Default values
    std::string mask = "";
    int n = 1;
    std::string size = "1024x1024";
    std::string response_format = "url";
    std::string user = "";

    if (input.contains("mask")) {
        mask = input["mask"].get<std::string>();
    }
    if (input.contains("n")) {
        n = input["n"].get<int>();
    }
    if (input.contains("size")) {
        size = input["size"].get<std::string>();
    }
    if (input.contains("response_format")) {
        response_format = input["response_format"].get<std::string>();
    }
    if (input.contains("user")) {
        user = input["user"].get<std::string>();
    }
    openai_.setMultiformPart({"image", input["image"].get<std::string>()},
                             std::map<std::string, std::string> {{"prompt", prompt},
                                                                 {"mask", mask},
                                                                 {"n", std::to_string(n)},
                                                                 {"size", size},
                                                                 {"response_format", response_format},
                                                                 {"user", user}});

    return openai_.post("images/edits", std::string {""}, "multipart/form-data");
}

// POST https://api.openai.com/v1/images/variations
// Creates a variation of a given image.
inline Json CategoryImage::variation(Json input) {
    // Default values
    int n = 1;
    std::string size = "1024x1024";
    std::string response_format = "url";
    std::string user = "";

    if (input.contains("n")) {
        n = input["n"].get<int>();
    }
    if (input.contains("size")) {
        size = input["size"].get<std::string>();
    }
    if (input.contains("response_format")) {
        response_format = input["response_format"].get<std::string>();
    }
    if (input.contains("user")) {
        user = input["user"].get<std::string>();
    }
    openai_.setMultiformPart(
        {"image", input["image"].get<std::string>()},
        std::map<std::string, std::string> {
            {"n", std::to_string(n)}, {"size", size}, {"response_format", response_format}, {"user", user}});

    return openai_.post("images/variations", std::string {""}, "multipart/form-data");
}

inline Json CategoryEmbedding::create(Json input) { return openai_.post("embeddings", input); }

inline Json CategoryFile::list() { return openai_.get("files"); }

inline Json CategoryFile::upload(Json input) {
    openai_.setMultiformPart({"file", input["file"].get<std::string>()},
                             std::map<std::string, std::string> {{"purpose", input["purpose"].get<std::string>()}});

    return openai_.post("files", std::string {""}, "multipart/form-data");
}

inline Json CategoryFile::del(const std::string &file_id) { return openai_.del("files/" + file_id); }

inline Json CategoryFile::retrieve(const std::string &file_id) { return openai_.get("files/" + file_id); }

inline Json CategoryFile::content(const std::string &file_id) { return openai_.get("files/" + file_id + "/content"); }

inline Json CategoryFineTune::create(Json input) { return openai_.post("fine-tunes", input); }

inline Json CategoryFineTune::list() { return openai_.get("fine-tunes"); }

inline Json CategoryFineTune::retrieve(const std::string &fine_tune_id) {
    return openai_.get("fine-tunes/" + fine_tune_id);
}

inline Json CategoryFineTune::content(const std::string &fine_tune_id) {
    return openai_.get("fine-tunes/" + fine_tune_id + "/content");
}

inline Json CategoryFineTune::cancel(const std::string &fine_tune_id) {
    return openai_.post("fine-tunes/" + fine_tune_id + "/cancel", Json {});
}

inline Json CategoryFineTune::events(const std::string &fine_tune_id) {
    return openai_.get("fine-tunes/" + fine_tune_id + "/events");
}

inline Json CategoryFineTune::del(const std::string &model) { return openai_.del("models/" + model); }

inline Json CategoryModeration::create(Json input) { return openai_.post("moderations", input); }

} // namespace _detail

// Public interface

using _detail::OpenAI;

// instance
using _detail::instance;
using _detail::start;

// Generic methods
using _detail::get;
using _detail::post;

// Helper categories access
using _detail::assistant;
using _detail::audio;
using _detail::chat;
using _detail::completion;
using _detail::edit;
using _detail::embedding;
using _detail::file;
using _detail::fineTune;
using _detail::image;
using _detail::model;
using _detail::moderation;
using _detail::thread;

using _detail::Json;

} // namespace openai

#endif // OPENAI_HPP_
