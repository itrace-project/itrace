#include "libitrace/decode.hpp"

#include <fcntl.h>

#include <ctime>

#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"

namespace libitrace {

void Decode::Run() {
	arglist perfargs = build_arglist_();
	print_perf_args(perfargs);
	Subprocess perfscript {"perf", perfargs};

	// set to everyone rw but umask will mask it to something different
	int fd = open(outfile_.c_str(), O_RDWR | O_CREAT, 0660);
	if (fd == -1) die("open");
	perfscript.SetStdout(fd);

	auto res = perfscript.Run();
	if (!res) throw std::runtime_error("Error decoding trace data");
	if (res->Exit != 0) throw std::runtime_error(res->Stderr);
}

void Decode::UseXed() { args_.xed = true; }

void Decode::AddTimeRange(
    std::optional<struct timespec> start, std::optional<struct timespec> end
) {
	args_.start_time = start;
	args_.end_time   = end;
}

void Decode::AddSource() {
    args_.src= true;
}

libitrace::arglist Decode::build_arglist_() {
	arglist args {args_.prefix};
	args.insert(args.end(), args_.synth_events);
	args.insert(args.end(), args_.insn_trace);
	args.insert(args.end(), {"-i", args_.infile});

	if (args_.xed) args.insert(args.end(), "--xed");

	if (args_.start_time || args_.end_time) {
		std::string timerange {};
		if (args_.start_time) timerange += timespec_to_string(*args_.start_time);
		timerange += ",";
		if (args_.end_time) timerange += timespec_to_string(*args_.end_time);
		args.insert(args.end(), {"--time", timerange});
	}

    if (args_.src) {
        args.insert(args.end(), {"-F," "+srccode,+time"});
    }

	return args;
}


}  // namespace libitrace
