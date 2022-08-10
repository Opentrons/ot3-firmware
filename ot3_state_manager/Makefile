python := poetry run python
pytest := poetry run py.test


tests ?= tests
test_opts ?=  --cov=ot3_state_manager --cov-report term-missing:skip-covered --cov-report xml:coverage.xml

# test modules to typecheck
tests_to_typecheck := \
	tests/

# Defined separately than the clean target so the wheel file doesn’t have to
# depend on a PHONY target
clean_cmd = rm -rf build dist .coverage coverage.xml '*.egg-info' '__pycache__' '*.pyc' '.mypy_cache' '.pytest_cache'

.PHONY: setup
setup:
	poetry env use 3.8 && poetry update && poetry install

.PHONY: teardown
teardown:
	poetry env info -p | xargs basename | xargs poetry env remove

.PHONY: build
build:
	poetry build

.PHONY: generate-header-file
generate-header-file:
	poetry run python3 generate_state_manager_headers.py $(PWD)/../include/common/simulation/ot3_state_manager_messages.hpp

.PHONY: clean
clean:
	$(clean_cmd)

.PHONY: test
test:
	$(pytest) $(tests) $(test_opts)

.PHONY: lint
lint:
	$(python) -m mypy ot3_state_manager $(tests_to_typecheck) generate_state_manager_headers.py
	$(python) -m isort --check ot3_state_manager tests generate_state_manager_headers.py
	$(python) -m black --check --line-length 88 ot3_state_manager tests generate_state_manager_headers.py
	$(python) -m flake8 ot3_state_manager tests generate_state_manager_headers.py
	$(python) -m mdformat --check README.md

.PHONY: format
format:
	$(python) -m isort ot3_state_manager tests generate_state_manager_headers.py
	$(python) -m black --line-length 88 ot3_state_manager tests generate_state_manager_headers.py
	$(python) -m mdformat README.md
