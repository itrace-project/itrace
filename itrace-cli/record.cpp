#include "libitrace/record.hpp"

#include <signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

#include <argparse/argparse.hpp>
#include <sstream>
#include <vector>

#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"
#include "record.hpp"

using std::cout, std::cerr, std::endl;

std::optional<std::pair<long, long>> parse_ipfilter(std::string ipfilter) {
	std::stringstream ss(ipfilter);
	char delimiter = ',';
	std::vector<std::string> tokens {};
	std::string token {};
	while (std::getline(ss, token, delimiter)) { tokens.push_back(token); }
	if (tokens.size() != 2) return std::nullopt;

	long start, end;
	try {
		start = std::stol(tokens[0], nullptr, 16);
		end   = std::stol(tokens[1], nullptr, 16);
	} catch (std::invalid_argument& e) { return std::nullopt; }

	return std::make_pair(start, end);
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

	if (args.is_used("snapshot")) instance.SetSnapshotMode();

	if (args.is_used("filter-symbol")) {
		std::string symbol = args.get<std::string>("filter-symbol");
		instance.AddSymbolFilter(symbol);
	}

	if (args.is_used("filter-instr-ptr")) {
		std::string ipfilter = args.get<std::string>("filter-instr-ptr");
		auto parsed          = parse_ipfilter(ipfilter);
		if (!parsed) {
			cerr << "Provide a valid instruction range <start>,<end> in hex" << endl;
			cerr << args << endl;
			exit(1);
		}
		auto [start, end] = *parsed;
		instance.AddInstrPtrFilter(start, end);
	}

	if (args.is_used("pid") && args.is_used("snapshot")) {
		// attach with snapshotting
		pid_t pid                         = args.get<int>("pid");
		libitrace::RunningProcess context = instance.Attach(pid);

		char c {};
		cout << "Enter [s] to take a snapshot, [q] to quit" << endl;
		while (std::cin.get(c)) {
			if (c == 's') {
				instance.TakeSnapshot(context);
				sleep(1);
			} else if (c == 'q') {
				break;
			} else {
				cout << "Enter [s] to take a snapshot, [q] to quit" << endl;
			}
		}

		kill(context.Pid, SIGINT);
		auto res = libitrace::Subprocess::Wait(context, true);
		if (!res || res->Exit != 0) {
			cerr << "Error waiting for attached process" << endl;
			if (res) cerr << res->Stderr << endl;
			exit(1);
		}

	} else if (args.is_used("pid")) {
		// attach without snapshotting
		pid_t pid                         = args.get<int>("pid");
		libitrace::RunningProcess context = instance.Attach(pid);

		cout << "Press [ENTER] to stop trace" << endl;
		fd_set readfds;
		FD_SET(STDIN_FILENO, &readfds);
		if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL) == -1) die("select");

		kill(context.Pid, SIGINT);
		auto res = libitrace::Subprocess::Wait(context, true);
		if (!res || res->Exit != 0) {
			cerr << "Error waiting for attached process" << endl;
			if (res) cerr << res->Stderr << endl;
			exit(1);
		}

	} else if (!target.empty()) {
		// run as child
		instance.Run();

	} else {
		cerr << "Sepcify target program\n";
		cerr << args;
		exit(1);
	}
}
