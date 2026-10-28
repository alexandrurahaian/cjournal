//
// Created by alex on 28 Oct 2026.
//

#ifndef CJOURNAL_MEMORY_H
#define CJOURNAL_MEMORY_H
#include "../Classes/command.h"


class memory : public command {
public:
    void handle(const std::string &command, const std::vector<std::string> &arguments) override;
    std::string info() {return "memory --> displays a snippet of a diary entry from the past written on the same date.";}
};


#endif //CJOURNAL_MEMORY_H
