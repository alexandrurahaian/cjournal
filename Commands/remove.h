//
// Created by alex on 10/27/26.
//

#ifndef CJOURNAL_REMOVE_H
#define CJOURNAL_REMOVE_H
#include "../Classes/command.h"


class remove : public command {
public:
    void handle(const std::string &command, const std::vector<std::string> &arguments) override;
    std::string info() {return "remove (filename) OPTIONAL FLAG: --F --> Removes the specified file.\nIf the '-F' flag is provided, it will do so without asking for confirmation first.";}
};


#endif //CJOURNAL_REMOVE_H
