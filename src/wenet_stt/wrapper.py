#
# This file is part of wenet_stt_python.
# (c) Copyright 2021 by David Zurow
# Licensed under the AGPL-3.0; see LICENSE file.
#

import json, os, re, sys

from cffi import FFI
import numpy as np

if sys.platform.startswith('win'): _platform = 'windows'
elif sys.platform.startswith('linux'): _platform = 'linux'
elif sys.platform.startswith('darwin'): _platform = 'macos'
else: raise Exception("unknown sys.platform")

_ffi = FFI()
_library_directory_path = os.path.dirname(os.path.abspath(__file__))
_library_binary_path = os.path.join(_library_directory_path,
    dict(windows='wenet_stt_lib.dll', linux='libwenet_stt_lib.so', macos='libwenet_stt_lib.dylib')[_platform])
_c_source_ignore_regex = re.compile(r'(\b(extern|WENET_STT_API)\b)|("C")|(//.*$)', re.MULTILINE)  # Pattern for extraneous stuff to be removed

def encode(text):
    """ For C interop: encode unicode text -> binary utf-8. """
    return text.encode('utf-8')
def decode(binary):
    """ For C interop: decode binary utf-8 -> unicode text. """
    return binary.decode('utf-8')

class FFIObject(object):

    def __init__(self):
        self.init_ffi()

    @classmethod
    def init_ffi(cls):
        cls._lib = _ffi.init_once(cls._init_ffi, cls.__name__ + '._init_ffi')

    @classmethod
    def _init_ffi(cls):
        _ffi.cdef(_c_source_ignore_regex.sub(' ', cls._library_header_text))
        # On Windows, we need to temporarily prepend the PATH with the directory containing the DLLs to ensure that the DLLs are found.
        if _platform == 'windows':
            os.environ['PATH'] = os.pathsep.join([_library_directory_path, os.environ['PATH']])
        try:
            return _ffi.dlopen(_library_binary_path)
        finally:
            if _platform == 'windows':
                os.environ['PATH'] = os.pathsep.join(os.environ['PATH'].split(os.pathsep)[1:])

class WenetSTT(FFIObject):

    _library_header_text = """
        WENET_STT_API void *wenet_stt__construct(const char *config_json_cstr);
        WENET_STT_API bool wenet_stt__destruct(void *model_vp);
        WENET_STT_API bool wenet_stt__decode(void *model_vp, float *wav_samples, int32_t wav_samples_len, char *text, int32_t text_max_len);
    """

    def __init__(self, config):
        if not isinstance(config, dict):
            raise TypeError("config must be a dict")
        assert 'model_path' in config
        if not os.path.exists(config['model_path']):
            raise FileNotFoundError("model_path does not exist")
        assert 'dict_path' in config
        if not os.path.exists(config['dict_path']):
            raise FileNotFoundError("dict_path does not exist")

        super().__init__()
        result = self._lib.wenet_stt__construct(encode(json.dumps(config)))
        if result == _ffi.NULL:
            raise Exception("wenet_stt__construct failed")
        self._model = result

    def __del__(self):
        if hasattr(self, '_model'):
            result = self._lib.wenet_stt__destruct(self._model)
            if not result:
                raise Exception("wenet_stt__destruct failed")

    @classmethod
    def build_config(cls, model_dir=None, config=None):
        if config is None:
            config = dict()
        if not isinstance(config, dict):
            raise TypeError("config must be a dict or None")
        if model_dir is not None:
            config['model_path'] = os.path.join(model_dir, 'final.zip')
            config['dict_path'] = os.path.join(model_dir, 'words.txt')
        return config

    def decode(self, wav_samples, text_max_len=1024):
        if not isinstance(wav_samples, np.ndarray): wav_samples = np.frombuffer(wav_samples, np.int16)
        wav_samples = wav_samples.astype(np.float32)
        wav_samples_char = _ffi.from_buffer(wav_samples)
        wav_samples_float = _ffi.cast('float *', wav_samples_char)
        text_p = _ffi.new('char[]', text_max_len)

        result = self._lib.wenet_stt__decode(self._model, wav_samples_float, len(wav_samples), text_p, text_max_len)
        if not result:
            raise Exception("wenet_stt__decode failed")

        text = decode(_ffi.string(text_p))
        if len(text) >= (text_max_len - 1):
            raise Exception("text may be too long")
        return text
