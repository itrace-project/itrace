#include "libitrace/subprocess.hpp"

#include <sys/wait.h>
#include <unistd.h>

// Helper functions
std::string read_pipe(int fd) {
	std::string out {};
	char buf[4096] {};
	ssize_t bytes {};
	while ((bytes = read(fd, buf, sizeof(buf)))) {
		if (bytes == 0 || bytes == -1) break;
		out.append(buf, bytes);
	}
	return out;
}

namespace libitrace {

std::optional<RunningProcess> Subprocess::Popen() {
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
		char** argv {new char*[args_.size() + 2]};
		argv[0] = cmd_.data();
		for (size_t i {0}; i < args_.size(); ++i) argv[i + 1] = args_[i].data();
		argv[args_.size() + 1] = nullptr;

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

	close(stdout_pipe[1]);
	close(stderr_pipe[1]);
	return RunningProcess {
	    cmd_, args_, child_pid, stdout_pipe[0], stderr_pipe[0]
    };
}

std::optional<CompletedProcess> Subprocess::Run() {
    auto metadata = Popen();

	// Close the write ends of the open fds to the pipes to send eof
	// pipe read before wait because if pipe becomes full causes deadlock
	std::string stdout {read_pipe(metadata->Stdout_pipe)};
	std::string stderr {read_pipe(metadata->Stderr_pipe)};
	close(metadata->Stdout_pipe);
	close(metadata->Stderr_pipe);

	int stat_loc {};
	if (waitpid(metadata->Pid, &stat_loc, 0) != metadata->Pid) {
		perror("unexpected pid from wait returned");
		return std::nullopt;
	}

	// TODO: Per wait man page, differentiate different exit conditions
	return CompletedProcess {
	    cmd_, args_, stdout, stderr, WEXITSTATUS(stat_loc)
	};
}

int Subprocess::SetStdout(int fd) {
	stdoutfd_ = fd;
	return 0;
}

}  // namespace libitrace
