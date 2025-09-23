#include "libitrace/subprocess.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include "libitrace/utils.hpp"

// Helper functions
std::string read_pipe(int fd) {
	std::string out {};
	char buf[4096] {};
	int bytes {};
	while ((bytes = read(fd, buf, sizeof(buf)))) {
		if (bytes == 0 || bytes == -1) break;
		buf[bytes] = '\n';
		out += buf;
	}
	return out;
}

namespace libitrace {

std::optional<CompletedProcess> Subprocess::Run() {
	int stdout_pipe[2] {};
	int stderr_pipe[2] {};
	if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
		perror("pipe creation failed");
		return std::nullopt;
	}

	pid_t child_pid = fork();
	if (child_pid == -1) {
		perror("error creating child process");
		return std::nullopt;
	}

	if (child_pid == 0) {
		// Setup arguments to exec
		char** argv {new char*[_args.size() + 2]};
		argv[0] = _cmd.data();
		for (size_t i {0}; i < _args.size(); ++i) argv[i + 1] = _args[i].data();
		argv[_args.size() + 1] = nullptr;

		if (stdoutfd_ == -1)
			dup2(stdout_pipe[1], STDOUT_FILENO);
		else
			dup2(stdoutfd_, STDOUT_FILENO);
		close(stdout_pipe[0]);
		dup2(stderr_pipe[1], STDERR_FILENO);
		close(stderr_pipe[0]);

		execvp(argv[0], argv);
		// argv not memory leak because execvp suceeding will mean all memory is
		// reclaimed. if argv fails need to dealloate

		delete[] argv;
		perror("execvp failed");
		exit(1);
	}

	// Close the write ends of the open fds to the pipes to send eof
	// pipe read before wait because if pipe becomes full causes deadlock
	close(stdout_pipe[1]);
	close(stderr_pipe[1]);
	std::string stdout {read_pipe(stdout_pipe[0])};
	std::string stderr {read_pipe(stderr_pipe[0])};
	close(stdout_pipe[0]);
	close(stderr_pipe[0]);

	int stat_loc {};
	if (waitpid(child_pid, &stat_loc, 0) != child_pid) {
		perror("unexpected pid from wait returned");
		return std::nullopt;
	}

	// TODO: Per wait man page, differentiate different exit conditions
	return CompletedProcess {
	    _cmd, _args, stdout, stderr, WEXITSTATUS(stat_loc)
	};
}

int Subprocess::SetStdout(int fd) {
	stdoutfd_ = fd;
	return 0;
}

}  // namespace libitrace
