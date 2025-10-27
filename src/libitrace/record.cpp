#include <sys/wait.h>

#include "libitrace/record.hpp"
#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"

void libitrace::Record::Run() {
    if (targetprogram_.empty()) {
        std::cerr << "Specify a target program when tracing with Run()\n";
        exit(1);
    }

	libitrace::arglist perfargs = {
	    "record", "-e", "intel_pt//u", "-o", outfile_, "--", targetprogram_
	};
	perfargs.insert(perfargs.end(), targetargs_.begin(), targetargs_.end());

	libitrace::Subprocess perfrecord {"perf", perfargs};
	auto res {perfrecord.Run()};
	if (!res || res->Exit != 0) die2(res->Stderr.c_str());
    std::cout << res->Stderr;
}

pid_t libitrace::Record::Attach(pid_t pid) {
	libitrace::arglist perfargs = {
	    "record", "-e", "intel_pt//u", "-o", outfile_, "-p", std::to_string(pid)
	};

	libitrace::Subprocess perfrecord {"perf", perfargs};
	auto res {perfrecord.Popen()};
    if (!res) die2("failed to attach to process");
    return res->Pid;
}
