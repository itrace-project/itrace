#!/usr/bin/python3

import argparse
import os
import shutil
import sys
import subprocess


def check_intel_pt() -> bool:
    result = subprocess.run(
        "perf list | grep intel_pt", shell=True, capture_output=True, text=True
    )
    if result.returncode == 0:
        if result.stdout != "":
            return True
    return False

def build_xed():
    git_clone = ["git", "clone"]

    if not os.path.exists("deps/xed"):
        subprocess.run(
            [*git_clone, "https://github.com/intelxed/xed.git", "deps/xed"],
            check=True,
            text=True,
        )

    if not os.path.exists("deps/mbuild"):
        subprocess.run(
            [*git_clone, "https://github.com/intelxed/mbuild.git", "deps/mbuild"],
            check=True,
            text=True,
        )
        
    if not os.path.exists("bin/xed"):
        subprocess.run([sys.executable, "mfile.py", "examples"], cwd="deps/xed")
        shutil.copy2("deps/xed/obj/wkit/bin/xed", "bin/xed")

    subprocess.run(["bin/xed", "-version"], check=True)

def build_perf2perfetto():
    if not os.path.exists("deps/perf2perfetto"):
        subprocess.run(
            ["git", "clone", "https://github.com/michoecho/perf2perfetto.git", "deps/perf2perfetto"],
            check=True
        )

    if not os.path.exists("bin/libperf2perfetto.so"):
        subprocess.run(
            ["cargo", "build", "--release"],
            cwd="deps/perf2perfetto"
        )
        shutil.copy2("deps/perf2perfetto/target/release/libperf2perfetto.so", "bin/libperf2perfetto.so")

def build_itrace():
    subprocess.run(["cmake", "-DCMAKE_INSTALL_PREFIX=/usr/local/bin",".."], cwd="build", check=True)
    subprocess.run(["make", "-j", str(os.cpu_count())], cwd="build", check=True)
    subprocess.run(["./itrace", "--help"], cwd="build")

def install():
    shutil.copy2("build/itrace", "/usr/local/bin")
    shutil.copy2("bin/xed", "/usr/local/bin")
    if os.path.exists("bin/libperf2perfetto.so"):
        shutil.copy2("bin/libperf2perfetto.so", "/usr/local/lib")

if __name__ == "__main__":
    print("Setting up itrace...")

    if not check_intel_pt():
        print("Intel PT unavailable")
        print(
            "Check list of processors that support Intel PT: "
            "https://www.intel.com/content/www/us/en/support/articles/000056730/processors.html"
        )
        exit(1)

    parser = argparse.ArgumentParser(description="setup")
    parser.add_argument(
        "--clean",
        help="Force a full installation and rebuild of dependencies and source",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--export",
        help="Build dependency needed for exporting trace to Perfetto. Need Rust and cargo",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--install",
        help="Install the binaries. Requires sudo",
        action="store_true",
        default=False
    )

    args = parser.parse_args()

    if args.clean:
        shutil.rmtree("deps")
        shutil.rmtree("bin")
        shutil.rmtree("build")

    subprocess.run(["mkdir", "-p", "deps"])
    subprocess.run(["mkdir", "-p", "bin"])
    subprocess.run(["mkdir", "-p", "build"])

    # Install and build Intel x86 Encoder/Decoder (Intel xed)
    build_xed()
    build_itrace()
    if args.export:
        build_perf2perfetto()

    if args.install:
        install()

    print("Setup complete!")
