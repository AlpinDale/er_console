#include "command_registry.h"
#include "command_context.h"

#include <sstream>

std::string dispatch_command(CommandRegistry &registry, CommandContext &ctx,
                             const std::string &input) {
  std::istringstream stream(input);
  std::string verb;
  if (!(stream >> verb)) {
    return "";
  }

  if (verb == "runes" && registry.runes) {
    return registry.runes(ctx, input);
  }

  return "Unknown command.";
}
