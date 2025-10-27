/*
 * decode.hpp
 *
 * A class for abstracting away the decoding of a trace.
 * */
#pragma once

#include <argparse/argparse.hpp>

namespace libitrace {

/*
 * @class Decode
 * @brief A class that abstracts the decoding of a trace into human readable
 * output. Uses subprocesses to spawn perf script instances
 * */
class Decode {
public:
	Decode() = delete;

	/*
	 * @brief Initialite a Decode instance
	 * @param path to trace binary file
     * @param path to trace output file
	 * */
	Decode(const std::string& infile, const std::string& outfile)
	    : infile_ {infile},
	      outfile_ {outfile} {}

	void Run();

private:
    std::string infile_ {};
    std::string outfile_ {};
};

}  // namespace libitrace
