#include <cstdlib>

#if defined(ER_CONSOLE_BUILD)
#include "kiero.cpp"
#else
int kiero_wrapper_placeholder() { return 0; }
#endif
