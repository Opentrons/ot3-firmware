"""Setup script."""
# Inspired by:
# https://hynek.me/articles/sharing-your-labor-of-love-pypi-quick-and-dirty/
import codecs
import os
import os.path
from setuptools import setup, find_packages


HERE = os.path.abspath(os.path.dirname(__file__))

VERSION = "0.1.0"

DISTNAME = "opentrons_ot3_firmware"
LICENSE = "Apache 2.0"
AUTHOR = "Opentrons"
EMAIL = "engineering@opentrons.com"
URL = "https://github.com/Opentrons/ot3-firmware"
DOWNLOAD_URL = ""
CLASSIFIERS = [
    "Development Status :: 5 - Production/Stable",
    "Environment :: Console",
    "Operating System :: OS Independent",
    "Intended Audience :: Science/Research",
    "Programming Language :: Python",
    "Programming Language :: Python :: 3",
    "Programming Language :: Python :: 3.7",
    "Topic :: Scientific/Engineering",
]
KEYWORDS = ["robots", "protocols", "synbio", "pcr", "automation", "lab"]
DESCRIPTION = "Firmware message bindings for Opentrons OT-3s."
PACKAGES = find_packages(where=".", exclude=["tests.*", "tests"])
INSTALL_REQUIRES = []


def read(*parts: str) -> str:
    """Build an absolute path from parts and return the contents of the resulting file.

    Assume UTF-8 encoding.
    """
    with codecs.open(os.path.join(HERE, *parts), "rb", "utf-8") as f:
        return f.read()


if __name__ == "__main__":
    setup(
        python_requires=">=3.7",
        name=DISTNAME,
        description=DESCRIPTION,
        license=LICENSE,
        url=URL,
        version=VERSION,
        author=AUTHOR,
        author_email=EMAIL,
        maintainer=AUTHOR,
        maintainer_email=EMAIL,
        keywords=KEYWORDS,
        long_description=read("README.md"),
        packages=PACKAGES,
        zip_safe=False,
        classifiers=CLASSIFIERS,
        install_requires=INSTALL_REQUIRES,
        include_package_data=True,
        entry_points={
            "console_scripts": [
                "opentrons_generate_header = opentrons_ot3_firmware.scripts.generate_header:main",  # noqa: E501
            ]
        },
    )
