#!/usr/bin/env bash

set -e -x

PYTHON_EXE=/opt/python/cp38-cp38/bin/python
WHEEL_PLAT=$1

if [ -z "$WHEEL_PLAT" ] || [ -z "$PYTHON_EXE" ]; then
    echo "ERROR: variable not set!"
    exit 1
fi

# $PYTHON_EXE -m pip install -f https://download.pytorch.org/whl/torch_stable.html torch==1.9.0+cpu
$PYTHON_EXE -m pip install -r requirements-build.txt
# export TORCH_CMAKE_PREFIX_PATH=$($PYTHON_EXE -c "import torch;print(torch.utils.cmake_prefix_path)")
# export TORCH_CMAKE_PREFIX_PATH=$(readlink -m _skbuild/linux-x86_64-3.8/libtorch/share/cmake)
$PYTHON_EXE setup.py bdist_wheel

mkdir -p wheelhouse
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TORCH_CMAKE_PREFIX_PATH/../../lib
for whl in dist/*.whl; do auditwheel repair $whl --plat $WHEEL_PLAT -w wheelhouse/; done
