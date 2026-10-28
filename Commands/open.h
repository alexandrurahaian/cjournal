//
// Created by alex on 10/26/26.
//

#ifndef CJOURNAL_OPEN_H
#define CJOURNAL_OPEN_H
#include "../Classes/command.h"


class open : public command{
public:
    void handle(const std::string &command, const std::vector<std::string> &arguments) override;
    std::string info() {return "open (filename) --> Opens the selected journal entry.";}
};


#endif //CJOURNAL_OPEN_H
