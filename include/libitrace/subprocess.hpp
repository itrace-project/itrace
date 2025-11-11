/*
 * subprocess.hpp
 *
 * A class for spawning subprocesses. Subprocess is heavily inspired by the
 * python subrpocess module.
 * */

#pragma once

#include <unistd.h>

#include <optional>
#include <string>
#include <vector>

namespace libitrace {

using cmd     = std::string;
using arglist = std::vector<std::string>;

/*
 * @struct RunningProcess
 * @brief a struct that contains the metadata of a running process such as pid,
 * stdout pipe, and stderr pipe
 * */
struct RunningProcess {
	cmd Cmd {};
	arglist Arglist {};
	pid_t Pid;
	int Stdout_pipe {};
	int Stderr_pipe {};
};

/*
 * @struct CompletedProcess
 * @brief a struct that contains the outputs of a subprocess such as the stdout,
 * stderr, and exit codes
 * */
struct CompletedProcess {
	cmd Cmd {};
	arglist Arglist {};
	std::string Stdout {};
	std::string Stderr {};
	int Exit {};
};

/*
 * @class Subprocess
 * @brief a class that wraps around the execution of a sobprocess
 * */
class Subprocess {
public:
	Subprocess() = delete;

	/*
	 * @brief initialize with a program identified by a path. Searches $PATH if
	 * no path is given
	 * @param cmd program to run as a string
	 * @param arglist arguments to the program as a vector of strings
	 * */
	Subprocess(cmd cmd, arglist args = arglist {})
	    : cmd_ {cmd},
	      args_ {args},
	      stdoutfd_ {-1},
	      capturestdout_ {true} {}

	/*
	 * @brief Run the specified program with the specified argument list and
	 * block until it terminates
	 * @return An optional CompletedProcess object with stdout, stderr, and exit
	 * status
	 * */
	std::optional<CompletedProcess> Run();

	/*
	 * @brief run the specified program with the specified argument list
	 * asynchronously
	 * @return An optional RunningProcess object
	 * */
	std::optional<RunningProcess> Popen();

	/*
	 * @brief Block until a process spawned by Popen terminates
     * @param RunningProcess context returned by Popen
     * @param Whether or not to capture stdout and stderr in CompletedProcess object
	 * @return An optional CompletedProcess object with stdout, stderr, and exit
	 * status
	 * */
	static std::optional<CompletedProcess> Wait(const RunningProcess& context, bool capturestdout=false);

	/*
	 * @brief set the stdout to a different file descriptor
	 * @param file descriptor to redirect stdout to
	 * @return -1 on failure, 0 on success
	 * */
	int SetStdout(int fd);

private:
	cmd cmd_ {};
	arglist args_ {};
	int stdoutfd_ {};
	bool capturestdout_ {};
};

}  // namespace libitrace
