# Inspired by:
# https://hynek.me/articles/sharing-your-labor-of-love-pypi-quick-and-dirty/
import codecs
import os
import os.path
import sys

from setuptools import find_packages, setup

# make stdout blocking since Travis sets it to nonblocking
if os.name == "posix":
    import fcntl

    flags = fcntl.fcntl(sys.stdout, fcntl.F_GETFL)
    fcntl.fcntl(sys.stdout, fcntl.F_SETFL, flags & ~os.O_NONBLOCK)

HERE = os.path.abspath(os.path.dirname(__file__))


VERSION = "0.0.1"
DISTNAME = "ot3_state_manager"
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
    "Programming Language :: Python :: 3.10",
    "Topic :: Scientific/Engineering",
]
KEYWORDS = ["robots", "protocols", "synbio", "pcr", "automation", "lab"]
DESCRIPTION = "OT3 State Manager"
PACKAGES = find_packages(where="ot3_state_manager", exclude=["tests.*", "tests"])
INSTALL_REQUIRES = []


def read(*parts):
    """
    Build an absolute path from *parts* and and return the contents of the
    resulting file.  Assume UTF-8 encoding.
    """
    with codecs.open(os.path.join(HERE, *parts), "rb", "utf-8") as f:
        return f.read()


setup(
    python_requires=">=3.10",
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
    long_description=__doc__,
    packages=PACKAGES,
    package_dir={"": "ot3_state_manager"},
    package_data={"ot3_state_manager": ["py.typed"]},
    zip_safe=False,
    classifiers=CLASSIFIERS,
    install_requires=INSTALL_REQUIRES,
    include_package_data=True,
)
