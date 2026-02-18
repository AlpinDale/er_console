#include "teleport.h"

#include "../memory_utils.h"
#include "command_context.h"

#include <sstream>

namespace {

std::string handle_teleport_command(CommandContext &ctx,
                                    const std::string &input) {
  if (!ctx.game_addrs || !ctx.resolve_game_addrs) {
    return "Missing game addresses.";
  }
  if (!ctx.resolve_game_addrs(ctx.game_addrs)) {
    return "Failed to resolve game addresses.";
  }
  if (!ctx.game_addrs->world_chr_man) {
    return "WorldChrMan not found.";
  }

  std::istringstream stream(input);
  std::string verb;
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  if (!(stream >> verb >> x >> y >> z)) {
    return "Usage: tp <x> <y> <z>";
  }

  auto world_chr_man =
      *reinterpret_cast<uintptr_t *>(ctx.game_addrs->world_chr_man);
  if (!world_chr_man) {
    return "WorldChrMan is null.";
  }

  auto player_list = *reinterpret_cast<uintptr_t *>(world_chr_man + 0x10EF8);
  if (!player_list) {
    return "Player list is null.";
  }

  auto player = *reinterpret_cast<uintptr_t *>(player_list);
  if (!player) {
    return "Player is null.";
  }

  uintptr_t ptr1 = 0;
  if (!safe_read_ptr(player + 0x190, ptr1) || !ptr1) {
    return "Failed to write player coordinates.";
  }

  uintptr_t ptr2 = 0;
  if (!safe_read_ptr(ptr1 + 0x68, ptr2) || !ptr2) {
    return "Failed to write player coordinates.";
  }

  if (!safe_write_f32(ptr2 + 0x70, x) || !safe_write_f32(ptr2 + 0x74, z) ||
      !safe_write_f32(ptr2 + 0x78, y)) {
    return "Failed to write player coordinates.";
  }

  return "Teleported.";
}

} // namespace

CommandInfo build_teleport_command() {
  CommandInfo info;
  info.name = "tp";
  info.usage = "tp <x> <y> <z>";
  info.description = "Teleport to coordinates (x y z).";
  info.handler = handle_teleport_command;
  return info;
}
