//
// Created by alex on 10/27/26.
//

#include "tags.h"
#include <string>
#include <ccolor.h>
#include <complex>
#include <vector>
#include <fstream>
#include <unordered_map>

#include "../Headers/utility.h"

std::unordered_map<std::string, std::string> tags::get_tags_from_file() {
    std::unordered_map<std::string, std::string> fetched_tags;
    std::ifstream file(LINUX_DOCUMENTS_FOLDER + '/' + TAGS_FILENAME.data());

    if (!file.is_open()) {
        error("Could not open " + LINUX_DOCUMENTS_FOLDER + '/' + TAGS_FILENAME.data());
        return fetched_tags;
    }

    std::string line;
    std::string last_tag;
    std::string tagged_files;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line[0] == '[') {
            if (!last_tag.empty()) {
                fetched_tags.emplace(last_tag, tagged_files);
                tagged_files.clear();
            }
            size_t last_bracket = line.find_last_of(']');
            std::string tag_name = line.substr(1, last_bracket - 1);
            if (tag_name.empty()) {
                error("Failed to get tag from line: " + line);
                return fetched_tags;
            }
            last_tag = tag_name;
        }
        else tagged_files += ('|' + line);
    }
    file.close();
    if (fetched_tags.find(last_tag) == fetched_tags.end())
        fetched_tags.emplace(last_tag, tagged_files);

    return fetched_tags;
}

std::vector<std::string> tags::get_files_from_string(const std::string& sequence) {
    std::vector<std::string> extracted;
    std::string part;
    for (const auto& c : sequence) {
        if (c != '|') part += c;
        else {
            extracted.push_back(part);
            part.clear();
        }
    }

    if (!part.empty()) extracted.push_back(part);
    return extracted;
}

void tags::change_set(const std::unordered_map<std::string, std::string>& tag_data, const std::string& tag_name,  const std::string& f_name, bool unset, bool ignore_doesnt_exist) {
        std::string file_name = LINUX_DOCUMENTS_FOLDER + '/' + f_name;

        if (!ignore_doesnt_exist && tag_data.find(tag_name) == tag_data.end()) {
            error(tag_name + " does not exist!");
            return;
        }
        /*if (!ignore_doesnt_exist && !std::filesystem::is_regular_file(file_name)) {
            error(file_name + " does not exist!");
            return;
        }*/
        if (!unset && tag_data.at(tag_name).find(file_name) != std::string::npos) {
            error(file_name + " already has the tag " + tag_name);
            return;
        }
        if (!ignore_doesnt_exist && unset && tag_data.at(tag_name).find(file_name) == std::string::npos) {
            error(file_name + " is not tagged with " + tag_name);
            return;
        }

        std::string old_tag_file = LINUX_DOCUMENTS_FOLDER + '/' + TAGS_FILENAME.data();
        std::string new_tag_file = LINUX_DOCUMENTS_FOLDER + "/.new_tags";

        std::ifstream old_file(old_tag_file);
        std::ofstream new_file(new_tag_file);

        if (!old_file.is_open()) {
            error("Failed to open old tag file for adding.");
            return;
        }
        if (!new_file.is_open()) {
            error("Failed to open new tag file for adding.");
            return;
        }

        std::string line;
        std::string prev_extracted_name;
        while (std::getline(old_file, line)) {
            if (!line.empty() && line[0] == '[') {
                new_file << line << '\n';
                std::string extracted_name = line.substr(1, line.find_last_of(']') - 1);
                std::cout << extracted_name << '\n';
                if (extracted_name == tag_name && !unset) {
                    new_file << file_name << '\n';
                }
                prev_extracted_name = extracted_name;
                continue;
            }
            if (unset && line.find(file_name) != std::string::npos && prev_extracted_name == tag_name) continue;
            new_file << line << '\n';
        }

        old_file.close();
        new_file.close();

        try {
            std::filesystem::rename(new_tag_file, old_tag_file);
            setFOutColour((unset ? "Successfully unset " : "Successfully set ") + tag_name + " to " + file_name + '!', OutputColour::BrightGreen, {}, true, 1);
        }
        catch (const std::filesystem::filesystem_error& e) {
            setFOutColour("Failed to overwrite old tags file --> ", OutputColour::BrightRed, {});
            std::cout << e.what() << '\n';
            resetColour();
        }
}

void tags::handle(const std::string &command, const std::vector<std::string> &arguments) {
    size_t arg_size = arguments.size();
    if (arg_size < 2) {
        error("Argument mismatch, please provide at least 1 argument.");
        return;
    }
    std::unordered_map<std::string, std::string> tag_data = get_tags_from_file();
    std::string option = arguments[1];

    if (option == "-view") {
        setFOutColour(" --- [ TAGS ] ---", OutputColour::BrightGreen, {}, true, 1);
        for (const auto& res : tag_data) {
            setFOutColour(res.first, OutputColour::Blue, {}, false, 1);
        }
        resetColour();
    }
    else if (option == "-add") {
        if (arg_size < 3) {
            error("Please provide a name for the tag.");
            return;
        }

        std::string tag_name = arguments[2];
        if (tag_data.find(tag_name) != tag_data.end()) {
            error(tag_name + " already exists!");
            return;
        }

        std::ofstream file(LINUX_DOCUMENTS_FOLDER + '/' + TAGS_FILENAME.data(), std::ios::app);
        if (!file.is_open()) {
            error("Failed to open .tags file to save added tag!");
            return;
        }

        file << '[' + tag_name + "]\n";
        file.close();
    }
    else if (option == "-remove") {
        if (arg_size < 3) {
            error("Please provide the name of the tag.");
            return;
        }
        std::string tag_name = arguments[2];
        if (tag_data.find(tag_name) == tag_data.end()) {
            error(tag_name + " doesn't exist!");
            return;
        }
        std::string tag_file = LINUX_DOCUMENTS_FOLDER + '/' + TAGS_FILENAME.data();
        std::string new_tag_file = LINUX_DOCUMENTS_FOLDER + "/.new_tags";

        std::ifstream old_file(tag_file);
        std::ofstream new_file(new_tag_file);

        if (!old_file.is_open()) {
            error("Failed to open old tags file for editing!");
            return;
        }
        if (!new_file.is_open()) {
            error("Failed to open new tags file for editing!");
            return;
        }
        bool continue_delete = false;
        std::string line;
        while (std::getline(old_file, line)) {
            if (!line.empty() && line[0] == '[') {
                std::string name = line.substr(1, line.find_last_of(']') - 1);
                if (name == tag_name) {
                    continue_delete = true;
                    continue;
                }
                continue_delete = false;
            }
            if (!continue_delete)
                new_file << line << '\n';
        }
        old_file.close();
        new_file.close();

        try {
            std::filesystem::rename(new_tag_file, tag_file);
            setFOutColour("Successfully removed " + tag_name + '!', OutputColour::BrightGreen, {}, true, 1);
        }
        catch (const std::filesystem::filesystem_error& e) {
            setFOutColour("Failed to overwrite old tags file --> ", OutputColour::BrightRed, {});
            std::cout << e.what() << '\n';
            resetColour();
        }
    }
    else if (option == "-set" || option == "-unset") {
        if (arg_size < 3 || arg_size < 4) {
            error("Please provide the name of the tag and the file name.");
            return;
        }
        change_set(tag_data, arguments[2], arguments[3], option == "-unset");
    }
    else if (option == "-getf") {
        if (arg_size < 3) {
            error("Please provide tag or list of tags you would like to search for.");
            return;
        }

        std::vector<std::string> tags_to_search;
        std::unordered_map<std::string, int> validated_files;
        std::string tag;

        for (const auto& c : arguments[2]) {
            if (c != '+') tag += c;
            else {
                tags_to_search.push_back(tag);
                tag.clear();
            }
        }
        if (!tag.empty()) tags_to_search.push_back(tag);

        setFOutColour("--- [ FILES CONTAINING TAGS: " + arguments[2] + " ] ---", OutputColour::BrightGreen, {}, true, 1);
        for (const auto& etag : tags_to_search) {
            if (tag_data.find(tag) != tag_data.end()) {
                const std::vector<std::string>& files = get_files_from_string(tag_data[etag]);
                for (const auto& file : files) {
                    if (validated_files.find(file) != validated_files.end()) validated_files[file]++;
                    else validated_files.emplace(file, 1);
                }
            }
        }

        setFOutColour("", OutputColour::Blue, {}, false);
        int nr_of_tags = static_cast<int>(tags_to_search.size());
        for (const auto& file_pair : validated_files) {
            if (file_pair.second == nr_of_tags)
                std::cout << file_pair.first << '\n';
        }

        resetColour();
    }
    else error("Unknown: " + option);
}