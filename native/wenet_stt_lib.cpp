//
// This file is part of wenet_stt_python.
// (c) Copyright 2021 by David Zurow
// Licensed under the AGPL-3.0; see LICENSE file.
//

#include <torch/script.h>

#include "decoder/params.h"
// #include "frontend/wav.h"
#include "utils/log.h"
// #include "utils/string.h"
#include "utils/timer.h"
#include "utils/utils.h"

#include "nlohmann/json.hpp"

#define BEGIN_INTERFACE_CATCH_HANDLER \
    try {
#define END_INTERFACE_CATCH_HANDLER(expr) \
    } catch(const std::exception& e) { \
        std::cerr << "Trying to survive fatal exception: " << e.what(); \
        return (expr); \
    }

namespace wenet {
    // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FeaturePipelineConfig,
    //     num_bins,
    //     sample_rate,
    //     frame_length,
    //     frame_shift
    // );
    // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DecodeOptions,
    //     chunk_size,
    //     num_left_chunks,
    //     ctc_weight,
    //     rescoring_weight,
    //     reverse_weight,
    //     ctc_endpoint_config,
    //     ctc_prefix_search_opts,
    //     ctc_wfst_search_opts
    // );
    // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CtcEndpointConfig,
    //     blank,
    //     blank_threshold,
    //     rule1,
    //     rule2,
    //     rule3
    // );
    void from_json(const nlohmann::json& j, CtcEndpointConfig& c) {
        if (j.contains("blank")) j.at("blank").get_to(c.blank);
        if (j.contains("blank_threshold")) j.at("blank_threshold").get_to(c.blank_threshold);
        // if (j.contains("rule1")) j.at("rule1").get_to(c.rule1);
        // if (j.contains("rule2")) j.at("rule2").get_to(c.rule2);
        // if (j.contains("rule3")) j.at("rule3").get_to(c.rule3);
    }
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CtcEndpointRule,
        must_decoded_sth,
        min_trailing_silence,
        min_utterance_length
    );
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CtcPrefixBeamSearchOptions,
        blank,
        first_beam_size,
        second_beam_size
    );
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CtcWfstBeamSearchOptions,
        max_active,
        min_active,
        beam,
        lattice_beam,
        acoustic_scale,
        nbest,
        blank_skip_thresh
    );
    // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DecodeResource);
}

// using namespace wenet;

std::shared_ptr<wenet::FeaturePipelineConfig> InitFeaturePipelineConfigFromJson(const nlohmann::json& j) {
    return std::make_shared<wenet::FeaturePipelineConfig>(
        j.at("num_bins").get<int>(),
        j.at("sample_rate").get<int>()
    );
}

std::shared_ptr<wenet::DecodeOptions> InitDecodeOptionsFromJson(const nlohmann::json& j) {
    if (!j.is_object()) LOG(FATAL) << "decode_options must be a valid JSON object";
    auto decode_config = std::make_shared<wenet::DecodeOptions>();
    if (j.contains("chunk_size")) j.at("chunk_size").get_to(decode_config->chunk_size);
    if (j.contains("num_left_chunks")) j.at("num_left_chunks").get_to(decode_config->num_left_chunks);
    if (j.contains("ctc_weight")) j.at("ctc_weight").get_to(decode_config->ctc_weight);
    if (j.contains("rescoring_weight")) j.at("rescoring_weight").get_to(decode_config->rescoring_weight);
    if (j.contains("reverse_weight")) j.at("reverse_weight").get_to(decode_config->reverse_weight);
    if (j.contains("ctc_endpoint_config")) j.at("ctc_endpoint_config").get_to(decode_config->ctc_endpoint_config);
    if (j.contains("ctc_prefix_search_opts")) j.at("ctc_prefix_search_opts").get_to(decode_config->ctc_prefix_search_opts);
    if (j.contains("ctc_wfst_search_opts")) j.at("ctc_wfst_search_opts").get_to(decode_config->ctc_wfst_search_opts);
    return decode_config;
}

std::shared_ptr<wenet::DecodeResource> InitDecodeResourceFromJson(const nlohmann::json& j) {
    if (!j.is_object()) LOG(FATAL) << "decode_resource must be a valid JSON object";
    auto resource = std::make_shared<wenet::DecodeResource>();

    auto model_path = j.at("model_path").get<std::string>();
    auto num_threads = j.at("num_threads").get<int>();
    LOG(INFO) << "Reading model " << model_path << " to use " << num_threads << " threads";
    auto model = std::make_shared<wenet::TorchAsrModel>();
    model->Read(model_path, num_threads);
    resource->model = model;

    std::shared_ptr<fst::Fst<fst::StdArc>> fst = nullptr;
    if (j.contains("fst_path")) {
        auto fst_path = j.at("fst_path").get<std::string>();
        LOG(INFO) << "Reading fst " << fst_path;
        fst.reset(fst::Fst<fst::StdArc>::Read(fst_path));
        CHECK(fst != nullptr);
    }
    resource->fst = fst;

    auto dict_path = j.at("dict_path").get<std::string>();
    LOG(INFO) << "Reading symbol table " << dict_path;
    auto symbol_table = std::shared_ptr<fst::SymbolTable>(
        fst::SymbolTable::ReadText(dict_path));
    resource->symbol_table = symbol_table;

    std::shared_ptr<fst::SymbolTable> unit_table = nullptr;
    if (j.contains("unit_path")) {
        auto unit_path = j.at("unit_path").get<std::string>();
        LOG(INFO) << "Reading unit table " << unit_path;
        unit_table = std::shared_ptr<fst::SymbolTable>(
            fst::SymbolTable::ReadText(unit_path));
        CHECK(unit_table != nullptr);
    } else if (fst == nullptr) {
        LOG(INFO) << "Using symbol table as unit table";
        unit_table = symbol_table;
    }
    resource->unit_table = unit_table;

    return resource;
}

struct WenetSTTModel {
    std::shared_ptr<wenet::FeaturePipelineConfig> feature_config_;
    std::shared_ptr<wenet::DecodeOptions> decode_config_;
    std::shared_ptr<wenet::DecodeResource> decode_resource_;

    WenetSTTModel(const std::string& config_json_str) {
        google::InitGoogleLogging("WenetSTT");

        if (!config_json_str.empty()) {
            auto config_json = nlohmann::json::parse(config_json_str);
            if (!config_json.is_object()) LOG(FATAL) << "config_json_str must be a valid JSON object";
            for (const auto& it : config_json.items()) {
                if (it.key() == "feature_pipeline_config") {
                    feature_config_ = InitFeaturePipelineConfigFromJson(it.value());
                } else if (it.key() == "decode_options") {
                    decode_config_ = InitDecodeOptionsFromJson(it.value());
                } else if (it.key() == "decode_resource") {
                    decode_resource_ = InitDecodeResourceFromJson(it.value());
                } else {
                    LOG(WARNING) << "unrecognized config key: " << it.key() << " = " << it.value();
                }
            }
        }
    }

};

extern "C" {
#include "wenet_stt_lib.h"
}

void *wenet_stt__construct(const char *config_json_cstr) {
    BEGIN_INTERFACE_CATCH_HANDLER
    auto model = new WenetSTTModel(config_json_cstr);
    return model;
    END_INTERFACE_CATCH_HANDLER(nullptr)
}

bool wenet_stt__destruct(void *model_vp) {
    BEGIN_INTERFACE_CATCH_HANDLER
    auto model = static_cast<WenetSTTModel*>(model_vp);
    delete model;
    return true;
    END_INTERFACE_CATCH_HANDLER(false)
}

bool wenet_stt__decode(void *model_vp, float *wav_samples, int32_t wav_samples_len, char *text, int32_t text_max_len) {
    BEGIN_INTERFACE_CATCH_HANDLER
    auto model = static_cast<WenetSTTModel*>(model_vp);
    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    auto wav_tensor = torch::from_blob(wav_samples, {1, wav_samples_len}, options);

    std::string hypothesis;  // FIXME!!!

    // Strip any trailing whitespace
    auto last_pos = hypothesis.find_last_not_of(' ');
    hypothesis = hypothesis.substr(0, last_pos + 1);

    auto cstr = hypothesis.c_str();
    strncpy(text, cstr, text_max_len);
    text[text_max_len - 1] = 0;  // Just in case.
    return true;
    END_INTERFACE_CATCH_HANDLER(false)
}
