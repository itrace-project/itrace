import subprocess
import argparse


def check_intel_pt() -> bool:
    result = subprocess.run(
        "perf list | grep intel_pt", shell=True, capture_output=True, text=True
    )
    if result.returncode == 0:
        if result.stdout != "":
            return True
    return False


if __name__ == "__main__":
    if not check_intel_pt():
        print("Intel PT unavailable")
        print(
            "Check list of processors that support Intel PT: "
            "https://www.intel.com/content/www/us/en/support/articles/000056730/processors.html"
        )
        exit(1)

    parser = argparse.ArgumentParser(description="itrace")
    parser.add_argument("target_program", help="Program to trace")

    args = parser.parse_args()

    print(f"Profiling: {args.target_program}")

    perf = ["perf", "record"]
    perf_event = ["-e", "intel_pt//u"]
    perf_output = ["-o", f"{args.target_program}.data"]
    target_program = [args.target_program]
    cmd = [*perf, *perf_event, *perf_output, "--", *target_program]

    stdout = open(f"{args.target_program}-output", "w")
    subprocess.run(
        cmd,
        stdout=stdout,
        check=True
    )
