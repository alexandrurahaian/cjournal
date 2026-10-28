//
// Created by alex on 10/27/26.
//

#ifndef CJOURNAL_TAGS_H
#define CJOURNAL_TAGS_H
#include "../Classes/command.h"
#include <unordered_map>

class tags : public command {
public:
    void handle(const std::string &command, const std::vector<std::string> &arguments) override;
    std::string info() {return "tags OPTIONAL ARGUMENTS: -view / -add (tag name) / -remove (tag name) / -set (tag name) (filename) / -unset (tag name) (filename) -->\n-view -> displays a list of all valid tags.\n-add -> adds a new tag\n-remove -> deletes a tag, all files with set tag will also have the tag removed.\n -set & -unset -> sets and unsets the tag for the specified file.\n-getf (tag name+tag name+tag name+etc) --> gets all files that have the specified tag (or tags)";}
    static void change_set(const std::unordered_map<std::string, std::string>& tag_data, const std::string& tag_name, const std::string& f_name, bool unset = false, bool ignore_doesnt_exist = false);
    static std::unordered_map<std::string, std::string> get_tags_from_file();
    static std::vector<std::string> get_files_from_string(const std::string& sequence);
};


#endif //CJOURNAL_TAGS_H
