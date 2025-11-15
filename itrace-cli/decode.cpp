#include "libitrace/decode.hpp"

#include "decode.hpp"

using std::cerr, std::endl;

struct timespec parse_time(std::string time) {
    size_t split = time.find(".");
    if (split == std::string::npos) {
        cerr << "Time value " << time << " is invalid" << endl;
        cerr << "Time value must be formatted as <seconds>.<nanoseconds>" << endl;
        exit(1);
    }

    std::string sec_str = time.substr(0, split);
    std::string nsec_str = time.substr(split+1);

    if (sec_str.empty() || nsec_str.empty()) {
        cerr << "Time value " << time << " is invalid" << endl;
        cerr << "Time value must be formatted as <seconds>.<nanoseconds>" << endl;
        exit(1);
    }

    char* end {nullptr};
    long sec = std::strtol(sec_str.c_str(), &end, 10);
    if (*end != '\0') {
        cerr << "Seconds value " << sec_str << " invalid";
        exit(1);
    }

    long nsec = std::strtol(nsec_str.c_str(), &end, 10);
    if (*end != '\0') {
        cerr << "Nanoseconds value " << sec_str << " invalid";
        exit(1);
    }

    struct timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = nsec;
    
    return ts;
}

std::pair<std::optional<struct timespec>, std::optional<struct timespec>> parse_time_input(std::string time) {
    size_t split = time.find(",");
    if (split == std::string ::npos) {
        cerr << "Time range " << time << " is invalid" << endl;
		cerr << "Time range must be formatted as <start>,<end> where <start> or <end> is optional"
		     << endl;
        exit(1);
	}

    std::string start_str {time.substr(0, split)};
    std::string end_str {time.substr(split+1)};
    if (start_str.empty() && end_str.empty()) {
        cerr << "Time range " << time << " is invalid" << endl;
        cerr << "Time range must not be just ','" << endl;
        exit(1);
    }

    std::optional<struct timespec> start {std::nullopt};
    std::optional<struct timespec> end {std::nullopt};
    if (!start_str.empty())
        start = parse_time(start_str);
    if (!end_str.empty())
        end = parse_time(end_str);

    return std::make_pair(start, end);
}

void decode(const argparse::ArgumentParser& args) {
	std::string infile {};
	std::string outfile {};

	try {
		infile  = args.get<std::string>("input");
		outfile = args.get<std::string>("output");
	} catch (std::logic_error& e) {
		cerr << e.what() << "\n";
		cerr << args;
		exit(1);
	}

	libitrace::Decode instance(infile, outfile);
	instance.UseXed();

    if (args.is_used("time")) {
        auto [start, end]  = parse_time_input(args.get<std::string>("time"));
        instance.AddTimeRange(start, end);
    }

	instance.Run();
}
