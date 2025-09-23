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
	    : _cmd {cmd},
	      _args {args},
	      stdoutfd_ {-1} {}

	/*
	 * @brief run the specified program with the specified argument list
	 * @return An optional CompletedProcess object
	 * */
	std::optional<CompletedProcess> Run();

	/*
	 * @brief set the stdout to a different file descriptor
	 * @param file descriptor to redirect stdout to
	 * @return -1 on failure, 0 on success
	 * */
	int SetStdout(int fd);

private:
	cmd _cmd {};
	arglist _args {};
	int stdoutfd_ {};
};

}  // namespace libitrace
