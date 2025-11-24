/*
 * record.hpp
 *
 * A class for abstracting away the tracing of a program.
 * */
#pragma once

#include <argparse/argparse.hpp>

#include "libitrace/subprocess.hpp"

namespace libitrace {

/*
 * @struct RecordArgs
 * @brief Arugments into the perf record subprocess that is spawned to trace
 * */
struct RecordArgs {
	std::string prefix {"record"};
	std::string ptargs {
	    "intel_pt/cyc=1,noretcomp/u"
	};  // TODO: only user mode now but add option to change it, maximum cyc granularity by default
	std::string outfile {};
	std::string program {};
	arglist programargs {};
	std::optional<pid_t> pid {std::nullopt};
	std::optional<std::string> symbol;
	std::optional<std::string> instrptr_range {};
	bool snapshot {false};
	bool filter {false};
};

/*
 * @class Record
 * @brief A class that abstracts the tracing of a program using perf. Uses
 * subprocesses to spawn perf that traces the program
 * */
class Record {
public:
	Record() = delete;

	/*
	 * @brief Initialite a Record instance for running a tracee
	 * @param target program to trace
	 * @param arguments to pass into target program
	 * @param path of the output file
	 * */
	Record(
	    const std::string& targetprogram, const std::vector<std::string>& targetargs,
	    const std::string& outfile
	) {
		perfargs_.outfile     = outfile;
		perfargs_.program     = targetprogram;
		perfargs_.programargs = targetargs;

		if (check_cyc_avail()) {
			perfargs_.ptargs = "intel_pt/cyc=1,noretcomp=1/u";
		} else {
			perfargs_.ptargs = "intel_pt/noretcomp=1/u";
		}
	}

	/*
	 * @brief Run the traget program under a perf record.
	 * */
	void Run();

	/*
	 * @brief Attach the tracer to the target pid
	 * @param Target pid
	 * @return Pid of tracer process
	 * */
	RunningProcess Attach(pid_t pid);

	/*
	 * @brief Add a symbol from the program binary to track. Perf will trace
	 * only that symbol
	 * @param string
	 * */
	void AddSymbolFilter(std::string symbol);

	/*
	 * @brief Add an instruction pointer range from the program binary to track.
	 * Perf will trace only within the range
	 * @param start address
	 * @param end address
	 * */
	void AddInstrPtrFilter(long start, long end);

	/*
	 * @brief Record a trace using snapshotting instead of an exhaustive trace
	 * */
	void SetSnapshotMode();

	/*
	 * @brief Snapshot a trace for an instance started with snapshot mode
	 * @param RunningProcess context returned by Attach
	 * */
	void TakeSnapshot(const RunningProcess& context);

private:
	RecordArgs perfargs_ {};

	libitrace::arglist build_arglist_();
	bool check_cyc_avail();
};

}  // namespace libitrace
