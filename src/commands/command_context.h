#pragma once

#include <string>

struct GameAddrs {
  uintptr_t world_chr_man = 0;
  uintptr_t add_soul_call = 0;
  bool initialized = false;
};

struct CommandContext {
  GameAddrs *game_addrs = nullptr;
  bool (*resolve_game_addrs)(GameAddrs *addrs) = nullptr;
  bool (*add_runes)(GameAddrs *addrs, int amount, std::string &error) = nullptr;
};
