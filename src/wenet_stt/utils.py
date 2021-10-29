#
# This file is part of wenet_stt_python.
# (c) Copyright 2021 by David Zurow
# Licensed under the AGPL-3.0; see LICENSE file.
#

from . import MODEL_DOWNLOADS

DOWNLOAD_CHUNK_SIZE = 1 * 1024 * 1024

def download_model(name, url=None, parent_dir='.', verbose=False):
    import os, zipfile
    from urllib.request import urlopen

    if url is None:
        url = MODEL_DOWNLOADS[name]

    filename = os.path.join(parent_dir, name + '.zip')
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
        zip_file.extractall(parent_dir)
    if verbose:
        print("Done!")
        print("Removing zip file...")
    os.remove(filename)
    if verbose:
        print("Done!")
