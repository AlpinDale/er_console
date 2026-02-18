#pragma once

#include "log_utils.h"

void init_input_hooks(bool *console_open, LogFn log_fn);
void install_input_hooks();
bool input_hooks_installed();
