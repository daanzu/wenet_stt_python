//
// This file is part of wenet_stt_python.
// (c) Copyright 2021 by David Zurow
// Licensed under the AGPL-3.0; see LICENSE file.
//

#include <torch/script.h>

#include "decoder/params.h"
#include "utils/log.h"
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


std::shared_ptr<wenet::FeaturePipelineConfig> InitFeaturePipelineConfigFromJson(const nlohmann::json& j) {
    return std::make_shared<wenet::FeaturePipelineConfig>(
        j.at("num_bins").get<int>(),
        j.at("sample_rate").get<int>()
    );
}

std::shared_ptr<wenet::FeaturePipelineConfig> InitFeaturePipelineConfigFromSimpleJson(const nlohmann::json& j) {
    auto num_bins = (j.contains("num_bins")) ? j.at("num_bins").get<int>() : FLAGS_num_bins;
    auto sample_rate = (j.contains("sample_rate")) ? j.at("sample_rate").get<int>() : FLAGS_sample_rate;
    return std::make_shared<wenet::FeaturePipelineConfig>(num_bins, sample_rate);
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

std::shared_ptr<wenet::DecodeOptions> InitDecodeOptionsFromSimpleJson(const nlohmann::json& j) {
    if (!j.is_object()) LOG(FATAL) << "decode_options must be a valid JSON object";
    auto decode_config = std::make_shared<wenet::DecodeOptions>();
    if (j.contains("chunk_size")) { j.at("chunk_size").get_to(decode_config->chunk_size); } else { decode_config->chunk_size = FLAGS_chunk_size; }
    if (j.contains("num_left_chunks")) { j.at("num_left_chunks").get_to(decode_config->num_left_chunks); } else { decode_config->num_left_chunks = FLAGS_num_left_chunks; }
    if (j.contains("ctc_weight")) { j.at("ctc_weight").get_to(decode_config->ctc_weight); } else { decode_config->ctc_weight = FLAGS_ctc_weight; }
    if (j.contains("rescoring_weight")) { j.at("rescoring_weight").get_to(decode_config->rescoring_weight); } else { decode_config->rescoring_weight = FLAGS_rescoring_weight; }
    if (j.contains("reverse_weight")) { j.at("reverse_weight").get_to(decode_config->reverse_weight); } else { decode_config->reverse_weight = FLAGS_reverse_weight; }
    if (j.contains("max_active")) { j.at("max_active").get_to(decode_config->ctc_wfst_search_opts.max_active); } else { decode_config->ctc_wfst_search_opts.max_active = FLAGS_max_active; }
    if (j.contains("min_active")) { j.at("min_active").get_to(decode_config->ctc_wfst_search_opts.min_active); } else { decode_config->ctc_wfst_search_opts.min_active = FLAGS_min_active; }
    if (j.contains("beam")) { j.at("beam").get_to(decode_config->ctc_wfst_search_opts.beam); } else { decode_config->ctc_wfst_search_opts.beam = FLAGS_beam; }
    if (j.contains("lattice_beam")) { j.at("lattice_beam").get_to(decode_config->ctc_wfst_search_opts.lattice_beam); } else { decode_config->ctc_wfst_search_opts.lattice_beam = FLAGS_lattice_beam; }
    if (j.contains("acoustic_scale")) { j.at("acoustic_scale").get_to(decode_config->ctc_wfst_search_opts.acoustic_scale); } else { decode_config->ctc_wfst_search_opts.acoustic_scale = FLAGS_acoustic_scale; }
    if (j.contains("blank_skip_thresh")) { j.at("blank_skip_thresh").get_to(decode_config->ctc_wfst_search_opts.blank_skip_thresh); } else { decode_config->ctc_wfst_search_opts.blank_skip_thresh = FLAGS_blank_skip_thresh; }
    if (j.contains("nbest")) { j.at("nbest").get_to(decode_config->ctc_wfst_search_opts.nbest); } else { decode_config->ctc_wfst_search_opts.nbest = FLAGS_nbest; }
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

std::shared_ptr<wenet::DecodeResource> InitDecodeResourceFromSimpleJson(const nlohmann::json& j) {
    if (!j.is_object()) LOG(FATAL) << "decode_resource must be a valid JSON object";
    auto resource = std::make_shared<wenet::DecodeResource>();

    auto model_path = j.at("model_path").get<std::string>();
    auto num_threads = (j.contains("num_threads")) ? j.at("num_threads").get<int>() : FLAGS_num_threads;
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

static bool one_time_initialized_ = false;

struct WenetSTTModel {
    std::shared_ptr<wenet::FeaturePipelineConfig> feature_config_;
    std::shared_ptr<wenet::DecodeOptions> decode_config_;
    std::shared_ptr<wenet::DecodeResource> decode_resource_;

    WenetSTTModel(const std::string& config_json_str) {
        if (!one_time_initialized_) {
            one_time_initialized_ = true;
            google::InitGoogleLogging("WenetSTT");
        }

        if (!config_json_str.empty()) {
            auto config_json = nlohmann::json::parse(config_json_str);
            if (!config_json.is_object()) LOG(FATAL) << "config_json_str must be a valid JSON object";
            feature_config_ = InitFeaturePipelineConfigFromSimpleJson(config_json);
            decode_config_ = InitDecodeOptionsFromSimpleJson(config_json);
            decode_resource_ = InitDecodeResourceFromSimpleJson(config_json);
        }
    }

    int sample_rate() const { return feature_config_->sample_rate; }
    bool is_streaming() const { return decode_config_->chunk_size > 0; }

    std::string DecodeUtterance(const std::vector<float>& wav_samples) {
        auto feature_pipeline = std::make_shared<wenet::FeaturePipeline>(*feature_config_);
        feature_pipeline->AcceptWaveform(wav_samples);
        feature_pipeline->set_input_finished();
        LOG(INFO) << "Num frames: " << feature_pipeline->num_frames();
        wenet::TorchAsrDecoder decoder(feature_pipeline, decode_resource_, *decode_config_);

        int wav_duration = wav_samples.size() / sample_rate();
        int decode_time = 0;
        while (true) {
            wenet::Timer timer;
            wenet::DecodeState state = decoder.Decode();
            if (state == wenet::DecodeState::kEndFeats) {
                decoder.Rescoring();
            }
            int chunk_decode_time = timer.Elapsed();
            decode_time += chunk_decode_time;
            if (decoder.DecodedSomething()) {
                LOG(INFO) << "Partial result: " << decoder.result()[0].sentence;
            }
            if (state == wenet::DecodeState::kEndFeats) {
                break;
            }
        }
        std::string hypothesis;
        if (decoder.DecodedSomething()) {
            hypothesis = decoder.result()[0].sentence;
        }
        LOG(INFO) << "Final result: " << hypothesis;
        LOG(INFO) << "Decoded " << wav_duration << "ms audio taking " << decode_time << "ms. RTF: " << std::setprecision(4) << static_cast<float>(decode_time) / wav_duration;

        // Strip any trailing whitespace
        auto last_pos = hypothesis.find_last_not_of(' ');
        hypothesis = hypothesis.substr(0, last_pos + 1);

        return hypothesis;
    }
};


extern "C" {
#include "wenet_stt_lib.h"
}

void *wenet_stt__construct_model(const char *config_json_cstr) {
    BEGIN_INTERFACE_CATCH_HANDLER
    auto model = new WenetSTTModel(config_json_cstr);
    return model;
    END_INTERFACE_CATCH_HANDLER(nullptr)
}

bool wenet_stt__destruct_model(void *model_vp) {
    BEGIN_INTERFACE_CATCH_HANDLER
    auto model = static_cast<WenetSTTModel*>(model_vp);
    delete model;
    return true;
    END_INTERFACE_CATCH_HANDLER(false)
}

bool wenet_stt__decode_utterance(void *model_vp, float *wav_samples, int32_t wav_samples_len, char *text, int32_t text_max_len) {
    BEGIN_INTERFACE_CATCH_HANDLER
    auto model = static_cast<WenetSTTModel*>(model_vp);
    auto hypothesis = model->DecodeUtterance(std::vector<float>(wav_samples, wav_samples + wav_samples_len));
    auto cstr = hypothesis.c_str();
    strncpy(text, cstr, text_max_len);
    text[text_max_len - 1] = 0;  // Just in case.
    return true;
    END_INTERFACE_CATCH_HANDLER(false)
}
