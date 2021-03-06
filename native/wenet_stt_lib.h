//
// This file is part of wenet_stt_python.
// (c) Copyright 2021 by David Zurow
// Licensed under the AGPL-3.0; see LICENSE file.
//

#pragma once

#if defined(_MSC_VER)
    #ifdef WENET_STT_EXPORTS
        #define WENET_STT_API extern "C" __declspec(dllexport)
    #else
        #define WENET_STT_API extern "C" __declspec(dllimport)
    #endif
#elif defined(__GNUC__)
    // unnecessary
    #define WENET_STT_API extern "C" __attribute__((visibility("default")))
#else
    #define WENET_STT_API
    #pragma warning Unknown dynamic link import / export semantics.
#endif

#include <cstdint>

WENET_STT_API void *wenet_stt__construct_model(const char *config_json_cstr);
WENET_STT_API bool wenet_stt__destruct_model(void *model_vp);
WENET_STT_API bool wenet_stt__decode_utterance(void *model_vp, float *wav_samples, int32_t wav_samples_len, char *text, int32_t text_max_len);

WENET_STT_API void *wenet_stt__construct_decoder(void *model_vp);
WENET_STT_API bool wenet_stt__destruct_decoder(void *decoder_vp);
WENET_STT_API bool wenet_stt__decode(void *decoder_vp, float *wav_samples, int32_t wav_samples_len, bool finalize);
WENET_STT_API bool wenet_stt__get_result(void *decoder_vp, char *text, int32_t text_max_len, bool *final_p);
WENET_STT_API bool wenet_stt__reset(void *decoder_vp);
