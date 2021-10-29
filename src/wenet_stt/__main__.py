#
# This file is part of wenet_stt_python.
# (c) Copyright 2021 by David Zurow
# Licensed under the AGPL-3.0; see LICENSE file.
#

import argparse

from . import _name, WenetSTT, MODEL_DOWNLOADS
from .utils import download_model

def main():
    parser = argparse.ArgumentParser(prog='python -m %s' % _name)
    subparsers = parser.add_subparsers(dest='command', help='sub-command')
    subparser = subparsers.add_parser('decode', help='Decode one or more WAV files')
    subparser.add_argument('model_dir', help='Model directory to use')
    subparser.add_argument('wav_file', nargs='+', help='WAV file(s) to decode')
    subparser = subparsers.add_parser('download', help='Download a model to decode with')
    subparser.add_argument('model', nargs='*', help='Model name(s) to download (will also be the output directory)')
    args = parser.parse_args()

    if args.command == 'decode':
        import wave
        wenet_stt = WenetSTT(WenetSTT.build_config(args.model_dir))
        for wave_file_path in args.wav_file:
            with wave.open(wave_file_path, 'rb') as wav_file:
                wav_data = wav_file.readframes(wav_file.getnframes())
                print(wenet_stt.decode(wav_data))

    elif args.command == 'download':
        if not args.model:
            print("List of available models:")
            for name in MODEL_DOWNLOADS:
                print(name)
        else:
            for name in args.model:
                if name not in MODEL_DOWNLOADS:
                    print("Model '%s' not found" % name)
                    continue
                download_model(name, verbose=True)

    else:
        parser.print_help()

if __name__ == '__main__':
    main()
