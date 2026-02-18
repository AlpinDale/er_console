#pragma once

#include <string>
#include <vector>

struct CommandContext;

using CommandHandler = std::string (*)(CommandContext &ctx, const std::string &input);

struct CommandInfo {
  std::string name;
  std::string usage;
  std::string description;
  CommandHandler handler = nullptr;
};

struct CommandRegistry {
  std::vector<CommandInfo> commands;
};

void register_command(CommandRegistry &registry, CommandInfo info);
const CommandInfo *find_command(const CommandRegistry &registry, const std::string &name);
std::string dispatch_command(CommandRegistry &registry, CommandContext &ctx,
                             const std::string &input);
