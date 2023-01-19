"""This script is used to get the versions of the submodules."""

import re
import sys
import argparse
import subprocess

VERSION_REGEX = re.compile("v([0-9])")
SUBSYSTEMS = [
    "head",
    "gantry-x",
    "gantry-y",
    "gripper",
    "pipettes-single",
    "pipettes-multi",
    "pipettes-96",
    "pippetes-384",
]


def main(args):
    version_info = {}
    for subsystem in SUBSYSTEMS:
        if args.target == "all" or subsystem in args.target:
            try:
                version = (
                    subprocess.check_output(
                        ["git", "describe", "--tags", "--always", "--match=v*"]
                    )
                    .decode()
                    .strip()
                )
                # version should be int
                version = int(re.search(VERSION_REGEX, version).group(1)) if re.search(VERSION_REGEX, version) else 0
                shortsha = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"]).decode().strip()
                branch = subprocess.check_output(["git", "rev-parse", "--abbrev-ref", "HEAD"]).decode().strip()
            except Exception as e:
                print(f"Could not get the version info for {subsystem}", file=sys.stderr)
                exit(1)

            # put the version_info together
            version_info[subsystem] = {
                "version": version,
                "shortsha": shortsha,
                "branch": branch,
            }
    return version_info


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Creates a json file of the submodule version info.")
    parser.add_argument(
        "--target",
        default="all",
        nargs="+",
        choices=SUBSYSTEMS,
        help="subsystem to generate file for; leave blank for all.",
    )
    args = parser.parse_args()
    print(main(args))
