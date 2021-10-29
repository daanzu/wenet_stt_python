#
# This file is part of wenet_stt_python.
# (c) Copyright 2021 by David Zurow
# Licensed under the AGPL-3.0; see LICENSE file.
#

import os, subprocess, tempfile, wave

import pytest

from wenet_stt import WenetSTT, MODEL_DOWNLOADS

test_model_path = os.path.join(os.path.dirname(__file__), 'model')
test_missing_model_path = os.path.join(os.path.dirname(__file__), 'missing_model')
test_wav_path = os.path.join(os.path.dirname(__file__), 'test.wav')


@pytest.fixture
def decoder():
    return WenetSTT(WenetSTT.build_config(test_model_path))

@pytest.fixture
def wav_samples():
    with wave.open(test_wav_path, 'rb') as wav_file:
        data = wav_file.readframes(wav_file.getnframes())
    return data


def test_missing_model():
    assert not os.path.exists(test_missing_model_path)
    with pytest.raises(FileNotFoundError):
        WenetSTT(WenetSTT.build_config(test_missing_model_path))

def test_init(decoder):
    pass

def test_destruct(decoder):
    del decoder

def test_decode(decoder, wav_samples):
    assert decoder.decode(wav_samples).strip().lower() == 'it depends on the context'


class TestCLI:

    def test_decode(self):
        process = subprocess.run(f'python -m wenet_stt decode {test_model_path} {test_wav_path}', shell=True, check=True, capture_output=True)
        assert process.stdout.decode().strip().lower() == 'it depends on the context'

    def test_decode_multiple(self):
        process = subprocess.run(f'python -m wenet_stt decode {test_model_path} {test_wav_path} {test_wav_path}', shell=True, check=True, capture_output=True)
        assert process.stdout.decode().strip().lower().splitlines() == ['it depends on the context'] * 2

    def test_download_list(self):
        process = subprocess.run(f'python -m wenet_stt download', shell=True, check=True, capture_output=True)
        assert process.stdout.decode().strip().splitlines() == ["List of available models:"] + list(MODEL_DOWNLOADS.keys())

    @pytest.mark.download
    def test_download_actual(self):
        test_model_name = list(MODEL_DOWNLOADS.keys())[0]
        with tempfile.TemporaryDirectory() as tmpdir:
            process = subprocess.run(f'python -m wenet_stt download {test_model_name}', shell=True, check=True, capture_output=False, cwd=tmpdir)
            assert os.path.isdir(os.path.join(tmpdir, test_model_name))
            for filename in 'final.zip train.yaml words.txt'.split():
                assert os.path.isfile(os.path.join(tmpdir, test_model_name, filename))
            test_model_path = os.path.join(tmpdir, test_model_name)

            process = subprocess.run(f'python -m wenet_stt decode {test_model_path} {test_wav_path}', shell=True, check=True, capture_output=True)
            assert process.stdout.decode().strip().lower() == 'it depends on the context'


@pytest.mark.download
def test_download_api():
    test_model_name = list(MODEL_DOWNLOADS.keys())[0]
    with tempfile.TemporaryDirectory() as tmpdir:
        assert WenetSTT.download_model_if_not_exists(test_model_name, parent_dir=tmpdir)
        assert os.path.isdir(os.path.join(tmpdir, test_model_name))
        for filename in 'final.zip train.yaml words.txt'.split():
            assert os.path.isfile(os.path.join(tmpdir, test_model_name, filename))
