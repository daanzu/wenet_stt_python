
_default:
	just --list
	just --summary

build:
	rm -rf dist/* || true
	python setup.py bdist_wheel

build-and-reinstall-wheel:
	rm -rf dist/* || true
	python setup.py bdist_wheel
	just reinstall-wheel

build-type type="Debug":
	python setup.py bdist_wheel --build-type={{type}}

build-manylinux:
	building/dockcross-manylinux2014-x64 bash building/build-wheel-dockcross.sh manylinux2014_x86_64

clean:
	rm -rf _skbuild/ native/wenet/runtime/server/x86/fc_base/ dist/*

reinstall-wheel filename="dist/*":
	python -m pip uninstall -y wenet-stt && python -m pip install {{filename}}

test args="":
	pytest {{args}} tests/

publish-wheels filenames="wheels/*":
	python -m twine upload {{filenames}}
