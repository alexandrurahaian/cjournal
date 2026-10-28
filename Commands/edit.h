//
// Created by alex on 10/26/26.
//

#ifndef CJOURNAL_EDIT_H
#define CJOURNAL_EDIT_H
#include "../Classes/command.h"


class edit : public command{
public:
    void handle(const std::string &command, const std::vector<std::string> &arguments) override;
    std::string info() {return "edit (filename.ext) -- > edits the selected file.";}
};


#endif //CJOURNAL_EDIT_H
