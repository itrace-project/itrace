#include "export.hpp"

#include <cstdlib>

#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"

using std::cerr, std::endl;

void exporter(const argparse::ArgumentParser& args) {
	const char* FILTER_PATH = std::getenv("DLFILTER_PATH");
	if (!FILTER_PATH) {
		cerr << "DLFILTER_PATH not set. Run ./setup.py --export and set "
		        "DLFILTER_PATH env var"
		     << endl;
		exit(1);
	}

	std::string filter_path(FILTER_PATH);
	std::string infile {};
	std::string outfile {};
	try {
		infile  = args.get<std::string>("input");
		outfile = args.get<std::string>("output");
	} catch (std::logic_error& e) {
		cerr << e.what() << "\n";
		cerr << args;
		exit(1);
	}

	libitrace::arglist perfargs = {"script",    "-i",      infile,  "--itrace=bei0ns", "--dlfilter",
	                               filter_path, "--dlarg", outfile, "--dlarg",         "t"};
	libitrace::print_perf_args(perfargs);

	libitrace::Subprocess perfscript {"perf", perfargs};
	auto res = perfscript.Run();
	if (!res) throw std::runtime_error("Error decoding trace data");
	if (res->Exit != 0) throw std::runtime_error(res->Stderr);
}
