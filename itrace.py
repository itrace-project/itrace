import argparse
import os
import subprocess

from typing import List
from pathlib import Path

custom_env = os.environ.copy()
custom_env["PATH"] = os.path.abspath("bin") + os.pathsep + custom_env["PATH"]

PROJECT_ROOT = Path().cwd()
TRACE_DATA = PROJECT_ROOT / "trace"

def check_intel_pt() -> bool:
    result = subprocess.run(
        "perf list | grep intel_pt", shell=True, capture_output=True, text=True
    )
    if result.returncode == 0:
        if result.stdout != "":
            return True
    return False

def perf_record(target: str, target_args: List[str]) -> Path:
    trace_output = TRACE_DATA / f"{target}.data"

    perf = ["perf", "record"]
    perf_event = ["-e", "intel_pt//u"]
    perf_output = ["-o", trace_output.as_posix()]
    target_program = [target] + target_args
    cmd = [*perf, *perf_event, *perf_output, "--", *target_program]

    subprocess.run(
        cmd,
        check=True
    )
    return trace_output

def perf_script(trace_output: Path):
    perf = ["perf", "script"]
    perf_input = ["-i", trace_output.as_posix()]
    perf_options = ["--insn-trace", "--xed"]
    cmd = [*perf, *perf_input, *perf_options]

    subprocess.run(
        cmd,
        env=custom_env,
    )

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="itrace")
    parser.add_argument("target_program", help="Program to trace")
    parser.add_argument("target_args", nargs=argparse.REMAINDER, help="Arguments for the target program")

    args = parser.parse_args()

    print(args.target_args)

    if not check_intel_pt():
        print("Intel PT unavailable")
        print(
            "Check list of processors that support Intel PT: "
            "https://www.intel.com/content/www/us/en/support/articles/000056730/processors.html"
        )
        exit(1)
    
    os.makedirs(TRACE_DATA, exist_ok=True)

    print(f"Profiling: {args.target_program} {' '.join(args.target_args)}")
    perf_record_output = perf_record(args.target_program, args.target_args)
    perf_script(perf_record_output)
