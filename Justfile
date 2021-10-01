
_default:
	just --list
	just --summary

build:
	python setup.py bdist_wheel

build-manylinux:
	building/dockcross-manylinux2014-x64 bash building/build-wheel-dockcross.sh manylinux2014_x86_64

reinstall-wheel filename:
	python -m pip uninstall -y wenet-stt && python -m pip install {{filename}}

test args="":
	pytest {{args}} tests/

clean:
	rm -rf _skbuild/ native/wenet/runtime/server/x86/fc_base/
