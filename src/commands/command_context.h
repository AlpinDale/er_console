#pragma once

#include <string>

struct GameAddrs {
  uintptr_t world_chr_man = 0;
  uintptr_t add_soul_call = 0;
  uintptr_t map_item_man_ptr = 0;
  uintptr_t item_give_func = 0;
  bool initialized = false;
};

struct CommandRegistry;

struct CommandContext {
  GameAddrs *game_addrs = nullptr;
  CommandRegistry *registry = nullptr;
  bool (*resolve_game_addrs)(GameAddrs *addrs) = nullptr;
  bool (*add_runes)(GameAddrs *addrs, int amount, std::string &error) = nullptr;
  bool (*add_item)(GameAddrs *addrs, int item_id, int quantity, std::string &error) = nullptr;
};
