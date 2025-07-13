#!/usr/bin/python

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

    # Allow users other than root to use Intel PT
    with open("/etc/sysctl.conf", 'a') as f:
        f.write("kernel.perf_event_paranoid=-1\n")

    print("Setup complete!")
