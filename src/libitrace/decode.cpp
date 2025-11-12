#include "libitrace/decode.hpp"

#include <fcntl.h>

#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"

namespace libitrace {

void Decode::Run() {
	arglist perfargs = {
	    "script", "-i", infile_, "--insn-trace", "--xed"
	};
	print_perf_args(perfargs);
	Subprocess perfscript {"perf", perfargs};

	// set to everyone rw but umask will mask it to something different
	int fd = open(outfile_.c_str(), O_RDWR | O_CREAT, 0660);
	if (fd == -1) die("open");
	perfscript.SetStdout(fd);

	auto res = perfscript.Run();
	if (!res) throw std::runtime_error("Error decoding trace data");
	if (res->Exit != 0) throw std::runtime_error(res->Stderr);
}

}  // namespace libitrace
