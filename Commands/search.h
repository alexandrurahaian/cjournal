//
// Created by alex on 10/27/26.
//

#ifndef CJOURNAL_SEARCH_H
#define CJOURNAL_SEARCH_H
#include "../Classes/command.h"


class search : public command {
public:
    void handle(const std::string &command, const std::vector<std::string> &arguments) override;
    std::string info() {return "search (search term) OPTIONAL ARGUMENT: --T (filename) --> Searches the file (if --T is provided) or all files for the search term.";}
};


#endif //CJOURNAL_SEARCH_H
