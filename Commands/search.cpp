//
// Created by alex on 10/27/26.
//

#include "search.h"
#include <vector>
#include <unordered_map>
#include <ccolor.h>
#include <filesystem>
#include <string>

#include "../Headers/utility.h"

void search::handle(const std::string &command, const std::vector<std::string> &arguments) {
    size_t arg_size = arguments.size();
    if (arg_size < 2) {
        error("Please provide a valid search term!");
        return;
    }
    const std::string& search_term = std::move(arguments[1]);
    std::string target_file;
    bool search_one = false;

    if (arg_size >= 4 && arguments[2] == "-T") {
        target_file = arguments[3];
        search_one = true;
    }
    else if (arg_size >= 5 && arguments[2] != "-T") {
        error("Unknown argument: " + arguments[2]);
        return;
    }

    if (search_one) target_file = LINUX_DOCUMENTS_FOLDER + '/' + target_file;
    if (search_one && !std::filesystem::is_regular_file(target_file)) {
        error(target_file + " does not exist or is not a file!");
        return;
    }

    const auto search_file = [search_term](const std::string& file_path) {
        std::string line;
        int count = 0;

        std::ifstream file(file_path);
        if (!file.is_open()) warn("Could not open file for searching: " + file_path);

        while (std::getline(file, line)) {
            if (line.find(search_term) != std::string::npos) count++;
        }
        file.close();

        return count;
    };

    if (search_one) {
        std::ifstream file(target_file);
        std::string line;
        int count = search_file(target_file);
        if (count != 0)
            setFOutColour("Found " + search_term + " " + std::to_string(count) + (count > 1 ? " times " : " time ") + " in " + target_file, OutputColour::BrightBlue, {}, true, 1);
        else setFOutColour("Didn't find " + search_term + " in " + target_file, OutputColour::Blue, {}, true, 1);
    }
    else {
        std::unordered_map<std::string, int> found;
        int searched = 0;

        for (const auto& entry : std::filesystem::directory_iterator(LINUX_DOCUMENTS_FOLDER, std::filesystem::directory_options::skip_permission_denied)) {
            if (!entry.is_regular_file()) continue;
            int count = search_file(entry.path().generic_string());

            if (count > 0) found.emplace(entry.path().generic_string(), count);
            searched++;
        }

        if (found.empty()) {
            warn("No results found for: " + search_term + " in " + std::to_string(searched));
            return;
        }

        setFOutColour(" --- SEARCH RESULTS FOR: " + search_term + " ---", OutputColour::BrightGreen, {}, true, 1);
        size_t max_len = 0;
        for (const auto& result : found) {
            max_len = std::max(max_len, result.first.length());
        }

        setFOutColour("", OutputColour::BrightBlue);
        for (const auto& result : found) {
            std::cout << std::left << std::setw(static_cast<int>(max_len)) << result.first << " - RESULTS: " << result.second << '\n';
        }
        resetColour();
    }
}
