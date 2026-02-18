#include "help.h"

#include "command_context.h"
#include "command_registry.h"

#include <sstream>
#include <vector>

static std::vector<std::string> split_words(const std::string &input) {
  std::istringstream stream(input);
  std::vector<std::string> parts;
  std::string token;
  while (stream >> token) {
    parts.push_back(token);
  }
  return parts;
}

std::string handle_help_command(CommandContext &ctx, const std::string &input) {
  if (!ctx.registry) {
    return "No commands registered.";
  }

  auto parts = split_words(input);
  if (parts.size() >= 2) {
    const CommandInfo *cmd = find_command(*ctx.registry, parts[1]);
    if (!cmd) {
      return "Unknown command.";
    }

    std::string out;
    out += cmd->name + "\n";
    out += "Usage: " + cmd->usage + "\n";
    out += cmd->description;
    return out;
  }

  std::string out = "Commands (use 'help <command>' for details):";
  for (const auto &cmd : ctx.registry->commands) {
    out += "\n" + cmd.name + ": " + cmd.usage + " - " + cmd.description;
  }
  return out;
}

CommandInfo build_help_command() {
  CommandInfo info;
  info.name = "help";
  info.usage = "help [command]";
  info.description = "List all commands or show details for one command.";
  info.handler = handle_help_command;
  return info;
}
