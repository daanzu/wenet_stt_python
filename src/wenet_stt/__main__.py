#
# This file is part of wenet_stt_python.
# (c) Copyright 2021 by David Zurow
# Licensed under the AGPL-3.0; see LICENSE file.
#

import argparse

from . import _name, WenetSTT, MODEL_DOWNLOADS

DOWNLOAD_CHUNK_SIZE = 1 * 1024 * 1024

def download_model(name, url, verbose):
    import os, zipfile
    from urllib.request import urlopen

    filename = name + '.zip'
    if os.path.exists(filename):
        raise FileExistsError(filename)
    if os.path.exists(name):
        raise FileExistsError(name)

    if verbose:
        print("Downloading model '%s'..." % name)
    with urlopen(url) as response:
        with open(filename, 'wb') as f:
            response_length = int(response.getheader('Content-Length'))  # Don't trust response.length!
            bytes_read = 0
            report_percentages = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
            for chunk in iter(lambda: response.read(DOWNLOAD_CHUNK_SIZE), b''):
                f.write(chunk)
                bytes_read += len(chunk)
                if verbose:
                    print('.', end='', flush=True)
                    while report_percentages and bytes_read >= report_percentages[0] * response_length / 100:
                        print(' %d%% ' % report_percentages.pop(0), end='', flush=True)

    if verbose:
        print("Done!")
        print("Extracting...")
    with zipfile.ZipFile(filename, 'r') as zip_file:
        zip_file.extractall()
    if verbose:
        print("Done!")
        print("Removing zip file...")
    os.remove(filename)
    if verbose:
        print("Done!")

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
                download_model(name, MODEL_DOWNLOADS[name], True)

    else:
        parser.print_help()

if __name__ == '__main__':
    main()
