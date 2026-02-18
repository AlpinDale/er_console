#pragma once

#include <string>

struct CommandContext;

std::string handle_help_command(CommandContext &ctx, const std::string &input);
struct CommandInfo;
CommandInfo build_help_command();
