#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include "ccolor/include/ccolor.h"
#include "Headers/command_handler.h"
#include "Headers/utility.h"
#include "Classes/command.h"
#include "Commands/list_entries.h"

void out_watermark() {
    setOutColour("", OutputColour::Blue);
    std::cout << R"(
              ##                                                       mmmm
              ""                                                       ""##
  m#####m   ####      m####m   ##    ##   ##m####  ##m####m   m#####m    ##
 ##"    "     ##     ##"  "##  ##    ##   ##"      ##"   ##   " mmm##    ##
 ##           ##     ##    ##  ##    ##   ##       ##    ##  m##"""##    ##
 "##mmmm#     ##     "##mm##"  ##mmm###   ##       ##    ##  ##mmm###    ##mmm
   """""      ##       """"     """" ""   ""       ""    ""   """" ""     """"
           ####"
                                                                                )";
    std::cout << '\n';
    std::cout << (LINUX_DOCUMENTS_FOLDER.find("Documents") != std::string::npos ? "Detected documents folder: " : "Save folder: ") << LINUX_DOCUMENTS_FOLDER << "\n\n\n";
    resetColour();
    std::cout << "Run 'help' for a list of commands.\n";
}

int main() {
    get_linux_doc_folder();
    if (LINUX_DOCUMENTS_FOLDER == "none") {
        std::string error_text = "Could not fetch the documents folder! Would you like to save entries to the application's working directory? (" + std::filesystem::current_path().generic_string() + ") (y/n):";
        setFOutColour(error_text, OutputColour::BrightRed, {}, true, 0);
        std::string op;
        std::getline(std::cin, op);
        if (op == "Y" || op == "y") {
            LINUX_DOCUMENTS_FOLDER = std::filesystem::current_path();
        }
        else return -1;
    }
    get_config();
    initialise_commands();
    out_watermark();
    can_increment_streak(); // resets the streak if 48 hrs has passed since the last diary edit/creation.
    change_stat_value("avg_word_count", get_average_word_count(), set);

    std::string command_text;
    do {
        std::cout << ">>: ";
        std::getline(std::cin, command_text);
        if (command_text.empty()) continue;
        if (command_text == "exit") {
            std::string op;
            setFOutColour("Are you sure you wish to exit cjournal? (y/n): ", OutputColour::BrightYellow, {}, true);
            std::getline(std::cin, op);
            if (op == "Y" || op == "y") return 0;
            continue;
        }
        if (command_text == "help") {
            output_help();
            continue;
        }

        const std::vector<std::string>& cmd_args = get_command_arguments(command_text);
        if (is_valid_command(cmd_args[0])) handle(cmd_args[0], cmd_args);
        else if (command_text != "exit" && command_text != "e") warn("Unknown command: " + cmd_args[0] + ". Run 'help' for a list of valid commands.");
    }
    while (command_text != "e");

    return 0;
}
