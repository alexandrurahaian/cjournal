//
// Created by alex on 10/26/26.
//

#ifndef CJOURNAL_NEW__H
#define CJOURNAL_NEW__H
#include "../Classes/command.h"


class new_ : public command {
public:
    void handle(const std::string &command, const std::vector<std::string> &arguments) override;
    std::string info() {return "new (filename) OPTIONAL flags: --obfuscate (if the file should be obfuscated upon saving).";}
};


#endif //CJOURNAL_NEW__H
