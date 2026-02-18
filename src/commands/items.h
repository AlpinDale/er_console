#pragma once

#include <string>

struct CommandContext;
struct CommandInfo;

std::string handle_items_command(CommandContext &ctx, const std::string &input);
CommandInfo build_items_command();
