//
// Created by alex on 10/25/26.
//

#ifndef CJOURNAL_LIST_ENTRIES_H
#define CJOURNAL_LIST_ENTRIES_H

#include "../Classes/command.h"

class list_entries : public command {
public:
    void handle(const std::string &command, const std::vector<std::string> &arguments) override;
    std::string info() override {return "list-entries [fetch_flag] --> lists all journal entries currently saved. Valid flags: --limit (COUNT) / --newest-first (optional amount). If more flags are provided, only the first one will be used.";}
};


#endif //CJOURNAL_LIST_ENTRIES_H
