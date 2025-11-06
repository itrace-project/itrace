#include "libitrace/record.hpp"

#include <sstream>
#include <sys/wait.h>

#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"

void libitrace::Record::Run() {
	if (perfargs_.program.empty()) {
		std::cerr << "Specify a target program when tracing with Run()\n";
		exit(1);
	}

	auto args = build_arglist_();
    for (auto arg : args)
        std::cout << arg << " ";
    std::cout << std::endl;
	libitrace::Subprocess perfrecord {"perf", args};

	auto res {perfrecord.Run()};
	if (!res || res->Exit != 0) die2(res->Stderr.c_str());

	std::cout << res->Stderr;
}

pid_t libitrace::Record::Attach(pid_t pid) {
    perfargs_.pid = pid;

	auto args = build_arglist_();
	libitrace::Subprocess perfrecord {"perf", args};

	auto res {perfrecord.Popen()};
	if (!res) die2("failed to attach to process");

	return res->Pid;
}

libitrace::arglist libitrace::Record::build_arglist_() {
	libitrace::arglist args {perfargs_.prefix};
	args.insert(args.end(), {"-e", perfargs_.ptargs});
	args.insert(args.end(), {"-o", perfargs_.outfile});

    if (perfargs_.filter) {
        std::string filter {};
        if (perfargs_.symbol)
            filter += "filter " + *perfargs_.symbol + " @ " + perfargs_.program + " ";

        if (perfargs_.instrptr_range)
            filter += "filter " + *perfargs_.instrptr_range + " @ " + perfargs_.program + " ";

        args.insert(args.end(), {"--filter", filter});
    }

	if (perfargs_.pid) {
		args.insert(args.end(), {"-p", std::to_string(*perfargs_.pid)});
	} else {
		args.insert(args.end(), {"--", perfargs_.program});
		args.insert(
		    args.end(), perfargs_.programargs.begin(),
		    perfargs_.programargs.end()
		);
	}

	return args;
}

void libitrace::Record::AddSymbolFilter(std::string symbol) {
    perfargs_.symbol = symbol;
    perfargs_.filter = true;
}

void libitrace::Record::AddInstrPtrFilter(long start, long end) {
	if (end <= start)
		throw std::runtime_error(
		    "Please specify a valid instruction pointer range in hex"
		);

	long size = end - start;
    std::stringstream ss {};
    ss << "0x" << std::hex << start << "/0x" << size;
    perfargs_.instrptr_range = ss.str();
    perfargs_.filter = true;
}
