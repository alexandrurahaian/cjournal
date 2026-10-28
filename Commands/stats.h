//
// Created by alex on 10/27/26.
//

#ifndef CJOURNAL_STATS_H
#define CJOURNAL_STATS_H
#include "../Classes/command.h"


class stats : public command {
public:
    void handle(const std::string &command, const std::vector<std::string> &arguments) override;
    std::string info() {return "stats --> displays your cjournal statistics.";}
};


#endif //CJOURNAL_STATS_H
