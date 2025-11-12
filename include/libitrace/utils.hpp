#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <string>

#include "libitrace/subprocess.hpp"

namespace libitrace {

#define die(msg)            \
	do {                    \
		perror(msg);        \
		exit(EXIT_FAILURE); \
	} while (0)

#define die2(msg)           \
	do {                    \
		printf("%s", msg);  \
		exit(EXIT_FAILURE); \
	} while (0)

void print_perf_args(const libitrace::arglist& perfargs);
std::string format_args(const libitrace::arglist& args);

}  // namespace libitrace
