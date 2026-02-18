#include "items.h"
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

std::string handle_items_command(CommandContext &ctx, const std::string &input) {
  auto parts = split_words(input);
  if (parts.size() >= 5 && parts[0] == "item" && parts[1] == "add") {
    std::string type = parts[2];
    int item_id = 0;
    int quantity = 0;
    int upgrade = 0;
    try {
      item_id = std::stoi(parts[3]);
      quantity = std::stoi(parts[4]);
      if (parts.size() >= 6) {
        upgrade = std::stoi(parts[5]);
      }
    } catch (...) {
      return "Invalid item arguments.";
    }

    int type_offset = -1;
    if (type == "weapon") {
      type_offset = 0x0;
    } else if (type == "armor") {
      type_offset = 0x10000000;
    } else if (type == "talisman") {
      type_offset = 0x20000000;
    } else if (type == "goods") {
      type_offset = 0x40000000;
    } else {
      return "Invalid item type. Use weapon, armor, talisman, goods.";
    }

    if (upgrade < 0) {
      return "Upgrade must be >= 0.";
    }

    int final_id = item_id + type_offset;
    if (type == "weapon") {
      final_id += upgrade;
    }

    if (!ctx.add_item || !ctx.resolve_game_addrs || !ctx.game_addrs) {
      return "Item system not initialized.";
    }

    std::string error;
    if (!ctx.add_item(ctx.game_addrs, final_id, quantity, error)) {
      return "Failed: " + error;
    }
    return "Item add queued: " + std::to_string(item_id) + " x" +
           std::to_string(quantity);
  }

  return "Usage: item add <type> <item_id> <quantity> [upgrade]";
}

CommandInfo build_items_command() {
  CommandInfo info;
  info.name = "item";
  info.usage = "item add <type> <item_id> <quantity> [upgrade]";
  info.description = "Add an item by ID with type: weapon|armor|talisman|goods.";
  info.handler = handle_items_command;
  return info;
}
