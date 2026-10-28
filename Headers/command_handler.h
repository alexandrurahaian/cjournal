//
// Created by alex on 10/24/26.
//

#ifndef CJOURNAL_COMMAND_HANDLER_H
#define CJOURNAL_COMMAND_HANDLER_H
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "../Classes/command.h"

inline std::unordered_map<std::string, std::unique_ptr<command>> registered_commands;

std::vector<std::string> get_command_arguments(const std::string& raw_command);
bool is_valid_command(const std::string& command);
void handle(const std::string& command, const std::vector<std::string>& arguments);
void output_help();
void initialise_commands();

#endif //CJOURNAL_COMMAND_HANDLER_H
