#include "export.hpp"

#include <unistd.h>

#include <cstdlib>

#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"

using std::cerr, std::endl;

void exporter(const argparse::ArgumentParser& args) {
	if (access(FILTER_PATH, F_OK) != 0) {
		cerr << "libperf2perfetto.so not found in /usr/local/lib. Run ./setup.py --export --install"
		     << endl;
		exit(1);
	}

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

	libitrace::arglist perfargs = {"script",     "-i",
	                               infile,       "--itrace=bei0ns",
	                               "--dlfilter", std::string(FILTER_PATH),
	                               "--dlarg",    outfile,
	                               "--dlarg",    "t"};
	libitrace::print_perf_args(perfargs);

	libitrace::Subprocess perfscript {"perf", perfargs};
	auto res = perfscript.Run();
	if (!res) throw std::runtime_error("Error decoding trace data");
	if (res->Exit != 0) throw std::runtime_error(res->Stderr);
}
