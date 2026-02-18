#include "command_registry.h"
#include "command_context.h"

#include <algorithm>
#include <sstream>

void register_command(CommandRegistry &registry, CommandInfo info) {
  registry.commands.push_back(std::move(info));
}

const CommandInfo *find_command(const CommandRegistry &registry, const std::string &name) {
  auto it = std::find_if(registry.commands.begin(), registry.commands.end(),
                         [&](const CommandInfo &cmd) { return cmd.name == name; });
  return it != registry.commands.end() ? &(*it) : nullptr;
}

std::string dispatch_command(CommandRegistry &registry, CommandContext &ctx,
                             const std::string &input) {
  std::istringstream stream(input);
  std::string verb;
  if (!(stream >> verb)) {
    return "";
  }

  const CommandInfo *cmd = find_command(registry, verb);
  if (!cmd || !cmd->handler) {
    return "Unknown command.";
  }

  return cmd->handler(ctx, input);
}
