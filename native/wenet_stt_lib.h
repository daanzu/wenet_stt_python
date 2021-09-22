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

WENET_STT_API void *wenet_stt__construct(const char *config_json_cstr);
WENET_STT_API bool wenet_stt__destruct(void *model_vp);
WENET_STT_API bool wenet_stt__decode(void *model_vp, float *wav_samples, int32_t wav_samples_len, char *text, int32_t text_max_len);
