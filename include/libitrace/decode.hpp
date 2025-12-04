/*
 * decode.hpp
 *
 * A class for abstracting away the decoding of a trace.
 * */
#pragma once

#include <argparse/argparse.hpp>
#include <optional>
#include <string>

#include "libitrace/subprocess.hpp"

namespace libitrace {

/*
 * @struct ScriptArgs
 * @brief Arugments into the perf script subprocess that is spawned to decode
 * trace
 * */
struct ScriptArgs {
	std::string prefix {"script"};                 // perf script
	std::string synth_events {"--itrace=ibxwpe"};  // synthesize all events
	std::string insn_trace {"--insn-trace"};       // instruction trace by default
	std::string infile {};
	std::optional<struct timespec> start_time {std::nullopt};
	std::optional<struct timespec> end_time {std::nullopt};
    bool src {};
	bool xed {};
};

/*
 * @class Decode
 * @brief A class that abstracts the decoding of a trace into human readable.
 * The perf command used to run this is `perf script --insn-trace --xed -i
 * <input_file>` output. Uses subprocesses to spawn perf script instance.
 * */
class Decode {
public:
	Decode() = delete;

	/*
	 * @brief Initialite a Decode instance
	 * @param path to trace binary file
	 * @param path to trace output file
	 * */
	Decode(const std::string& infile, const std::string& outfile) : outfile_ {outfile} {
		args_.infile = infile;
	}

	void Run();

	/*
	 * @brief Use xed to decode x86 instructions
	 * */
	void UseXed();

	/*
	 * @brief Add a start and/or end time to decode trace from.
	 * @param A timespec struct that contains a seconds and nanoseconds field
	 * */
	void AddTimeRange(
	    std::optional<struct timespec> start = std::nullopt,
	    std::optional<struct timespec> end   = std::nullopt
	);

	/*
	 * @brief Add the source code and source line interleaved in the trace. Only works if compiled
	 * with the debug flag.
	 * */
	void AddSource();

private:
	ScriptArgs args_ {};
	std::string outfile_ {};

	libitrace::arglist build_arglist_();
};

}  // namespace libitrace
