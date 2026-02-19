#include "npc_spawn.h"

#include "../game_actions.h"
#include "command_context.h"

#include <sstream>

namespace {

std::string handle_npc_spawn_command(CommandContext &ctx,
                                     const std::string &input) {
  if (!ctx.game_addrs || !ctx.resolve_game_addrs) {
    return "Missing game addresses.";
  }

  std::istringstream stream(input);
  std::string verb;
  int npc_id = 0;
  if (!(stream >> verb >> npc_id)) {
    return "Usage: npc <id>";
  }

  std::string error;
  if (!spawn_npc(ctx.game_addrs, npc_id, error)) {
    return "Failed: " + error;
  }

  return "Spawned NPC.";
}

} // namespace

CommandInfo build_npc_spawn_command() {
  CommandInfo info;
  info.name = "npc";
  info.usage = "npc <id>";
  info.description = "Spawn an NPC in front of you.";
  info.handler = handle_npc_spawn_command;
  return info;
}
