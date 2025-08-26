#include <argparse/argparse.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "libitrace/subprocess.hpp"

using std::cout;

bool intelpt_available() {
	libitrace::Subprocess p {"perf list | grep intel_pt", {}};
	auto res {p.Run()};
    if (!res) return false;
	if (res->Exit == 0 && !res->Stdout.empty()) return true;
	return false;
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("itrace", "0.0.1");
	program.add_argument("target_program").help("Program to trace");

	std::vector<std::string> target_args {
	    program.parse_known_args(std::vector<std::string>(argv, argv + argc))
	};

	std::string target_program {program.get<std::string>("target_program")};

	if (!intelpt_available()) {
		cout << "Intel PT unavailable\n";
		cout << "Check list of processors that support Intel PT: "
		     << "https://www.intel.com/content/www/us/en/support/articles/"
		        "000056730/processors.html\n";
	}
}
