#pragma once

#include <string>

struct CommandRegistry;
struct GameAddrs;

std::string handle_console_command(CommandRegistry &registry, GameAddrs &addrs,
                                   const std::string &command);
