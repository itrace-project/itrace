#include <argparse/argparse.hpp>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <vector>

#include "libitrace/record.hpp"
#include "libitrace/utils.hpp"
#include "record.hpp"

using std::cout, std::cerr;

void record(const argparse::ArgumentParser& args) {
	std::vector<std::string> target {};
	std::string outfile {};
	try {
		target  = args.get<std::vector<std::string>>("target");
		outfile = args.get<std::string>("output");
	} catch (std::logic_error& e) {
		cerr << e.what() << "\n";
		cerr << args;
		exit(1);
	}

    if (!target.empty()) {
        std::string program = target[0];
        std::vector<std::string> programargs(target.begin() + 1, target.end());
        libitrace::Record instance(program, programargs, outfile);
        instance.Run();

    } else if (args.is_used("pid")) {
        libitrace::Record instance(outfile);
        pid_t pid = args.get<int>("pid");
        pid_t perfpid = instance.Attach(pid);
        cout << "Press any key to stop trace\n";
        fd_set readfds;
        FD_SET(STDIN_FILENO, &readfds);
        if (select(STDIN_FILENO+1, &readfds, NULL, NULL, NULL) == -1) die("select");

        kill(perfpid, SIGINT);
        int stat_loc {};
        if (waitpid(perfpid, &stat_loc, 0) == -1) die("waitpid");

    } else {
        cerr << "Sepcify one of pid or target program\n";
        cerr << args;
        exit(1);
    }
}

