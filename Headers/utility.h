//
// Created by alex on 10/25/26.
//

#ifndef CJOURNAL_UTILITY_H
#define CJOURNAL_UTILITY_H

#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>
#include <ccolor.h>
#include <climits>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <unordered_map>

enum journal_fetch_option {
    today,
    limit,
    newest_first,
    oldest_first,
    none
};

enum config_value_type {
    INT,
    TEXT,
    BOOLEAN
};

enum stat_value_change_type {
    increase_by,
    decrease_by,
    set
};

struct fetch_option {
    std::string value;
    journal_fetch_option option = none;
};

struct config {
    int max_lines_per_page = 20;
};

inline const std::string JOURNAL_ENTRY_EXTENSIONS[2] = {".cj", ".ocj"};
constexpr static std::string_view CONFIG_FILENAME = ".config";
constexpr static std::string_view TAGS_FILENAME = ".tags";
constexpr static std::string_view STATS_FILENAME = ".stats";
constexpr static std::string_view XOR_PASSWORD = "chC14560hho19f;'v1nCWO!@51gf!g3*^134bn3gf";

inline std::string LINUX_DOCUMENTS_FOLDER = "none";
inline config FETCHED_CONFIG = {};
inline struct termios _orig_termios;

static void handle_config_value(const std::string& config_name, const std::string& config_value) {
    try {
        if (config_name == "max_lines_per_page") FETCHED_CONFIG.max_lines_per_page = std::stoi(config_value);
        else warn("Unknown configuration name: " + config_name);
    }
    catch (...) {
        error("Failed to convert " + config_value + " for " + config_name);
    }
}

inline std::string mask_xor(const std::string& input) {
    std::string output = input;
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] ^ XOR_PASSWORD[i % XOR_PASSWORD.size()];
    }

    return output;
}

inline void change_stat_value(const std::string& stat_name, const int stat_value, stat_value_change_type value_change_type) {
    std::filesystem::path old_path = std::filesystem::path(LINUX_DOCUMENTS_FOLDER) / STATS_FILENAME;
    std::filesystem::path new_path = std::filesystem::path(LINUX_DOCUMENTS_FOLDER) / ".new_stats";

    std::ifstream old_file(old_path);
    std::ofstream new_file(new_path);

    if (!old_file.is_open()) {
        error("Could not open old stats file for updating.");
        return;
    }
    if (!new_file.is_open()) {
        error("Could not open new stats file for updating.");
        return;
    }

    auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
    };

    bool found = false;
    std::string line;

    while (std::getline(old_file, line)) {
        if (!line.empty() && line.find('=') != std::string::npos) {
            size_t equal_pos = line.find('=');
            std::string extracted_name = line.substr(0, equal_pos);
            trim(extracted_name); // Clean up any accidental spaces

            if (extracted_name == stat_name) {
                found = true;
                if (value_change_type == stat_value_change_type::set) {
                    new_file << stat_name << '=' << stat_value << '\n';
                }
                else if (value_change_type == stat_value_change_type::increase_by ||
                         value_change_type == stat_value_change_type::decrease_by) {
                    try {
                        std::string val_str = line.substr(equal_pos + 1);
                        trim(val_str);
                        int old_val = std::stoi(val_str);

                        if (value_change_type == stat_value_change_type::increase_by) {
                            old_val += stat_value;
                        } else {
                            old_val = std::clamp(old_val - stat_value, 0, INT_MAX);
                        }

                        new_file << stat_name << '=' << old_val << '\n';
                    }
                    catch (...) {
                        error("Failed to convert old stat value of " + stat_name + " to int.");
                        new_file << line << '\n';
                    }
                }
                continue;
            }
        }
        new_file << line << '\n';
    }

    old_file.close();
    new_file.close();

    try {
        std::filesystem::rename(new_path, old_path);
    }
    catch (const std::filesystem::filesystem_error& e) {
        setFOutColour("", OutputColour::BrightRed);
        std::cout << "Failed to overwrite old stats file --> " << e.what() << '\n';
        resetColour();
    }
}

inline bool can_increment_streak() {
    std::filesystem::path path = std::filesystem::path(LINUX_DOCUMENTS_FOLDER) / ".streak";
    std::ifstream unix_file(path);
    if (!unix_file.is_open()) {
        warn("Could not open file, allowing streak increase.");
        return true;
    }

    std::string str_unix;
    std::getline(unix_file, str_unix);
    unix_file.close();

    if (str_unix.empty()) return true;

    std::time_t file_unix = std::stoll(str_unix);
    std::time_t current_unix = std::time(nullptr);

    if (current_unix < file_unix) return true;

    long long difference = current_unix - file_unix;

    if (difference >= 172800) {
        change_stat_value("write_streak", 0, set);
        return true;
    }

    std::tm* file_ptr = std::localtime(&file_unix);
    if (!file_ptr) return true;
    std::tm local_file = *file_ptr;

    std::tm* current_ptr = std::localtime(&current_unix);
    if (!current_ptr) return true;
    std::tm local_current = *current_ptr;

    if (local_file.tm_year == local_current.tm_year &&
        local_file.tm_yday == local_current.tm_yday) {
        return false;
        }

    return true;
}

inline int get_stat_streak_am(const int option = 1) {
    // option 1 --> current streak
    // option 2 --> longest streak;
    std::filesystem::path path = std::filesystem::path(LINUX_DOCUMENTS_FOLDER) / STATS_FILENAME;
    std::ifstream file(path);
    if (!file.is_open()) {
        error("Could not open stats file to fetch current streak values.");
        return 0;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.find('=') != std::string::npos) {
            size_t equal_pos = line.find_first_of('=');
            std::string extracted_name = line.substr(0, equal_pos);
            int value;
            try {
                value = std::stoi(line.substr(equal_pos + 1));
            }
            catch (...) {
                error("Failed to convert value from line: " + line);
                continue;
            }

            if ((option == 1 && extracted_name == "write_streak") ||
                (option == 2 && extracted_name == "longest_write_streak")) {
                file.close();
                return  value;
            }
        }
    }
    file.close();
    return 0;
}

inline void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    _orig_termios = raw;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_iflag &= ~(IXON | ICRNL);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

inline void disableRawMode(struct termios &orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

inline void get_config() {
    if (LINUX_DOCUMENTS_FOLDER.empty() || !std::filesystem::exists(LINUX_DOCUMENTS_FOLDER)) return;

    std::string config_file_path = LINUX_DOCUMENTS_FOLDER + '/' + CONFIG_FILENAME.data();
    std::ifstream c_file(config_file_path);
    if (!c_file.is_open()) {
        error("Failed to open configuration file: " + config_file_path);
        return;
    }

    std::string line;
    while (std::getline(c_file, line)) {
        if (line.empty()) continue;
        size_t equal_pos = line.find('=');
        if (equal_pos == std::string::npos) continue;

        std::string config_name = line.substr(0, equal_pos);
        std::string config_value = line.substr(equal_pos + 1);

        handle_config_value(config_name, config_value);
    }
}

inline void get_linux_doc_folder() {
    const char* home_env = std::getenv("HOME");
    if (!home_env) return;
    std::string home(home_env);

    std::filesystem::path config_path = std::filesystem::path(home) / ".config" / "user-dirs.dirs";
    std::ifstream configFile(config_path);

    if (!configFile.is_open()) return;

    std::string line;
    while (std::getline(configFile, line)) {
        if (line.rfind("XDG_DOCUMENTS_DIR=", 0) == 0) {
            size_t start = line.find('"');
            size_t end = line.rfind('"');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                std::string raw_path = line.substr(start + 1, end - start - 1);

                if (raw_path.rfind("$HOME", 0) == 0) {
                    LINUX_DOCUMENTS_FOLDER = home + raw_path.substr(5);
                    break;
                }
                LINUX_DOCUMENTS_FOLDER = raw_path;
                break;
            }
        }
    }

    std::filesystem::path dir(LINUX_DOCUMENTS_FOLDER + "/cjournal");

    if (!LINUX_DOCUMENTS_FOLDER.empty()) {
        std::filesystem::path dirPath = std::filesystem::path(LINUX_DOCUMENTS_FOLDER) / "cjournal";
        try {
            std::filesystem::create_directories(dirPath);
            LINUX_DOCUMENTS_FOLDER = dirPath.string();
        }
        catch (const std::filesystem::filesystem_error& e) {
            error("Failed to create cjournal directory: " + std::string(e.what()));
            LINUX_DOCUMENTS_FOLDER.clear();
        }
    }

    if (LINUX_DOCUMENTS_FOLDER.empty() || LINUX_DOCUMENTS_FOLDER == "none") {
        LINUX_DOCUMENTS_FOLDER = (std::filesystem::path(home) / "Documents").string();
    }

    if (!std::filesystem::is_regular_file(dir / CONFIG_FILENAME)) {
        std::ofstream config_file(dir / CONFIG_FILENAME);
        config_file.close();
    }

    if (!std::filesystem::is_regular_file(dir / TAGS_FILENAME)) {
        std::ofstream tags_file(dir / TAGS_FILENAME);
        tags_file.close();
    }

    if (!std::filesystem::is_regular_file(dir / STATS_FILENAME)) {
        std::ofstream stats_file(dir / STATS_FILENAME);
        if (!stats_file.is_open()) {
            error("Failed to open stats file!");
            return;
        }
        stats_file << "write_streak=0\n";
        stats_file << "longest_write_streak=0\n";
        stats_file << "avg_word_count=0\n";
        stats_file << "entries_made=0\n";
        stats_file.close();
    }
}

inline bool has_journal_extension(const std::string& extension) {
    for (const auto& ext : JOURNAL_ENTRY_EXTENSIONS)
        if (ext == extension) return true;
    return false;
}

struct file_entry_strct {
    std::filesystem::path path;
    std::filesystem::file_time_type write_time;
};

static void merge(std::vector<file_entry_strct>& arr, journal_fetch_option fetch_option, int left, int right, int mid) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<file_entry_strct> l(n1), r(n2);
    for (int i = 0; i < n1; i++)
        l[i] = arr[left + i];
    for (int i = 0; i < n2; i++)
        r[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        bool take_left = false;

        switch (fetch_option) {
            case newest_first:
                take_left = (l[i].write_time >= r[j].write_time);
                break;
            case oldest_first:
                take_left = (l[i].write_time <= r[j].write_time);
                break;
            default:
                take_left = (l[i].write_time >= r[j].write_time);
                break;
        }

        if (take_left) {
            arr[k] = l[i];
            i++;
        } else {
            arr[k] = r[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = l[i];
        i++;
        k++;
    }
    while (j < n2) {
        arr[k] = r[j];
        j++;
        k++;
    }
}

static void sort_vector(std::vector<file_entry_strct>& arr, int left, int right, journal_fetch_option fetch_option) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    sort_vector(arr, left, mid, fetch_option);
    sort_vector(arr, mid + 1, right, fetch_option);
    merge(arr, fetch_option, left, right, mid);
}

inline std::vector<file_entry_strct> get_journal_entries(const std::string& directory = LINUX_DOCUMENTS_FOLDER, fetch_option option = {}) {
    std::vector<file_entry_strct> fetched;
    int count = 0;

    for (const auto& dir_entry : std::filesystem::directory_iterator(directory, std::filesystem::directory_options::skip_permission_denied)) {
        if (dir_entry.is_regular_file() && has_journal_extension(dir_entry.path().extension())) {
            fetched.push_back({dir_entry.path(), std::filesystem::last_write_time(dir_entry.path())});
            count++;
            if (!option.value.empty()) {
                if (option.option == limit && count >= std::stoi(option.value)) return fetched;

                if ((option.option == newest_first || option.option == oldest_first) && count >= std::stoi(option.value)) {
                    sort_vector(fetched, 0, static_cast<int>(fetched.size() - 1), option.option);
                    return fetched;
                }
            }
        }
    }

    if (option.option == newest_first || option.option == oldest_first) sort_vector(fetched, 0, static_cast<int>(fetched.size() - 1), option.option);
    
    return fetched;
}

inline int get_average_word_count() {
    const std::vector<file_entry_strct>& files = get_journal_entries();
    if (files.empty()) return 0;

    int total_words = 0;
    int valid_files = 0;

    for (const auto& f : files) {
        std::ifstream file(f.path);
        if (!file.is_open()) continue;

        valid_files++;
        std::string line;
        while (std::getline(file, line)) {
            bool in_word = false;
            for (char c : line) {
                if (std::isspace(static_cast<unsigned char>(c))) {
                    in_word = false;
                } else if (!in_word) {
                    in_word = true;
                    total_words++;
                }
            }
        }
        file.close();
    }

    if (valid_files == 0) return 0;
    return total_words / valid_files;
}

#endif //CJOURNAL_UTILITY_H
