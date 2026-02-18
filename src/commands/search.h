#pragma once

#include <string>

struct CommandContext;
struct CommandInfo;

std::string handle_search_command(CommandContext &ctx,
                                  const std::string &input);
CommandInfo build_search_command();
