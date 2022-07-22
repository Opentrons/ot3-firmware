OT_PYTHON ?= python
pipenv_envvars := $(and $(CI),PIPENV_IGNORE_VIRTUALENVS=1)
pipenv := $(pipenv_envvars) $(OT_PYTHON) -m pipenv
python := $(pipenv) run python
pip := $(pipenv) run pip
pytest := $(pipenv) run py.test
pipenv_opts := --dev
SHX := npx shx


tests ?= tests
test_opts ?=  --cov=ot3_state_manager --cov-report term-missing:skip-covered --cov-report xml:coverage.xml

# test modules to typecheck
tests_to_typecheck := \
	tests/

# Defined separately than the clean target so the wheel file doesn’t have to
# depend on a PHONY target
clean_cmd = $(SHX) rm -rf build dist .coverage coverage.xml '*.egg-info' '**/__pycache__' '**/*.pyc' '**/.mypy_cache'

.PHONY: setup
setup:
	$(pipenv) sync $(pipenv_opts)
	$(pipenv) run pip freeze

.PHONY: clean
clean:
	$(clean_cmd)

.PHONY: teardown
teardown:
	$(pipenv) --rm

.PHONY: test
test:
	$(pytest) $(tests) $(test_opts)

.PHONY: lint
lint:
	$(python) -m mypy ot3_state_manager $(tests_to_typecheck)
	$(python) -m isort --check ot3_state_manager tests setup.py
	$(python) -m black --check --line-length 88 ot3_state_manager tests setup.py
	$(python) -m flake8 ot3_state_manager tests setup.py

.PHONY: format
format:
	$(python) -m isort ot3_state_manager tests setup.py
	$(python) -m black --line-length 88 ot3_state_manager tests setup.py
