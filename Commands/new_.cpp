//
// Created by alex on 10/26/26.
//

#include "new_.h"
#include <vector>
#include <string>
#include <ccolor.h>

#include "../Headers/utility.h"

void new_::handle(const std::string &command, const std::vector<std::string> &arguments) {
    size_t arg_size = arguments.size();
    if (arg_size < 2) {
        error("Please provide a file name!");
        return;
    }

    std::string argument = arguments[1];
    size_t first_pos = argument.find(JOURNAL_ENTRY_EXTENSIONS[0]);
    if (first_pos == std::string::npos) {
        first_pos = argument.find(JOURNAL_ENTRY_EXTENSIONS[1]);
    }

    if (first_pos != std::string::npos) {
        argument = argument.substr(0, first_pos);
    }
    std::string filename = LINUX_DOCUMENTS_FOLDER + '/' + argument;

    if (arg_size >= 3) {
        if (arguments[2] != "--obfuscate") {
            error("Unknown argument: " + arguments[2]);
            return;
        }
        filename += JOURNAL_ENTRY_EXTENSIONS[1];
    } else filename += JOURNAL_ENTRY_EXTENSIONS[0];

    if (!std::filesystem::exists(filename)) {
        std::string op;
        std::cout << "Are you sure you wish to create the file: " << filename << "? (y/n): ";
        std::getline(std::cin, op);

        if (op == "y" || op == "Y") {
            std::ofstream f(filename);
            f.close();
            change_stat_value("entries_made", 1, stat_value_change_type::increase_by);
            change_stat_value("write_streak", 1, stat_value_change_type::increase_by);
            std::ofstream file(LINUX_DOCUMENTS_FOLDER + '/' + ".streak");
            if (file.is_open()) {
                file << std::time(nullptr);
                file.close();
            }
        } else setFOutColour("Cancelled. " + filename + " has not been created.", OutputColour::BrightYellow, {}, true, 1);
    }
    else {
        setFOutColour(filename + " already exists! Would you like to clear the file? (y/n): ", OutputColour::BrightYellow);
        std::string op;
        std::getline(std::cin, op);
        if (op == "y" || op == "Y") {
            std::cout << "Clearing " + filename + " will permanently remove all text currently saved. Continue? (y/n): ";
            std::getline(std::cin, op);
            if (op == "y" || op == "Y") {
                std::ofstream f(filename, std::ios::trunc);
                f.close();
                setFOutColour("Cleared " + filename + " of it's contents.", OutputColour::BrightBlue, {}, true, 1);
            }
            else setFOutColour("Cancelled. " + filename + " has not been changed.", OutputColour::BrightYellow, {}, true, 1);
        }
    }
}
