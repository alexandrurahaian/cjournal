//
// Created by alex on 10/26/26.
//

#include "edit.h"

#include <algorithm>
#include <ccolor.h>
#include <fstream>
#include <vector>

#include <sys/ioctl.h>

#include "../Headers/utility.h"

std::vector<std::string> load_file(const std::string& filename) {
    std::filesystem::path file_path(filename);
    bool is_obfuscated_file = (file_path.extension() == JOURNAL_ENTRY_EXTENSIONS[1]);

    std::vector<std::string> loaded;
    std::string line;

    if (is_obfuscated_file) {
        std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) return {};

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer(size, '\0');
        if (file.read(&buffer[0], size)) {
            std::string decrypted = mask_xor(buffer);
            file.close();

            std::stringstream ss(decrypted);
            while (std::getline(ss, line)) {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                loaded.push_back(line);
            }
        } else {
            file.close();
        }
    }
    else {
        std::ifstream file(filename);
        if (!file.is_open()) return {};

        while (std::getline(file, line)) {
            loaded.push_back(line);
        }
        file.close();
    }

    return loaded;
}


void save(const std::string& filename, const std::vector<std::string>& content) {
    std::filesystem::path file_path(filename);
    bool is_obfuscated_file = (file_path.extension() == JOURNAL_ENTRY_EXTENSIONS[1]);

    if (is_obfuscated_file) {
        std::string full_text;
        for (const auto& line : content) {
            full_text += line + '\n';
        }

        std::string obfuscated = mask_xor(full_text);
        std::ofstream file(filename, std::ios::out | std::ios::trunc | std::ios::binary);
        if (file.is_open()) {
            file.write(obfuscated.data(), obfuscated.size());
            file.close();
        }
    }
    else {
        std::ofstream file(filename, std::ios::trunc);
        if (file.is_open()) {
            for (const auto& line : content) {
                file << line << '\n';
            }
            file.close();
        }
    }
}

void refreshScreen(const std::vector<std::string>& text, size_t cursor_col, size_t cursor_row, size_t row_offset) {
    resetColour();
    std::cout << "\033[2J\033[H";

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_cols = (w.ws_col > 0) ? w.ws_col : 80;
    int term_rows = (w.ws_row > 0) ? w.ws_row : 24;

    int physical_row = 1;
    int target_screen_row = 1;
    int target_screen_col = 1;

    for (size_t r = row_offset; r < text.size(); ++r) {
        if (physical_row >= term_rows) {
            break;
        }

        const std::string& line = text[r];
        std::string display_line = (line.length() > static_cast<size_t>(term_cols))
                                    ? line.substr(0, term_cols)
                                    : line;

        std::cout << display_line << "\r\n";
        if (r == cursor_row) {
            target_screen_row = physical_row;
            target_screen_col = std::min(cursor_col, static_cast<size_t>(term_cols)) + 1;
        }

        physical_row++;
    }

    std::cout << "\033[" << target_screen_row << ";" << target_screen_col << "H";
    std::cout.flush();
}

void edit::handle(const std::string &command, const std::vector<std::string> &arguments) {
    size_t arg_size = arguments.size();
    if (arg_size < 2) {
        error("Please provide a file to edit.");
        return;
    }

    std::string filename = LINUX_DOCUMENTS_FOLDER + '/' + arguments[1];
    std::vector<std::string> contents = load_file(filename);
    if (contents.empty()) {
        contents.push_back("");
    }

    int cursor_row = 0;
    int cursor_col = 0;
    size_t row_offset = 0;

    enableRawMode();

    auto update_scroll = [&]() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int term_rows = (w.ws_row > 0) ? w.ws_row : 24;
        int visible_rows = term_rows > 1 ? term_rows - 1 : 1;

        if (cursor_row < static_cast<int>(row_offset)) {
            row_offset = cursor_row;
        }
        else if (cursor_row >= static_cast<int>(row_offset + visible_rows)) {
            row_offset = cursor_row - visible_rows + 1;
        }
    };

    update_scroll();
    refreshScreen(contents, cursor_col, cursor_row, row_offset);

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        std::string &current_line = contents[cursor_row];

        if (c == 19) {
            save(filename, contents);
            setFOutColour("\nSaved " + filename, OutputColour::BrightGreen, {}, true);
            if (can_increment_streak()) {
                std::filesystem::path path = std::filesystem::path(LINUX_DOCUMENTS_FOLDER) / ".streak";
                std::ofstream file(path, std::ios::trunc);
                if (!file.is_open()) {
                    warn("Failed to open streak file. Not incrementing streak.");
                    break;
                }
                file << std::to_string(std::time(nullptr));
                file.close();

                int longest_streak = get_stat_streak_am(2);
                int current_streak = 0;
                change_stat_value("write_streak", 1, increase_by);
                current_streak = get_stat_streak_am(1);

                if (current_streak > longest_streak)
                    change_stat_value("longest_write_streak", current_streak, set);
            }

            int total_words = 0;
            for (const auto& line : contents) {
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

            change_stat_value("avg_word_count", total_words, increase_by);
            break;
        }
        else if (c == '\033') {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                if (seq[0] == '[') {
                    if (seq[1] == 'D') {
                        if (cursor_col > 0) {
                            cursor_col--;
                        } else if (cursor_row > 0) {
                            cursor_row--;
                            cursor_col = static_cast<int>(contents[cursor_row].length());
                        }
                    }
                    else if (seq[1] == 'C') {
                        if (cursor_col < static_cast<int>(current_line.length())) {
                            cursor_col++;
                        } else if (cursor_row + 1 < static_cast<int>(contents.size())) {
                            cursor_row++;
                            cursor_col = 0;
                        }
                    }
                    else if (seq[1] == 'A') {
                        if (cursor_row > 0) {
                            cursor_row--;
                            cursor_col = std::min(cursor_col, static_cast<int>(contents[cursor_row].length()));
                        }
                    }
                    else if (seq[1] == 'B') {
                        if (cursor_row + 1 < static_cast<int>(contents.size())) {
                            cursor_row++;
                            cursor_col = std::min(cursor_col, static_cast<int>(contents[cursor_row].length()));
                        }
                    }
                }
            }
        }
        else if (c == 127 || c == 8) { // Backspace
            if (cursor_col > 0) {
                current_line.erase(cursor_col - 1, 1);
                cursor_col--;
            } else if (cursor_row > 0) {
                int prev_len = static_cast<int>(contents[cursor_row - 1].length());
                contents[cursor_row - 1] += current_line;
                contents.erase(contents.begin() + cursor_row);
                cursor_row--;
                cursor_col = prev_len;
            }
        }
        else if (c >= 32 && c <= 126) { // Printable characters
            current_line.insert(cursor_col, 1, c);
            cursor_col++;
        }
        else if (c == '\n' || c == '\r') { // Enter key
            std::string remainder = current_line.substr(cursor_col);
            current_line.erase(cursor_col);
            contents.insert(contents.begin() + cursor_row + 1, remainder);
            cursor_row++;
            cursor_col = 0;
        }

        // Update scroll offset based on new cursor position, then redraw
        update_scroll();
        refreshScreen(contents, cursor_col, cursor_row, row_offset);
    }

    disableRawMode(_orig_termios);
    std::cout << "\033[2J\033[H";
}