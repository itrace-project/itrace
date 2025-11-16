#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>
#include <map>

using namespace ftxui;

int main() {
    auto screen = ScreenInteractive::Fullscreen();

    enum class ScreenState { Menu, ArgsInput };
    ScreenState current_screen = ScreenState::Menu;

    // --- Menu ---
    std::vector<std::string> commands = {"Record", "Decode", "Export"};
    int menu_selected = 0;

    // --- Argument labels ---
    std::map<std::string, std::vector<std::string>> args_labels = {
        {"Record", {"Output", "PID", "Filter Symbol", "Filter Instruction Pointer", "Snapshot"}},
        {"Decode", {"Input", "Output", "Time Window"}},
        {"Export", {"Input", "Output"}}
    };

    // --- Argument descriptions ---
    std::map<std::string, std::vector<std::string>> arg_descriptions = {
        {"Record", {
            "Output file of trace [nargs=0..1] [default: itrace.data]",
            "Process id to attach to",
            "Symbol to filter trace data on",
            "Instruction pointer addresses to filter trace data on. Formatted as <start>,<end> where addresses are in hex",
            "Record the trace in snapshot mode"
        }},
        {"Decode", {
            "Path to .data trace file [nargs=0..1] [default: itrace.data]",
            "Output file of trace [nargs=0..1] [default: itrace.trace]",
            "Only decode trace within <start>,<end> time window. Only <start> with decode until end, only <end> will decode from the start to the end. Format of the timestamps are in <seconds>.<nanoseconds> or the same way it is displayed in the decoded .trace file"
        }},
        {"Export", {
            "Path to .data trace file [nargs=0..1] [default: itrace.data]",
            "Output file of trace [nargs=0..1] [default: itrace.ftf]"
        }}
    };

    // --- Default argument values ---
    std::map<std::string, std::vector<std::string>> input_values;
    input_values["Record"] = {"itrace.data", "", "", "", "No"};
    input_values["Decode"] = {"itrace.data", "itrace.trace", ""};
    input_values["Export"] = {"itrace.data", "itrace.ftf"};

    // --- Optional argument formats ---
    std::map<std::string, std::vector<std::string>> arg_formats;
    arg_formats["Record"] = {"", "<pid>", "<symbol>", "<start>,<end>", ""};
    arg_formats["Decode"] = {"", "", "<start>,<end>"};
    arg_formats["Export"] = {"", ""};

    // --- Components ---
    std::vector<std::string> snapshot_entries = {"No", "Yes"};
    int snapshot_selected = input_values["Record"][4] == "Yes" ? 1 : 0; // map current value to toggle
    Component snapshot_toggle = Toggle(&snapshot_entries, &snapshot_selected);
    
    auto menu = Menu(&commands, &menu_selected);
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
            components.push_back(Input(&input_values[cmd][i], labels[i] + (placeholder.empty() ? "" : " (" + placeholder + ")")));
          }
        }
        input_components[cmd] = components;
        input_containers[cmd] = Container::Vertical(components);
    }

    Component top_container = Container::Vertical({menu_container});
    for (auto& [cmd, container] : input_containers)
        top_container->Add(container);

    // --- Track focused input index per command ---
    std::map<std::string, int> focused_input_index;
    std::map<std::string, bool> show_popup;
    for (auto& cmd : commands) {
        focused_input_index[cmd] = 0;
        show_popup[cmd] = false;
    }

    // --- Renderer ---
    Component ui = Renderer(top_container, [&] {
        if (current_screen == ScreenState::Menu) {
            return vbox({
                       text("Choose a command") | bold,
                       menu_container->Render() | border,
                       separator(),
                       text("Press ENTER to configure"),
                       text("Press q to exit"),
                   }) | border | center;
        } else {
            auto& cmd = commands[menu_selected];
            auto& labels = args_labels[cmd];
            int arg_selected = focused_input_index[cmd];

            Elements rows;
            for (size_t i = 0; i < labels.size(); ++i) {
                Elements elems;
                elems.push_back(vcenter(text(labels[i]) | bold));
                elems.push_back(text(" "));
                auto elem = input_components[cmd][i]->Render() | border;
                elems.push_back(elem);

                rows.push_back(hbox(elems));
            }

            rows.push_back(separator());
            rows.push_back(text("Press q to go back, ENTER to confirm, Press x for more information"));

            Element layout = vbox(rows) | border | center;

            if (show_popup[cmd]) {
                layout = window(text("Info"), vbox({
                    paragraph(arg_descriptions[cmd][arg_selected]),
                    separator(),
                    text("Press x to close")
                })) | center;
            }

            return layout;
        }
    });

    // --- Event handling ---
    ui = CatchEvent(ui, [&](Event e) {
        if (current_screen == ScreenState::Menu) {
            if (e == Event::ArrowDown) {
                menu_selected = (menu_selected + 1) % commands.size();
                input_containers[commands[menu_selected]]->TakeFocus();
                return true;
            }
            if (e == Event::ArrowUp) {
                menu_selected = (menu_selected - 1 + commands.size()) % commands.size();
                input_containers[commands[menu_selected]]->TakeFocus();
                return true;
            }
            if (e == Event::Return) {
                current_screen = ScreenState::ArgsInput;
                input_containers[commands[menu_selected]]->TakeFocus();
                return true;
            }
            if (e == Event::Character('q')) {
                screen.Exit();
                return true;
            }
        } else { // ArgsInput
            auto& cmd = commands[menu_selected];
            int& arg_selected = focused_input_index[cmd];

            if (show_popup[cmd]) {
                if (e == Event::Character('x')) {
                    show_popup[cmd] = false;
                    return true;
                }
            } else {
                if (e == Event::ArrowDown) {
                    arg_selected = (arg_selected + 1) % input_components[cmd].size();
                    input_components[cmd][arg_selected]->TakeFocus();
                    return true;
                }
                if (e == Event::ArrowUp) {
                    arg_selected = (arg_selected - 1 + input_components[cmd].size()) % input_components[cmd].size();
                    input_components[cmd][arg_selected]->TakeFocus();
                    return true;
                }
                if (e == Event::Character('x')) {
                    show_popup[cmd] = true; // open popup
                    return true;
                }
            }

            if (e == Event::Character('q')) {
                current_screen = ScreenState::Menu;
                menu_container->TakeFocus();
                return true;
            }
        }
        return false;
    });

    // --- Start UI ---
    menu_container->TakeFocus();
    screen.Loop(ui);

    return 0;
}
