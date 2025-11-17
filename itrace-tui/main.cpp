#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <csignal>
#include <cstdio>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace ftxui;

int main() {
	auto screen = ScreenInteractive::Fullscreen();

	enum class ScreenState { Menu, ArgsInput };
	ScreenState current_screen = ScreenState::Menu;

	// --- Menu ---
	std::vector<std::string> commands = {"Record", "Decode", "Export"};
	int menu_selected                 = 0;

	// --- Argument labels ---
	std::map<std::string, std::vector<std::string>> args_labels = {
	    {"Record",
	     {"Target", "Output", "PID", "Filter Symbol", "Filter Instruction Pointer", "Snapshot"}},
	    {"Decode", {"Input", "Output", "Time Window"}                                          },
	    {"Export", {"Input", "Output"}	                                                     }
	};

	// --- Argument descriptions ---
	std::map<std::string, std::vector<std::string>> arg_descriptions = {
	    {"Record",
	     {"Target program and its arguments [nargs: 0 or more] ",
	      "Output file of trace [nargs=0..1] [default: itrace.data]", "Process id to attach to",
	      "Symbol to filter trace data on",
	      "Instruction pointer addresses to filter trace data on. Formatted as <start>,<end> where "
	      "addresses are in hex",
	      "Record the trace in snapshot mode"}                      },
	    {"Decode",
	     {"Path to .data trace file [nargs=0..1] [default: itrace.data]",
	      "Output file of trace [nargs=0..1] [default: itrace.trace]",
	      "Only decode trace within <start>,<end> time window..."}  },
	    {"Export",
	     {"Path to .data trace file [nargs=0..1] [default: itrace.data]",
	      "Output file of trace [nargs=0..1] [default: itrace.ftf]"}}
	};

	// --- Default argument values ---
	std::map<std::string, std::vector<std::string>> input_values;
	input_values["Record"] = {"", "itrace.data", "", "", "", "No"};
	input_values["Decode"] = {"itrace.data", "itrace.trace", ""};
	input_values["Export"] = {"itrace.data", "itrace.ftf"};

	// --- Optional argument formats ---
	std::map<std::string, std::vector<std::string>> arg_formats;
	arg_formats["Record"] = {"<target>", "<output>", "<pid>", "<symbol>", "<start>,<end>", ""};
	arg_formats["Decode"] = {"<input>", "<output>", "<start>,<end>"};
	arg_formats["Export"] = {"<input>", "<output>"};

	// --- Full command storage ---
	std::map<std::string, std::string> full_command;
	for (auto& cmd : commands) full_command[cmd] = "";

	// --- Snapshot toggle ---
	std::vector<std::string> snapshot_entries = {"No", "Yes"};
	int snapshot_selected                     = input_values["Record"][5] == "Yes" ? 1 : 0;
	Component snapshot_toggle                 = Toggle(&snapshot_entries, &snapshot_selected);

	// --- Components ---
	auto menu           = Menu(&commands, &menu_selected);
	auto menu_container = Container::Vertical({menu});

	std::map<std::string, std::vector<Component>> input_components;
	std::map<std::string, Component> input_containers;

	for (auto& [cmd, labels] : args_labels) {
		std::vector<Component> components;
		for (size_t i = 0; i < labels.size(); ++i) {
			if (labels[i] == "Snapshot") {
				components.push_back(snapshot_toggle);
			} else {
				std::string placeholder = input_values[cmd][i].empty() ? arg_formats[cmd][i] : "";
				components.push_back(Input(
				    &input_values[cmd][i],
				    labels[i] + (placeholder.empty() ? "" : " (" + placeholder + ")")
				));
			}
		}
		input_components[cmd] = components;
		input_containers[cmd] = Container::Vertical(components);
	}

	Component top_container = Container::Vertical({menu_container});
	for (auto& [cmd, container] : input_containers) top_container->Add(container);

	// --- Track focused input index and popups ---
	std::map<std::string, int> focused_input_index;
	std::map<std::string, bool> show_info_popup;
	std::map<std::string, bool> show_output_popup;
	std::map<std::string, bool> show_completed_output_popup;
	std::map<std::string, std::string> command_output;

	for (auto& cmd : commands) {
		focused_input_index[cmd] = 0;
		show_info_popup[cmd]     = false;
		show_output_popup[cmd]   = false;
		command_output[cmd]      = "";
	}

	// --- Running processes for Record ---
	std::map<std::string, pid_t> running_processes;

	auto reset_inputs = [&]() {
		// Reset input values
		input_values["Record"] = {"", "itrace.data", "", "", "", "No"};
		input_values["Decode"] = {"itrace.data", "itrace.trace", ""};
		input_values["Export"] = {"itrace.data", "itrace.ftf"};

		// Reset toggles
		snapshot_selected = 0;

		// Reset focused indices
		for (auto& cmd : commands) { focused_input_index[cmd] = 0; }
	};

	// --- Async command runner ---
	auto run_command_async = [](const std::string& cmd) -> pid_t {
		pid_t pid = fork();
		if (pid == 0) {
			execl("/bin/sh", "sh", "-c", cmd.c_str(), (char*)nullptr);
			_exit(127);
		}
		return pid;
	};

	// --- Blocking command runner ---
	auto run_command = [](const std::string& cmd) {
		std::array<char, 128> buffer;
		std::string result;
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
		if (!pipe) return std::string("Failed to run command\n");
		while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();
		return result;
	};

	// --- CLI flags mapping ---
	std::map<std::string, std::string> cli_flags = {
	    {"Time Window",                "--time"            },
	    {"Input",	                  "--input"           },
	    {"Output",	                 "--output"          },
	    {"PID",	                    "--pid"             },
	    {"Filter Symbol",              "--filter-symbol"   },
	    {"Filter Instruction Pointer", "--filter-instr-ptr"},
	    {"Snapshot",                   "--snapshot"        },
	    {"Target",	                 ""                  }
	};

	// --- Renderer ---
	Component ui = Renderer(top_container, [&] {
		if (current_screen == ScreenState::Menu) {
			return vbox({
			           text("Choose a command") | bold,
			           menu_container->Render() | border,
			           separator(),
			           text("Press ENTER to configure"),
			           text("Press ESCAPE to exit"),
			       }) |
			       border | center;
		} else {
			auto& cmd        = commands[menu_selected];
			auto& labels     = args_labels[cmd];
			int arg_selected = focused_input_index[cmd];

			Elements rows;
			for (size_t i = 0; i < labels.size(); ++i) {
				Elements elems;
				elems.push_back(vcenter(text(labels[i]) | bold));
				elems.push_back(text(" "));
				elems.push_back(input_components[cmd][i]->Render() | border);
				rows.push_back(hbox(elems));
			}

			rows.push_back(separator());

			// Status message for recording

			rows.push_back(text("Press ESCAPE to go back, ENTER to run command, TAB for info"));

			Element layout = vbox(rows) | border | center;

			if (show_info_popup[cmd]) {
				layout = window(
				             text("Info"), vbox(
				                               {paragraph(arg_descriptions[cmd][arg_selected]),
				                                separator(), text("Press TAB to close")}
				                           )
				         ) |
				         center;
			}

			if (show_output_popup[cmd]) {
				layout =
				    window(
				        text("Command Output"),
				        vbox(
				            {text("Command:") | bold, paragraph(full_command[cmd]), separator(),
				             text("Output:") | bold, paragraph(command_output[cmd]), separator(),
				             text("Press ESCAPE to close")}
				        )
				    ) |
				    center;
			}

			if (show_completed_output_popup[cmd]) {
				layout =
				    window(
				        text("Command Output"),
				        vbox(
				            {text("Command:") | bold, paragraph(full_command[cmd]), separator(),
				             text("Output:") | bold, paragraph(command_output[cmd]), separator(),
				             text("Press ESCAPE to close")}
				        )
				    ) |
				    center;
			}

			return layout;
		}
	});

	// --- Event handling ---
	ui = CatchEvent(ui, [&](Event e) {
		if (current_screen == ScreenState::Menu) {
			if (e == Event::ArrowDown) {
				menu_selected = (menu_selected + 1) % commands.size();
				return true;
			}
			if (e == Event::ArrowUp) {
				menu_selected = (menu_selected - 1 + commands.size()) % commands.size();
				return true;
			}
			if (e == Event::Return) {
				current_screen = ScreenState::ArgsInput;
				input_containers[commands[menu_selected]]->TakeFocus();
				return true;
			}
			if (e == Event::Escape) {
				screen.Exit();
				return true;
			}
		} else {
			auto& cmd         = commands[menu_selected];
			int& arg_selected = focused_input_index[cmd];

			if (show_info_popup[cmd] || show_output_popup[cmd] ||
			    show_completed_output_popup[cmd]) {
				if (e == Event::Tab || e == Event::Escape) {
					show_info_popup[cmd]             = false;
					show_output_popup[cmd]           = false;
					show_completed_output_popup[cmd] = false;
					return true;
				}
				if (e == Event::Return) {
					if (cmd == "Record" && running_processes[cmd] > 0) {
						kill(running_processes[cmd], SIGINT);
						waitpid(running_processes[cmd], nullptr, 0);
						running_processes[cmd] = 0;
						command_output[cmd]    = "Recording stopped.";
						return true;
					}
				}
			} else {
				if (e == Event::ArrowDown) {
					arg_selected = (arg_selected + 1) % input_components[cmd].size();
					input_components[cmd][arg_selected]->TakeFocus();
					return true;
				}
				if (e == Event::ArrowUp) {
					arg_selected = (arg_selected - 1 + input_components[cmd].size()) %
					               input_components[cmd].size();
					input_components[cmd][arg_selected]->TakeFocus();
					return true;
				}
				if (e == Event::Tab) {
					show_info_popup[cmd] = true;
					return true;
				}

				if (e == Event::Return) {
					std::string lower_line = cmd;
					std::transform(
					    lower_line.begin(), lower_line.end(), lower_line.begin(),
					    [](unsigned char c) { return std::tolower(c); }
					);

					std::string command_line = "itrace " + lower_line;
					std::string input_file   = "";  // capture input/output file for status

					for (size_t i = 0; i < input_components[cmd].size(); ++i) {
						const std::string& label = args_labels[cmd][i];
						if (label == "Snapshot") {
							if (snapshot_selected == 1) { command_line += " --snapshot"; }
						} else if (!input_values[cmd][i].empty()) {
							command_line += " " + cli_flags[label] + " " + input_values[cmd][i];
							if (label == "Input") input_file = input_values[cmd][i];
						}
					}

					full_command[cmd] = command_line;

					// Show starting message
					if (cmd == "Record") {
						running_processes[cmd] = run_command_async(command_line);
						command_output[cmd]    = "Recording started ...";
					} else if (cmd == "Decode") {
						command_output[cmd] = "Decoding file " + input_file + "...";

						// Run asynchronously in background thread
						std::thread([&, command_line, cmd]() {
							std::string result               = run_command(command_line);
							command_output[cmd]              = "Completed decoding!";
							show_completed_output_popup[cmd] = true;
						}).detach();
					} else if (cmd == "Export") {
						command_output[cmd] = "Exporting file " + input_file + "...";

						std::thread([&, command_line, cmd]() {
							std::string result               = run_command(command_line);
							command_output[cmd]              = "Completed exporting!";
							show_completed_output_popup[cmd] = true;
						}).detach();
					}

					show_output_popup[cmd] = true;  // show popup immediately
					reset_inputs();
					return true;
				}

				if (e == Event::Escape) {
					current_screen = ScreenState::Menu;
					return true;
				}
			}
		}
		return false;
	});

	menu_container->TakeFocus();
	screen.Loop(ui);

	// Kill any running Record process on exit
	if (running_processes["Record"] > 0) {
		kill(running_processes["Record"], SIGINT);
		waitpid(running_processes["Record"], nullptr, 0);
	}

	return 0;
}
