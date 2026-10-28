//
// Created by alex on 10/24/26.
//
#include "../Headers/command_handler.h"
#include "../Classes/command.h"
#include <string>
#include <vector>
#include <iostream>

#include "../Commands/edit.h"
#include "../Commands/list_entries.h"
#include "../Commands/memory.h"
#include "../Commands/new_.h"
#include "../Commands/open.h"
#include "../Commands/remove.h"
#include "../Commands/search.h"
#include "../Commands/stats.h"
#include "../Commands/tags.h"

std::vector<std::string> get_command_arguments(const std::string& raw_command) {
    std::vector<std::string> args;
    std::string arg;

    bool foundQuote = false;
    for (const char& c : raw_command) {
        if (c == '"') {
            foundQuote = !foundQuote;
            continue;
        }

        if (c == ' ' && !foundQuote) {
            if (!arg.empty()) {
                args.push_back(arg);
                arg.clear();
            }
            continue;
        }

        arg += c;
    }

    if (!arg.empty()) {
        args.push_back(arg);
    }
    return args;
}

bool is_valid_command(const std::string& command) {
    if (registered_commands.find(command) != registered_commands.end()) return true;
    return false;
}

void handle(const std::string& command, const std::vector<std::string>& arguments) {
    if (!is_valid_command(command)) return;

    registered_commands.at(command)->handle(command, arguments);
}

void output_help() {
    std::cout << "help --> displays a list of all commands.\ne -- > exits the program without confirmation.\nexit --> exits the program after confirmation.\n";
    for (const auto& cmd : registered_commands)
       std::cout << cmd.second->info() << '\n';
}

void initialise_commands() {
    registered_commands.emplace("list-entries", std::make_unique<list_entries>());
    registered_commands.emplace("open", std::make_unique<open>());
    registered_commands.emplace("edit", std::make_unique<edit>());
    registered_commands.emplace("new", std::make_unique<new_>());
    registered_commands.emplace("remove", std::make_unique<class remove>());
    registered_commands.emplace("search", std::make_unique<search>());
    registered_commands.emplace("tags", std::make_unique<tags>());
    registered_commands.emplace("stats", std::make_unique<stats>());
    registered_commands.emplace("memory", std::make_unique<memory>());
}
