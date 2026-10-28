//
// Created by alex on 10/24/26.
//

#ifndef CJOURNAL_COMMAND_H
#define CJOURNAL_COMMAND_H

#include <string>
#include <vector>

class command {
public:
    virtual ~command() = default;
    virtual void handle(const std::string& command, const std::vector<std::string>& arguments) = 0;
    virtual std::string info() = 0;
};


#endif //CJOURNAL_COMMAND_H