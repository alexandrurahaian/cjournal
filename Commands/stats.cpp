//
// Created by alex on 10/27/26.
//

#include "stats.h"
#include "../Headers/utility.h"

#include <vector>
#include <string>
#include <fstream>

static void handle_stats(const std::string& stat_name, const std::string& stat_value) {
    setFOutColour("", OutputColour::BrightBlue);
    if (stat_name == "write_streak") std::cout << std::left << std::setw(30) << "Current writing streak: " << stat_value << " days\n";
    else if (stat_name == "longest_write_streak") std::cout << std::left << std::setw(30) << "Longest writing streak: " << stat_value << " days\n";
    else if (stat_name == "avg_word_count") std::cout << std::left << std::setw(30) << "Average word count: " << stat_value << " words\n";
    else if (stat_name == "entries_made") std::cout << std::left << std::setw(30) << "Diary entries made: " << stat_value << " diary entries\n";

    resetColour();
}

void stats::handle(const std::string &command, const std::vector<std::string> &arguments) {
    std::filesystem::path path = std::filesystem::path(LINUX_DOCUMENTS_FOLDER) / STATS_FILENAME;
    std::ifstream file(path.string());

    if (!file.is_open()) {
        error("Could not open stats file: " + path.string());
        return;
    }

    setFOutColour("--- [ STATISTICS ] ---", OutputColour::BrightGreen, {}, true, 1);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line.find('=') == std::string::npos) continue;

        size_t equal_pos = line.find('=');

        std::string stat_name = line.substr(0, equal_pos);
        std::string stat_value = line.substr(equal_pos + 1);

        handle_stats(stat_name, stat_value);
    }
    file.close();
}
