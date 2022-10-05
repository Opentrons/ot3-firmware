# OT-3 State Manager

The OT-3 State Manager is an intermediary service that maintains the overall state of the
OT-3, provides intercommunication between the OT-3's peripherals, and can be externally queried
to retrieve the state.

## Required Software

- [Poetry](https://python-poetry.org/docs/#installation)
- Python 3.8
  - I use [pyenv](https://github.com/pyenv/pyenv)

## Dependencies

The `state_manager` depends on the `api` and `hardware` project from the
[opentrons](https://github.com/Opentrons/opentrons) repo. When creating a virtualenv, poetry
expects the `opentrons` repo to be a sibiling to `ot3-firmware`. If the `opentrons` repo does
not exist as a sibling to `ot3-firmware`, build the project will fail.

**Example Structure:**

```
├── repos
    ├── ot3-firmware
    ├── opentrons
```

## Setup

To setup the state_manager project, navigate to `ot3-firmware/state_manager`
and run `make setup`

## Teardown

To teardown the virtualenv created by poetry, navigate to `ot3-firmware/state_manager`
and run `make teardown`

## Tests

To run unittests for project, navigate to `ot3-firmware/state_manager`
and run `make test`

## Formatting

To run formatting, navigate to `ot3-firmware/state_manager`
and run `make format`

## Lint

To run linting, navigate to `ot3-firmware/state_manager`
and run `make lint`
