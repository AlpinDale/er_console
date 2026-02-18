#include "runes.h"
#include "command_context.h"

#include <sstream>
#include <vector>

std::string handle_runes_command(CommandContext &ctx, const std::string &input) {
  std::istringstream stream(input);
  std::vector<std::string> parts;
  std::string token;
  while (stream >> token) {
    parts.push_back(token);
  }

  if (parts.size() >= 3 && parts[0] == "runes" && parts[1] == "add") {
    int amount = 0;
    try {
      amount = std::stoi(parts[2]);
    } catch (...) {
      return "Invalid rune amount.";
    }

    if (!ctx.add_runes || !ctx.resolve_game_addrs || !ctx.game_addrs) {
      return "Rune system not initialized.";
    }

    std::string error;
    if (!ctx.add_runes(ctx.game_addrs, amount, error)) {
      return "Failed: " + error;
    }
    return "Added runes: " + std::to_string(amount);
  }

  return "Usage: runes add <amount>";
}
