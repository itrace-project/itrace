#include <unistd.h>

#include <argparse/argparse.hpp>
#include <iostream>
#include <string>

#include "decode.hpp"
#include "libitrace/subprocess.hpp"
#include "record.hpp"

using std::cerr;

bool intelpt_available() {
	libitrace::Subprocess p {
	    "perf", {"list", "|", "grep", "intel_pt"}
	};
	auto res {p.Run()};
	if (res && res->Exit == 0 && !res->Stdout.empty()) return true;
	return false;
}

void parseargs(
    int argc, char** argv, argparse::ArgumentParser& recordargs,
    argparse::ArgumentParser& decodeargs, argparse::ArgumentParser& program
) {
	recordargs.add_description("Record the trace of a program");
	recordargs.add_argument("target")
	    .help("Target program and its arguments")
	    .nargs(argparse::nargs_pattern::any);
	recordargs.add_argument("-o", "--output")
	    .help("Output file of trace")
	    .default_value(std::string("itrace.data"));
	recordargs.add_argument("-p", "--pid")
	    .help("Process id to attach to")
	    .scan<'i', int>();
	recordargs.add_argument("-s", "--filter-symbol")
	    .help("Symbol to filter trace data on");
	recordargs.add_argument("-a", "--filter-instr-ptr")
	    .help(
	        "Instruction pointer addresses to filter trace data on. Formatted "
	        "as <start>,<end> where addresses are in hex"
	    );

	decodeargs.add_description("Decode a trace into human readable form");
	decodeargs.add_argument("-i", "--input")
	    .help("Path to .data trace file")
	    .default_value(std::string("itrace.data"));
	decodeargs.add_argument("-o", "--output")
	    .help("Output file of trace")
	    .default_value(std::string("itrace.trace"));

	program.add_subparser(recordargs);
	program.add_subparser(decodeargs);

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception& err) {
		cerr << err.what();
		cerr << program;
		exit(1);
	}
}

int main(int argc, char** argv) {
	if (!intelpt_available()) {
		cerr << "Intel PT unavailable\n";
		cerr << "Check list of processors that support Intel PT: "
		     << "https://www.intel.com/content/www/us/en/support/articles/"
		        "000056730/processors.html\n";
		exit(1);
	}

	argparse::ArgumentParser program("itrace", "0.0.1");
	argparse::ArgumentParser recordargs("record");
	argparse::ArgumentParser decodeargs("decode");
	parseargs(argc, argv, recordargs, decodeargs, program);

	if (program.is_subcommand_used("record")) {
		record(recordargs);
	} else if (program.is_subcommand_used("decode")) {
		decode(decodeargs);
	} else {
		cerr << "Unknown subcommand\n";
		cerr << program.help().str();
		exit(1);
	}
}
