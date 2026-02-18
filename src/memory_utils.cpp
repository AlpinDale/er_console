#include "memory_utils.h"

#include <cstring>

#include <windows.h>

bool get_module_info(const char *name, uintptr_t &base, size_t &size) {
  HMODULE mod = GetModuleHandleA(name);
  if (!mod) {
    return false;
  }
  base = reinterpret_cast<uintptr_t>(mod);
  auto *dos = reinterpret_cast<IMAGE_DOS_HEADER *>(base);
  auto *nt = reinterpret_cast<IMAGE_NT_HEADERS *>(base + dos->e_lfanew);
  size = static_cast<size_t>(nt->OptionalHeader.SizeOfImage);
  return true;
}

uintptr_t pattern_scan(uintptr_t base, size_t size,
                       const unsigned char *pattern, const char *mask) {
  size_t pattern_len = std::strlen(mask);
  for (size_t i = 0; i + pattern_len <= size; ++i) {
    bool match = true;
    for (size_t j = 0; j < pattern_len; ++j) {
      if (mask[j] == 'x' && pattern[j] != *(unsigned char *)(base + i + j)) {
        match = false;
        break;
      }
    }
    if (match) {
      return base + i;
    }
  }
  return 0;
}

uintptr_t pattern_scan_exact(uintptr_t base, size_t size,
                             const unsigned char *pattern, size_t pattern_len) {
  for (size_t i = 0; i + pattern_len <= size; ++i) {
    if (std::memcmp(reinterpret_cast<void *>(base + i), pattern, pattern_len) ==
        0) {
      return base + i;
    }
  }
  return 0;
}

uintptr_t resolve_relative(uintptr_t addr, int offset, int addend) {
  int32_t rel = *reinterpret_cast<int32_t *>(addr + offset);
  return addr + rel + addend;
}

bool safe_read_ptr(uintptr_t addr, uintptr_t &out) {
  __try {
    out = *reinterpret_cast<uintptr_t *>(addr);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    out = 0;
    return false;
  }
}

bool safe_read_u8(uintptr_t addr, uint8_t &out) {
  __try {
    out = *reinterpret_cast<uint8_t *>(addr);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    out = 0;
    return false;
  }
}

bool safe_read_u32(uintptr_t addr, uint32_t &out) {
  __try {
    out = *reinterpret_cast<uint32_t *>(addr);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    out = 0;
    return false;
  }
}

bool safe_write_u8(uintptr_t addr, uint8_t value) {
  __try {
    *reinterpret_cast<uint8_t *>(addr) = value;
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

bool safe_write_u32(uintptr_t addr, uint32_t value) {
  __try {
    *reinterpret_cast<uint32_t *>(addr) = value;
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}
