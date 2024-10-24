#!/usr/bin/env python


import argparse as ap
import os.path as p
import os
import sys
import shutil
import subprocess

known_builds = ["debug", "release", "clang-debug", "clang-release", "sanitizers"]

PATH = "PATH"
CURRENT_PATH = os.getenv(PATH)

CURRENT_ENV = os.environ.copy()

BUILD_ENV = {
    "debug": CURRENT_ENV,
    "release": CURRENT_ENV,
    "clang-debug": CURRENT_ENV | {"CC": "clang", "CXX": "clang++"},
    "clang-release": CURRENT_ENV | {"CC": "clang", "CXX": "clang++"},
    "sanitizers": CURRENT_ENV,
}

RELEASE = "--buildtype=release"
DEBUG = "--buildtype=debug"
MESON = "meson"

BUILD_FLAGS = {
    "debug": ["--werror", "-Ddocumentation=false"] + [DEBUG],
    "release": ["--werror"] + [RELEASE],
    "clang-debug": ["--werror", "-Ddocumentation=false"] + [DEBUG],
    "clang-release": ["--werror"] + [RELEASE],
    "sanitizers": [
        "--werror",
        "-Ddocumentation=false",
        "-Db_sanitize=address,undefined",
    ]
    + [DEBUG],
}


def configure_build(folder: known_builds) -> int:
    """configures a build folder"""
    if folder not in known_builds:
        raise ValueError(f"Oops: {folder} is not a know build")
    return subprocess.run(
        [MESON, "setup", folder] + BUILD_FLAGS[folder], env=BUILD_ENV[folder]
    ).returncode


def build_folder(folder) -> int:
    """build the current build in the folder"""
    run_result = subprocess.run(
        [MESON, "compile", "-C", folder],
        env=BUILD_ENV[folder],
    )
    return run_result.returncode


def test_folder(folder) -> int:
    """Runs the unit tests"""
    run_result = subprocess.run(
        [MESON, "devenv", "-C", folder, MESON, "test"],
    )
    return run_result.returncode


def configure_build_and_test(folder: str) -> int:
    if not p.isdir(folder):
        ret = configure_build(folder)
        if ret != 0:
            print(f"Oops unable to configure: {folder}", file=sys.stderr)
            exit(ret)
        ret = build_folder(folder)
        if ret != 0:
            print(f"Oops unable to build: {folder}", file=sys.stderr)
            exit(ret)

    return test_folder(folder)


if __name__ == "__main__":
    parser = ap.ArgumentParser("test.py", description="run one or multiple tests")
    parser.add_argument(
        "builds", nargs="*", default=["debug"], help="specify the folder of the build"
    )

    np = parser.parse_args()

    builds = np.builds
    if builds == ["all"]:
        builds = known_builds

    # when people use tab completion on there terminal they might get debug/
    # instead of debug.
    for i, build in enumerate(builds):
        no_trailing_sep = build.removesuffix(os.sep)
        if build != no_trailing_sep:
            print(f"Replacing build {build} with {no_trailing_sep}", file=sys.stderr)
            builds[i] = no_trailing_sep

    for build in builds:
        ret = 0
        if build in known_builds:
            ret = configure_build_and_test(build)
            if ret != 0:
                exit(ret)
        else:
            if p.isfile(p.join(".", build, "build.ninja")):
                test_folder(build)
            else:
                print(f"{p.join('.', build, 'build.ninja')} is a file", file=sys.stderr)
                exit(f"{build} doesn't seem to be a build directory")

    print(f"Builds: {builds} completed")
