#pragma once

#include "commands/command_context.h"
#include "log_utils.h"

#include <string>

bool queue_item(GameAddrs *addrs, int item_id, int quantity,
                std::string &error);
void process_item_queue(GameAddrs *addrs, LogFn log_fn);
