//
// Created by alex on 28 Oct 2026.
//

#include "memory.h"
#include "../Headers/utility.h"
#include <string>
#include <vector>
#include <filesystem>
#include <ctime>

bool is_file_match(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) return false;

    auto ftime = std::filesystem::last_write_time(path);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
    );

    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
    std::tm* file_time_info = std::localtime(&cftime);
    int fileMonth = file_time_info->tm_mon;
    int fileDay = file_time_info->tm_mday;

    std::time_t now = std::time(nullptr);
    std::tm* today_info = std::localtime(&now);

    return (fileMonth == today_info->tm_mon && fileDay == today_info->tm_mday);
}

void memory::handle(const std::string &command, const std::vector<std::string> &arguments) {
    std::vector<file_entry_strct> files = get_journal_entries(LINUX_DOCUMENTS_FOLDER, {"", journal_fetch_option::oldest_first});
    for (const auto& f_entry : files) {
        if (is_file_match(f_entry.path)) {
            std::ifstream file(f_entry.path.string());
            if (!file.is_open()) continue;
            int extracted_length = 0;
            std::string line;
            std::getline(file, line);
            file.close();
            if (!line.empty()) {
                auto ftime = std::filesystem::last_write_time(f_entry.path);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                line = mask_xor(line);
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                std::tm* file_time_info = std::localtime(&cftime);
                std::string text = "On " + std::to_string(file_time_info->tm_mday) + '/' + std::to_string(file_time_info->tm_mon) + '/' + std::to_string(file_time_info->tm_year % 100) + " @ " + std::to_string(file_time_info->tm_hour) + ':' + std::to_string(file_time_info->tm_min) +':'+ std::to_string(file_time_info->tm_sec) + ":\n";
                setFOutColour(text, OutputColour::BrightBlue, {}, false, 1);
                std::cout << line << '\n';
                resetColour();
                return;
            }
        }
    }

    setFOutColour("No diary entry written on this date found.", OutputColour::BrightBlue, {}, true, 1);
}
