#pragma once

#include <cstddef>
#include <cstdint>

bool get_module_info(const char *name, uintptr_t &base, size_t &size);
uintptr_t pattern_scan(uintptr_t base, size_t size,
                       const unsigned char *pattern, const char *mask);
uintptr_t pattern_scan_exact(uintptr_t base, size_t size,
                             const unsigned char *pattern, size_t pattern_len);
uintptr_t resolve_relative(uintptr_t addr, int offset, int addend);

bool safe_read_ptr(uintptr_t addr, uintptr_t &out);
bool safe_read_u8(uintptr_t addr, uint8_t &out);
bool safe_read_u32(uintptr_t addr, uint32_t &out);
bool safe_write_u8(uintptr_t addr, uint8_t value);
bool safe_write_u32(uintptr_t addr, uint32_t value);
