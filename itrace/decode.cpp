#include "libitrace/decode.hpp"
#include "decode.hpp"

using std::cerr;

void decode(const argparse::ArgumentParser& args) {
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

    libitrace::Decode instance(infile, outfile);
    instance.Run();
}

