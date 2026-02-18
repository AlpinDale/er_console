#pragma once

#include <string>

struct CommandContext;

using CommandHandler = std::string (*)(CommandContext &ctx, const std::string &input);

struct CommandRegistry {
  CommandHandler runes = nullptr;
};

std::string dispatch_command(CommandRegistry &registry, CommandContext &ctx,
                             const std::string &input);
