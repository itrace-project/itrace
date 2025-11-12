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
	for (size_t i = 0; i < args.size() - 1; ++i) {
		stream << args[i] << " ";
	}
	stream << args.back();
	return stream.str();
}

}  // namespace libitrace
