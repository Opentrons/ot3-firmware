"""This script is used to get the versions of the submodules."""

import re
import sys
import json
import argparse
import subprocess
import os

VERSION_REGEX = re.compile("v([0-9])")
SUBSYSTEMS = [
    "head",
    "gantry-x",
    "gantry-y",
    "gripper",
    "pipettes-single",
    "pipettes-multi",
    "pipettes-96",
]

def find_files(hexdir):
    for direntry in os.scandir(hexdir):
        if not direntry.name.endswith('.hex'):
            print(f'ignoring file {direntry.name}, not a hex file', file=sys.stderr)
        nameparts = direntry.name.split('.')[0].split('-')
        revision = nameparts[-1]
        subsystem = '-'.join(nameparts[:-1])
        if subsystem in SUBSYSTEMS:
            yield (subsystem, revision, direntry.name)
        else:
            print(f'ignoring file {direntry.name}, not in subsystems', file=sys.stderr)

def main(args):
    version_info = {}
    subsystems = args.subsystem or SUBSYSTEMS
    hexdir = args.hex_dir
    try:
        version = subprocess.check_output(["git", "describe", "--tags", "--always", "--match=v*"]).decode().strip()
        # version should be int
        version = int(re.search(VERSION_REGEX, version).group(1)) if re.search(VERSION_REGEX, version) else 0
        shortsha = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"]).decode().strip()
        branch = subprocess.check_output(["git", "rev-parse", "--abbrev-ref", "HEAD"]).decode().strip()
    except Exception as e:
        print(f"Could not get the version info {e}", file=sys.stderr)
        exit(1)

    files = {k: {} for k in SUBSYSTEMS}

    for subsystem, revision, name in find_files(hexdir):
        files[subsystem][revision] = os.path.join(args.prefix, name)

    for subsystem in subsystems:
        if subsystem not in SUBSYSTEMS:
            print(f"Unknown subsystem {subsystem}", file=sys.stderr)
            continue

        # put the version_info together
        version_info[subsystem] = {
            "version": version,
            "shortsha": shortsha,
            "files_by_revision": files[subsystem],
            "branch": branch,
        }


    manifest = {'manifest_version': 1, 'subsystems': version_info}
    json.dump(manifest, args.output)

    return version_info


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Creates a json file of the submodule version info.")
    parser.add_argument(
        "--subsystem",
        nargs="*",
        help="subsystem to generate file for; leave blank for all.",
    )
    parser.add_argument(
        '--hex-dir',
        default='dist/applications',
        help='Directory for the hex files to find'
    )
    parser.add_argument(
        "output", metavar='OUTPUT',
        type=argparse.FileType('w'),
        default=sys.stdout,
        help="saves json output to given output path or stdout."
    )
    parser.add_argument(
        '--prefix', '-p',
        type=str,
        default='',
        help='Define a prefix for the hex file paths on the target'
    )
    args = parser.parse_args()
    main(args)
