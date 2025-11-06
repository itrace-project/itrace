#include "libitrace/decode.hpp"

#include <fcntl.h>

#include "libitrace/subprocess.hpp"
#include "libitrace/utils.hpp"

void libitrace::Decode::Run() {
	libitrace::arglist perfargs = {
	    "script", "-i", infile_, "--insn-trace", "--xed"
	};
	libitrace::Subprocess perfscript {"perf", perfargs};

	// set to everyone rw but umask will mask it to something different
	int fd = open(outfile_.c_str(), O_RDWR | O_CREAT, 0660);
	if (fd == -1) die("open");
	perfscript.SetStdout(fd);

	auto res = perfscript.Run();
	if (!res || res->Exit != 0) die2(res->Stderr.c_str());
}
