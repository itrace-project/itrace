#include "libitrace/record.hpp"

#include <signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

#include <argparse/argparse.hpp>
#include <sstream>
#include <vector>

#include "libitrace/utils.hpp"
#include "record.hpp"

using std::cout, std::cerr, std::endl;

std::pair<long, long> parse_ipfilter(std::string ipfilter) {
    std::stringstream ss(ipfilter);
    char delimiter = ',';
    std::vector<std::string> tokens {};
    std::string token {};
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    long start, end;
    try {
        start = std::stol(tokens[0], nullptr, 16);
        end = std::stol(tokens[1], nullptr, 16);
        if (start >= end) throw std::invalid_argument("start cannot be greater or equal to end");
    } catch (std::invalid_argument& e) {
        cerr << "Please use valid hex addresses" << endl;
        throw e;
    }

    return {start, end};
}

void record(const argparse::ArgumentParser& args) {
	std::vector<std::string> target {};
	std::string outfile {};
	try {
		target  = args.get<std::vector<std::string>>("target");
		outfile = args.get<std::string>("output");
	} catch (std::logic_error& e) {
		cerr << e.what() << "\n";
		cerr << args;
		exit(1);
	}

	if (target.empty()) {
		cerr << "Specify a target program\n";
		cerr << args << "\n";
		exit(1);
	}

	std::string program = target[0];
	std::vector<std::string> programargs(target.begin() + 1, target.end());
    libitrace::Record instance(program, programargs, outfile);

    if (args.is_used("filter-symbol")) {
        std::string symbol = args.get<std::string>("filter-symbol");
        instance.AddSymbolFilter(symbol);
    }

    if (args.is_used("filter-instr-ptr")) {
        std::string ipfilter = args.get<std::string>("filter-instr-ptr");
        auto [start, end] = parse_ipfilter(ipfilter);
        instance.AddInstrPtrFilter(start, end);
    }

	if (args.is_used("pid")) {
		pid_t pid = args.get<int>("pid");
		pid_t perfpid = instance.Attach(pid);

		cout << "Press any key to stop trace" << endl;
		fd_set readfds;
		FD_SET(STDIN_FILENO, &readfds);
		if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL) == -1)
			die("select");

		kill(perfpid, SIGINT);
		int stat_loc {};
		if (waitpid(perfpid, &stat_loc, 0) == -1) die("waitpid");

	} else if (!target.empty()) {
		instance.Run();

	} else {
		cerr << "Sepcify target program\n";
		cerr << args;
		exit(1);
	}
}
