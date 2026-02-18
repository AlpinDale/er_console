#pragma once

#include "commands/command_context.h"
#include "log_utils.h"

#include <string>

void set_game_actions_logger(LogFn fn);

bool resolve_game_addrs(GameAddrs *addrs);
bool add_runes(GameAddrs *addrs, int amount, std::string &error);
bool add_item_impl(GameAddrs *addrs, int item_id, int quantity,
                   std::string &error);
bool toggle_god_mode(GameAddrs *addrs, bool &enabled, std::string &error);
bool toggle_no_clip(GameAddrs *addrs, bool &enabled, std::string &error);
