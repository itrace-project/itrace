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


if __name__ == "__main__":
    print("Setting up itace...")

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

    args = parser.parse_args()

    if args.clean:
        shutil.rmtree("deps")
        shutil.rmtree("bin")

    custom_env = os.environ.copy()
    custom_env["PATH"] = os.path.abspath("bin") + os.pathsep + custom_env["PATH"]

    # Install and build Intel x86 Encoder/Decoder (Intel xed)
    subprocess.run(["mkdir", "-p", "deps"])
    subprocess.run(["mkdir", "-p", "bin"])

    git_clone = ["git", "clone"]

    if not os.path.exists("bin/xed"):
        subprocess.run(
            [*git_clone, "https://github.com/intelxed/xed.git", "deps/xed"],
            check=True,
            text=True,
        )
        subprocess.run(
            [*git_clone, "https://github.com/intelxed/mbuild.git", "deps/mbuild"],
            check=True,
            text=True,
        )
        subprocess.run([sys.executable, "mfile.py", "examples"], cwd="deps/xed")

        shutil.copy2("deps/xed/obj/wkit/bin/xed", "bin/xed")

    subprocess.run(["xed", "-version"], env=custom_env, check=True)

    print("Setup complete!")
