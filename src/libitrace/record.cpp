#include "libitrace/record.hpp"

#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include <sstream>

#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"

namespace libitrace {

void Record::Run() {
	if (perfargs_.program.empty()) {
		std::cerr << "Specify a target program when tracing with Run()\n";
		exit(1);
	}

	auto args = build_arglist_();
	print_perf_args(args);

	Subprocess perfrecord {"perf", args};
	auto res {perfrecord.Run()};
	if (!res) throw std::runtime_error("Error starting perf record instance");
	if (res->Exit != 0) throw std::runtime_error(res->Stderr);
}

RunningProcess Record::Attach(pid_t pid) {
	perfargs_.pid = pid;

	auto args = build_arglist_();
	print_perf_args(args);
	Subprocess perfrecord {"perf", args};

	auto res {perfrecord.Popen()};
	if (!res) throw std::runtime_error("Error starting perf record instance");

	return *res;
}

void Record::AddSymbolFilter(std::string symbol) {
	perfargs_.symbol = symbol;
	perfargs_.filter = true;
}

void Record::AddInstrPtrFilter(long start, long end) {
	if (end <= start)
		throw std::runtime_error("Provide a valid instruction range <start>,<end> in hex");

	long size = end - start;
	std::stringstream ss {};
	ss << "0x" << std::hex << start << "/0x" << size;
	perfargs_.instrptr_range = ss.str();
	perfargs_.filter         = true;
}

void Record::SetSnapshotMode() { perfargs_.snapshot = true; }

void Record::TakeSnapshot(const RunningProcess& context) {
	if (!perfargs_.snapshot)
		throw std::runtime_error("Start record with snapshot mode to take snapshot");

	kill(context.Pid, SIGUSR2);
}

arglist Record::build_arglist_() {
	arglist args {perfargs_.prefix};
	args.insert(args.end(), {"-e", perfargs_.ptargs});
	args.insert(args.end(), {"-o", perfargs_.outfile});

	if (perfargs_.filter) {
		std::string filter {};
		if (perfargs_.symbol)
			filter += "filter " + *perfargs_.symbol + " @ " + perfargs_.program + " ";

		if (perfargs_.instrptr_range)
			filter += "filter " + *perfargs_.instrptr_range + " @ " + perfargs_.program;

		args.insert(args.end(), {"--filter", filter});
	}

	if (perfargs_.snapshot) args.insert(args.end(), {"--snapshot"});

	if (perfargs_.pid) {
		args.insert(args.end(), {"-p", std::to_string(*perfargs_.pid)});
	} else {
		args.insert(args.end(), {"--", perfargs_.program});
		args.insert(args.end(), perfargs_.programargs.begin(), perfargs_.programargs.end());
	}

	return args;
}

bool Record::check_cyc_avail() {
	int fd = open("/sys/bus/event_source/devices/intel_pt/caps/psb_cyc", O_RDONLY);
	if (fd == -1) throw std::runtime_error("Error opening file that indiates cyc support");

	char val {};
	if (read(fd, &val, sizeof(char)) == -1)
		throw std::runtime_error("Error checking cyc availability");
	close(fd);

	if (val == '1') return true;

	return false;
}

}  // namespace libitrace
