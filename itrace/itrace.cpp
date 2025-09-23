#include <fcntl.h>

#include <argparse/argparse.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"

using std::cout;

bool intelpt_available() {
	// libitrace::Subprocess p {"perf list | grep intel_pt", {}};
	libitrace::Subprocess p {
	    "perf", {"list", "|", "grep", "intel_pt"}
	};
	auto res {p.Run()};
	if (!res) return false;
	// cout << res->Stdout;
	if (res->Exit == 0 && !res->Stdout.empty()) return true;
	return false;
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("itrace", "0.0.1");
	program.add_argument("target_program").help("Program to trace");

	std::vector<std::string> target_args {};
	try {
		target_args = program.parse_known_args(
		    std::vector<std::string>(argv, argv + argc)
		);
	} catch (const std::exception& err) {
		cout << err.what();
		std::cerr << program;
		exit(1);
	}

	std::string target_program {program.get<std::string>("target_program")};

	if (!intelpt_available()) {
		cout << "Intel PT unavailable\n";
		cout << "Check list of processors that support Intel PT: "
		     << "https://www.intel.com/content/www/us/en/support/articles/"
		        "000056730/processors.html\n";
		exit(1);
	}

	libitrace::arglist args = {"record", "-e", "intel_pt//u", target_program};
	args.insert(args.end(), target_args.begin(), target_args.end());

	libitrace::Subprocess perf_record {"perf", args};
	auto res {perf_record.Run()};
	if (!res || res->Exit != 0) die2(res->Stderr.c_str());

	args = {"script", "--insn-trace", "--xed"};
	libitrace::Subprocess perf_script {"perf", args};

	// set to everyone rw but umask will mask it to something different
	int fd = open("test.trace", O_RDWR | O_CREAT, 666);
	if (fd == -1) die("open");
	perf_script.SetStdout(fd);

	res = perf_script.Run();
	if (!res || res->Exit != 0) die2(res->Stderr.c_str());
}
