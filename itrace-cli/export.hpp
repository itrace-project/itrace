#pragma once

#include <argparse/argparse.hpp>

constexpr const char* FILTER_PATH = "/usr/local/lib/libperf2perfetto.so";

void exporter(const argparse::ArgumentParser& args);
