/*
 * record.hpp
 *
 * A class for abstracting away the tracing of a program.
 * */
#pragma once

#include <argparse/argparse.hpp>

#include "libitrace/subprocess.hpp"

namespace libitrace {

struct RecordArgs {
	std::string prefix {"record"};
	std::string ptargs {"intel_pt//u"};
	std::string outfile {};
	std::string program {};
	arglist programargs {};
	std::optional<pid_t> pid {std::nullopt};
    std::optional<std::string> symbol;
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
	    const std::string& targetprogram,
	    const std::vector<std::string>& targetargs, const std::string& outfile
	) {
		perfargs_.outfile     = outfile;
		perfargs_.program     = targetprogram;
		perfargs_.programargs = targetargs;
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
	pid_t Attach(pid_t pid);

	/*
	 * @brief Add a symbol from the program binary to track. Perf will trace
	 * only that symbol
	 * @param string
	 * */
	void AddSymbolFilter(std::string symbol);

private:
	RecordArgs perfargs_ {};

	libitrace::arglist build_arglist_();
};

}  // namespace libitrace
