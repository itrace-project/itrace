#include "libitrace/utils.hpp"

#include <iostream>
#include <sstream>
#include <string>

using std::cout, std::endl;

namespace libitrace {

void print_perf_args(const arglist& perfargs) {
	cout << "[" << " perf " << format_args(perfargs) << " ]" << endl;
}

std::string format_args(const libitrace::arglist& args) {
	std::stringstream stream {};
	for (size_t i = 0; i < args.size() - 1; ++i) { stream << args[i] << " "; }
	stream << args.back();
	return stream.str();
}

std::string timespec_to_string(const timespec& ts) {
	char buf[64];
	std::snprintf(buf, sizeof(buf), "%ld.%09ld", ts.tv_sec, ts.tv_nsec);
	return std::string(buf);
}

}  // namespace libitrace
