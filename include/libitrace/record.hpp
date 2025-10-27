/*
 * record.hpp
 *
 * A class for abstracting away the tracing of a program.
 * */
#pragma once

#include <argparse/argparse.hpp>

namespace libitrace {

/*
 * @class Record
 * @brief A class that abstracts the tracing of a program using perf. Uses
 * subprocesses to spawn perf that traces the program
 * */
class Record {
public:
	Record() = delete;

	/*
	 * @brief Initialite a Record instance for attaching to a tracee
     * @arapm path of the output file
	 * */
	Record(const std::string& outfile)
	      : outfile_ {outfile} {}

	/*
	 * @brief Initialite a Record instance for running a tracee
	 * @param target program to trace
     * @param arguments to pass into target program
     * @arapm path of the output file
	 * */
	Record(
	    const std::string& targetprogram,
	    const std::vector<std::string>& targetargs, const std::string& outfile
	)
	    : targetprogram_ {targetprogram},
	      targetargs_ {targetargs},
	      outfile_ {outfile} {}

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

private:
	std::string targetprogram_ {};
	std::vector<std::string> targetargs_ {};
	std::string outfile_ {};
};

}  // namespace libitrace
