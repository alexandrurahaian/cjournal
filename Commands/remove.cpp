//
// Created by alex on 10/27/26.
//

#include "remove.h"
#include "tags.h"
#include <vector>
#include <string>
#include "../Headers/utility.h"
#include <ccolor.h>
#include <filesystem>

void remove::handle(const std::string &command, const std::vector<std::string> &arguments) {
    size_t arg_size = arguments.size();

    if (arg_size < 2) {
        error("Please provide a file name.");
        return;
    }
    std::filesystem::path path = std::filesystem::path(LINUX_DOCUMENTS_FOLDER) / arguments[1];
    if (!std::filesystem::exists(path)) {
        error(arguments[1] + " does not exist! Please provide a valid file name!");
        return;
    }

    const auto delete_file = [path]() {
        if (std::filesystem::remove(path)) {
            setFOutColour("Successfully deleted " + path.generic_string(), OutputColour::Blue, {}, true, 1);
            const std::unordered_map<std::string, std::string>& tags = tags::get_tags_from_file();

            std::string base_folder = LINUX_DOCUMENTS_FOLDER;
            if (base_folder.back() != '/') base_folder += '/';

            for (const auto& tag_entry : tags) {
                std::vector<std::string> files = tags::get_files_from_string(tag_entry.second);
                for (const auto& file : files) {
                    if (file == path.string()) {
                        std::string relative_name = file;
                        if (file.rfind(base_folder, 0) == 0) {
                            relative_name = file.substr(base_folder.length());
                        }
                        tags::change_set(tags, tag_entry.first, relative_name, true, true);
                        break;
                    }
                }
            }
        }
        else error("Failed to delete " + path.generic_string());
    };

    if (arg_size >= 3 && arguments[2] == "-F") delete_file();
    else if (arg_size >= 3 && arguments[2] != "-F") error("Unknown argument: " + arguments[2]);
    else {
        std::string op;
        setFOutColour("Are you sure you wish to permanently delete: " + path.generic_string() + "? (y/n): ", OutputColour::BrightYellow, {}, true);
        std::getline(std::cin, op);
        if (op == "y" || op == "Y") delete_file();
        else warn("Cancelled. " + path.generic_string() + " has not been modified.");
    }
}