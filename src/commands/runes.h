#pragma once

#include <string>

struct CommandContext;

std::string handle_runes_command(CommandContext &ctx, const std::string &input);
struct CommandInfo;
CommandInfo build_runes_command();
