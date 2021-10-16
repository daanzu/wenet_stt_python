#
# This file is part of wenet_stt_python.
# (c) Copyright 2021 by David Zurow
# Licensed under the AGPL-3.0; see LICENSE file.
#

_name = 'wenet_stt'
__version__ = '0.2.0'
__author__ = 'David Zurow'
__license__ = 'AGPL-3.0'

MODEL_DOWNLOADS = {
    'gigaspeech_20210728_u2pp_conformer': 'https://github.com/daanzu/wenet_stt_python/releases/download/models/gigaspeech_20210728_u2pp_conformer.zip',
    'gigaspeech_20210811_conformer_bidecoder': 'https://github.com/daanzu/wenet_stt_python/releases/download/models/gigaspeech_20210811_conformer_bidecoder.zip'
}

from .wrapper import WenetSTT
