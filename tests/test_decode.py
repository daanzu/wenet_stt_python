#
# This file is part of wenet_stt_python.
# (c) Copyright 2021 by David Zurow
# Licensed under the AGPL-3.0; see LICENSE file.
#

import os, subprocess, tempfile, wave

import pytest

from wenet_stt import WenetSTTModel, WenetSTTDecoder, MODEL_DOWNLOADS

test_model_path = os.path.join(os.path.dirname(__file__), 'model')
test_missing_model_path = os.path.join(os.path.dirname(__file__), 'missing_model')
test_wav_path = os.path.join(os.path.dirname(__file__), 'test.wav')


@pytest.fixture
def model():
    return WenetSTTModel(WenetSTTModel.build_config(test_model_path))

@pytest.fixture
def model_factory():
    return lambda config={}: WenetSTTModel(WenetSTTModel.build_config(test_model_path, config=config))

@pytest.fixture
def decoder_factory():
    return lambda config={}: WenetSTTDecoder(WenetSTTModel(WenetSTTModel.build_config(test_model_path, config=config)))

@pytest.fixture
def wav_samples():
    with wave.open(test_wav_path, 'rb') as wav_file:
        data = wav_file.readframes(wav_file.getnframes())
    return data


def test_missing_model():
    assert not os.path.exists(test_missing_model_path)
    with pytest.raises(FileNotFoundError):
        WenetSTTModel(WenetSTTModel.build_config(test_missing_model_path))

def test_init(model):
    pass

def test_destruct(model):
    del model

def test_decode(model, wav_samples):
    assert model.decode(wav_samples).lower() == 'it depends on the context'

def test_decode_multithreaded(model_factory, wav_samples):
    assert model_factory(dict(num_threads=2)).decode(wav_samples).lower() == 'it depends on the context'

def test_decode_streaming(decoder_factory, wav_samples):
    chunks = [wav_samples[i:i+1024] for i in range(0, len(wav_samples), 1024)]
    assert len(chunks) > 2
    decoder = decoder_factory()
    for i, chunk in enumerate(chunks):
        if i == 1:
            text, final = decoder.get_result()
            assert final == False
            assert text == ''
        finalize = bool(i == len(chunks) - 1)
        decoder.decode(chunk, finalize)
        if not finalize:
            text, final = decoder.get_result()
            assert final == False
    text, final = decoder.get_result(True)
    assert final == True
    assert text.lower() == 'it depends on the context'


class TestCLI:

    def test_decode(self):
        process = subprocess.run(f'python3 -m wenet_stt decode {test_model_path} {test_wav_path}', shell=True, check=True, capture_output=True)
        assert process.stdout.decode().strip().lower() == 'it depends on the context'

    def test_decode_multiple(self):
        process = subprocess.run(f'python3 -m wenet_stt decode {test_model_path} {test_wav_path} {test_wav_path}', shell=True, check=True, capture_output=True)
        assert process.stdout.decode().strip().lower().splitlines() == ['it depends on the context'] * 2

    def test_download_list(self):
        process = subprocess.run(f'python3 -m wenet_stt download', shell=True, check=True, capture_output=True)
        assert process.stdout.decode().strip().splitlines() == ["List of available models:"] + list(MODEL_DOWNLOADS.keys())

    @pytest.mark.download
    def test_download_actual(self):
        test_model_name = list(MODEL_DOWNLOADS.keys())[0]
        with tempfile.TemporaryDirectory() as tmpdir:
            process = subprocess.run(f'python3 -m wenet_stt download {test_model_name}', shell=True, check=True, capture_output=False, cwd=tmpdir)
            assert os.path.isdir(os.path.join(tmpdir, test_model_name))
            for filename in 'final.zip train.yaml words.txt'.split():
                assert os.path.isfile(os.path.join(tmpdir, test_model_name, filename))
            test_model_path = os.path.join(tmpdir, test_model_name)

            process = subprocess.run(f'python3 -m wenet_stt decode {test_model_path} {test_wav_path}', shell=True, check=True, capture_output=True)
            assert process.stdout.decode().strip().lower() == 'it depends on the context'


@pytest.mark.download
def test_download_api():
    test_model_name = list(MODEL_DOWNLOADS.keys())[0]
    with tempfile.TemporaryDirectory() as tmpdir:
        assert WenetSTTModel.download_model_if_not_exists(test_model_name, parent_dir=tmpdir)
        assert os.path.isdir(os.path.join(tmpdir, test_model_name))
        for filename in 'final.zip train.yaml words.txt'.split():
            assert os.path.isfile(os.path.join(tmpdir, test_model_name, filename))
