#!/usr/bin/env bash

set -e -x

PYTHON_EXE=/opt/python/cp38-cp38/bin/python
WHEEL_PLAT=$1

if [ -z "$WHEEL_PLAT" ] || [ -z "$PYTHON_EXE" ]; then
    echo "ERROR: variable not set!"
    exit 1
fi

$PYTHON_EXE -m pip install -r requirements-build.txt
export WENET_LINUX_C11ABI=0
$PYTHON_EXE setup.py bdist_wheel

mkdir -p wheelhouse
for whl in dist/*.whl; do auditwheel repair $whl --plat $WHEEL_PLAT -w wheelhouse/; done
