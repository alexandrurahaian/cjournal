//
// Created by alex on 10/25/26.
//

#include "list_entries.h"
#include "../Headers/utility.h"
#include <filesystem>
#include <ccolor.h>
#include <cfenv>
#include <complex>
#include <vector>
#include <iostream>
#include <optional>

static int fetch_int_from_arg(const std::string& arg) {
    try {
        int x = std::stoi(arg);
        if (x <= 0) throw std::exception();
        return x;
    }
    catch (...) {
        error("Invalid value! Value must be >= 1!");
        return 0;
    }
}

void list_entries::handle(const std::string &command, const std::vector<std::string> &arguments) {
    fetch_option option;
    option.option = none;
    size_t arg_size = arguments.size();
    if (arg_size >= 2) {
        const std::string& op = arguments[1];
        if (op == "--limit") option.option = limit;
        else if (op == "--newest-first") option.option = journal_fetch_option::newest_first;
        else if (op == "--oldest-first") option.option = journal_fetch_option::oldest_first;
        else {
            error("Unknown argument: " + arguments[1] + "!");
            return;
        }
        if (arg_size >= 3) {
            if (fetch_int_from_arg(arguments[2]) != 0) option.value = arguments[2];
            else if (op == "--limit") return;
        }
    }

    setFOutColour("--- JOURNAL ENTRIES ---", OutputColour::BrightGreen, {}, false, 1);
    setFOutColour("Directory: " + LINUX_DOCUMENTS_FOLDER, OutputColour::BrightGreen, {}, true, 2);
    setFOutColour("", OutputColour::Blue, {});
    const std::vector<file_entry_strct>& fetched = get_journal_entries(LINUX_DOCUMENTS_FOLDER, option);
    size_t max_path_len = 0;

    for (const auto& p : fetched) {
        max_path_len = std::max(max_path_len, p.path.string().length());
    }
    for (const auto& p : fetched) {
        std::string path_str = p.path.string();
        std::string label = (p.path.extension() == JOURNAL_ENTRY_EXTENSIONS[1])
                            ? " | CJOURNAL FILE (OBFUSCATED)"
                            : " | CJOURNAL FILE";

        std::cout << std::left << std::setw(static_cast<int>(max_path_len) + 2) << path_str << label << '\n';
    }
    resetColour();
    setFOutColour((!fetched.empty() ? "Total journal entries: " + std::to_string(fetched.size()) : "No journal entries found."), OutputColour::BrightBlue, {}, true, 1);
}
