/*
 * subprocess.hpp
 *
 * A class for spawning subprocesses. Subprocess is heavily inspired by the
 * python subrpocess module.
 * */

#pragma once

#include <string>

namespace libitrace {

using cmd     = std::string;
using arglist = std::vector<std::string>;

struct CompletedProcess {
	cmd Cmd {};
	arglist Arglist {};
	std::string Stdout {};
	std::string Stderr {};
	int Exit {};
};

class Subprocess {
public:
	Subprocess() = delete;

	Subprocess(cmd cmd, arglist args = arglist {}) : _cmd {cmd}, _args {args} {}

	std::optional<CompletedProcess> Run();

private:
	cmd _cmd {};
	arglist _args {};
};

}  // namespace libitrace
