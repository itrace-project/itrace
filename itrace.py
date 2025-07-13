import argparse
import os
import subprocess

custom_env = os.environ.copy()
custom_env["PATH"] = os.path.abspath("bin") + os.pathsep + custom_env["PATH"]

def check_intel_pt() -> bool:
    result = subprocess.run(
        "perf list | grep intel_pt", shell=True, capture_output=True, text=True
    )
    if result.returncode == 0:
        if result.stdout != "":
            return True
    return False

def perf_record(target: str) -> str:
    perf = ["perf", "record"]
    perf_event = ["-e", "intel_pt//u"]
    perf_output = ["-o", f"{target}.data"]
    target_program = [target]
    cmd = [*perf, *perf_event, *perf_output, "--", *target_program]

    stdout = open(f"{target}-output", "w")
    subprocess.run(
        cmd,
        stdout=stdout,
        check=True
    )
    return f"{target}.data"

def perf_script(data_file: str):
    perf = ["perf", "script"]
    perf_input = ["-i", data_file]
    perf_options = ["--insn-trace", "--xed"]
    cmd = [*perf, *perf_input, *perf_options]

    subprocess.run(
        cmd,
        env=custom_env,
        check=True
    )

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
    perf_record_output = perf_record(args.target_program)
    perf_script(perf_record_output)
