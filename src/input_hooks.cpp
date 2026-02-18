#include "input_hooks.h"

#include <cstring>
#include <windows.h>

#if defined(ER_CONSOLE_BUILD)
#include "MinHook.h"
#endif

namespace {

bool *g_console_open = nullptr;
LogFn g_log_fn = nullptr;
bool g_input_hooks_installed = false;
bool g_input_hooks_attempted = false;

using GetAsyncKeyStateFn = SHORT(WINAPI *)(int);
using GetKeyStateFn = SHORT(WINAPI *)(int);
using GetKeyboardStateFn = BOOL(WINAPI *)(PBYTE);
using GetRawInputDataFn = UINT(WINAPI *)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
using GetRawInputBufferFn = UINT(WINAPI *)(PRAWINPUT, PUINT, UINT);

GetAsyncKeyStateFn g_get_async_key_state = nullptr;
GetKeyStateFn g_get_key_state = nullptr;
GetKeyboardStateFn g_get_keyboard_state = nullptr;
GetRawInputDataFn g_get_raw_input_data = nullptr;
GetRawInputBufferFn g_get_raw_input_buffer = nullptr;

SHORT WINAPI hk_get_async_key_state(int vkey) {
  if (g_console_open && *g_console_open) {
    return 0;
  }
  return g_get_async_key_state ? g_get_async_key_state(vkey) : 0;
}

SHORT WINAPI hk_get_key_state(int vkey) {
  if (g_console_open && *g_console_open) {
    return 0;
  }
  return g_get_key_state ? g_get_key_state(vkey) : 0;
}

BOOL WINAPI hk_get_keyboard_state(PBYTE state) {
  if (g_console_open && *g_console_open) {
    if (state) {
      std::memset(state, 0, 256);
    }
    return TRUE;
  }
  return g_get_keyboard_state ? g_get_keyboard_state(state) : FALSE;
}

UINT WINAPI hk_get_raw_input_data(HRAWINPUT hRawInput, UINT uiCommand,
                                  LPVOID pData, PUINT pcbSize,
                                  UINT cbSizeHeader) {
  if (g_console_open && *g_console_open) {
    if (pcbSize) {
      *pcbSize = 0;
    }
    return 0;
  }
  return g_get_raw_input_data
             ? g_get_raw_input_data(hRawInput, uiCommand, pData, pcbSize,
                                    cbSizeHeader)
             : 0;
}

UINT WINAPI hk_get_raw_input_buffer(PRAWINPUT pData, PUINT pcbSize,
                                    UINT cbSizeHeader) {
  if (g_console_open && *g_console_open) {
    if (pcbSize) {
      *pcbSize = 0;
    }
    return 0;
  }
  return g_get_raw_input_buffer
             ? g_get_raw_input_buffer(pData, pcbSize, cbSizeHeader)
             : 0;
}

} // namespace

void init_input_hooks(bool *console_open, LogFn log_fn) {
  g_console_open = console_open;
  g_log_fn = log_fn;
}

bool input_hooks_installed() { return g_input_hooks_installed; }

void install_input_hooks() {
#if defined(ER_CONSOLE_BUILD)
  if (g_input_hooks_installed || g_input_hooks_attempted) {
    return;
  }

  g_input_hooks_attempted = true;

  MH_STATUS init_status = MH_Initialize();
  if (init_status != MH_OK && init_status != MH_ERROR_ALREADY_INITIALIZED) {
    if (g_log_fn) {
      g_log_fn("MinHook init failed (%s)", MH_StatusToString(init_status));
    }
    return;
  }

  if (MH_CreateHook(reinterpret_cast<LPVOID>(GetAsyncKeyState),
                    reinterpret_cast<LPVOID>(hk_get_async_key_state),
                    reinterpret_cast<LPVOID *>(&g_get_async_key_state)) ==
      MH_OK) {
    MH_EnableHook(reinterpret_cast<LPVOID>(GetAsyncKeyState));
  }

  if (MH_CreateHook(reinterpret_cast<LPVOID>(GetKeyState),
                    reinterpret_cast<LPVOID>(hk_get_key_state),
                    reinterpret_cast<LPVOID *>(&g_get_key_state)) == MH_OK) {
    MH_EnableHook(reinterpret_cast<LPVOID>(GetKeyState));
  }

  if (MH_CreateHook(reinterpret_cast<LPVOID>(GetKeyboardState),
                    reinterpret_cast<LPVOID>(hk_get_keyboard_state),
                    reinterpret_cast<LPVOID *>(&g_get_keyboard_state)) ==
      MH_OK) {
    MH_EnableHook(reinterpret_cast<LPVOID>(GetKeyboardState));
  }

  if (MH_CreateHook(reinterpret_cast<LPVOID>(GetRawInputData),
                    reinterpret_cast<LPVOID>(hk_get_raw_input_data),
                    reinterpret_cast<LPVOID *>(&g_get_raw_input_data)) ==
      MH_OK) {
    MH_EnableHook(reinterpret_cast<LPVOID>(GetRawInputData));
  }

  if (MH_CreateHook(reinterpret_cast<LPVOID>(GetRawInputBuffer),
                    reinterpret_cast<LPVOID>(hk_get_raw_input_buffer),
                    reinterpret_cast<LPVOID *>(&g_get_raw_input_buffer)) ==
      MH_OK) {
    MH_EnableHook(reinterpret_cast<LPVOID>(GetRawInputBuffer));
  }

  g_input_hooks_installed = true;
  if (g_log_fn) {
    g_log_fn("Input hooks installed");
  }
#else
  (void)g_console_open;
  (void)g_log_fn;
#endif
}
