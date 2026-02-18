#include "console_commands.h"

#include "commands/command_context.h"
#include "commands/command_registry.h"
#include "commands/help.h"
#include "commands/items.h"
#include "commands/runes.h"
#include "commands/search.h"
#include "game_actions.h"
#include "item_queue.h"

#include <algorithm>
#include <sstream>

namespace {

std::string handle_skyrim_additem(GameAddrs &addrs,
                                  std::istringstream &stream) {
  std::string item_hex;
  long long amount = 0;
  if (!(stream >> item_hex >> amount)) {
    return {};
  }

  std::string hex = item_hex;
  if (hex.rfind("0x", 0) == 0 || hex.rfind("0X", 0) == 0) {
    hex = hex.substr(2);
  }
  if (hex.size() == 1 && (hex == "f" || hex == "F")) {
    hex = "0F";
  }

  int id = 0;
  try {
    id = std::stoi(hex, nullptr, 16);
  } catch (...) {
    id = 0;
  }

  if (id == 0x0F && amount > 0) {
    std::string error;
    if (!add_runes(&addrs, static_cast<int>(amount), error)) {
      return "Failed: " + error;
    }
    return "Added runes: " + std::to_string(amount);
  }

  return {};
}

std::string handle_toggle_god_mode(GameAddrs &addrs) {
  bool enabled = false;
  std::string error;
  if (!toggle_god_mode(&addrs, enabled, error)) {
    return "Failed: " + error;
  }
  return enabled ? "God mode: on" : "God mode: off";
}

std::string handle_toggle_no_clip(GameAddrs &addrs) {
  bool enabled = false;
  std::string error;
  if (!toggle_no_clip(&addrs, enabled, error)) {
    return "Failed: " + error;
  }
  return enabled ? "NoClip: on" : "NoClip: off";
}

void ensure_commands_registered(CommandRegistry &registry) {
  if (!registry.commands.empty()) {
    return;
  }
  register_command(registry, build_help_command());
  register_command(registry, build_runes_command());
  register_command(registry, build_items_command());
  register_command(registry, build_search_command());
}

} // namespace

std::string handle_console_command(CommandRegistry &registry, GameAddrs &addrs,
                                   const std::string &command) {
  std::istringstream stream(command);
  std::string verb;
  stream >> verb;
  std::string verb_lower = verb;
  std::transform(
      verb_lower.begin(), verb_lower.end(), verb_lower.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  if (verb_lower == "player.additem") {
    std::string response = handle_skyrim_additem(addrs, stream);
    if (!response.empty()) {
      return response;
    }
  }

  if (verb_lower == "tgm") {
    return handle_toggle_god_mode(addrs);
  }

  if (verb_lower == "tcl") {
    return handle_toggle_no_clip(addrs);
  }

  CommandContext ctx;
  ctx.game_addrs = &addrs;
  ctx.registry = &registry;
  ctx.resolve_game_addrs = resolve_game_addrs;
  ctx.add_runes = add_runes;
  ctx.add_item = queue_item;

  ensure_commands_registered(registry);
  return dispatch_command(registry, ctx, command);
}
