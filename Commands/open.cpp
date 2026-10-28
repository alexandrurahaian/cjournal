//
// Created by alex on 10/26/26.
//

#include "open.h"

#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <ccolor.h>

#include "../Headers/utility.h"

void open::handle(const std::string &command, const std::vector<std::string> &arguments) {
    size_t arg_size = arguments.size();
    if (arg_size < 2) {
        error("Please provide the name of the file you would like to open.");
        return;
    }

    std::filesystem::path file_path = std::filesystem::path(LINUX_DOCUMENTS_FOLDER) / arguments[1];
    if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
        error("Invalid file: " + file_path.generic_string());
        return;
    }

    std::ifstream file;
    bool is_obfuscated_file = false;
    if (file_path.extension() == JOURNAL_ENTRY_EXTENSIONS[1]) {
        file = std::ifstream(file_path.generic_string(), std::ios::binary | std::ios::ate);
        is_obfuscated_file = true;
    }
    else file = std::ifstream(file_path.generic_string());
    if (!file.is_open()) {
        error("Failed to open: " + file_path.generic_string());
        return;
    }

    std::string line;
    int line_count = 0;

    if (is_obfuscated_file) {
        std::string extracted;
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer(size, '\0');
        if (file.read(&buffer[0], size)) {
            extracted = mask_xor(buffer);
        }
        file.close();

        for (const auto& c : extracted) {
            if (c != '\n' && c != '\r')
                line += c;
            else {
                if (c == '\r') continue;
                std::cout << line << '\n';
                line.clear();
                line_count++;
                if (line_count >= FETCHED_CONFIG.max_lines_per_page) {
                    setOutColour((std::to_string(line_count) + " lines displayed."), OutputColour::Black, OutputColour::BrightWhite, {}, {}, true);
                    std::cout << " press enter to continue displaying, or q to stop ...\n";
                    std::string op;
                    std::getline(std::cin, op);
                    if (op == "q" || op == "Q") return;

                    line_count = 0;
                }
            }
        }
        if (!line.empty())
            std::cout << line << '\n';
        setFOutColour("[ Reached EOF ]", OutputColour::BrightGreen, {}, true, 1);
        return;
    }

    while (std::getline(file, line)) {
        std::cout << line << '\n';
        line_count++;
        if (line_count >= FETCHED_CONFIG.max_lines_per_page) {
            setOutColour((std::to_string(line_count) + " lines displayed."), OutputColour::Black, OutputColour::BrightWhite, {}, {}, true);
            std::cout << " press enter to continue displaying, or q to stop ...\n";
            std::string op;
            std::getline(std::cin, op);
            if (op == "q" || op == "Q") return;

            line_count = 0;
        }
    }
    file.close();
    setFOutColour("[ Reached EOF ]", OutputColour::BrightGreen, {}, true, 1);
}
