#include "game_actions.h"

#include "memory_utils.h"

namespace {

LogFn g_log_fn = nullptr;

} // namespace

void set_game_actions_logger(LogFn fn) { g_log_fn = fn; }

bool resolve_game_addrs(GameAddrs *addrs) {
  if (!addrs) {
    return false;
  }
  if (addrs->initialized) {
    return addrs->world_chr_man && addrs->add_soul_call;
  }

  uintptr_t base = 0;
  size_t size = 0;
  if (!get_module_info("eldenring.exe", base, size)) {
    if (g_log_fn) {
      g_log_fn("Failed to find eldenring.exe module");
    }
    return false;
  }

  const unsigned char world_chr_man_pat[] = {0x48, 0x8B, 0x05, 0x00, 0x00,
                                             0x00, 0x00, 0x48, 0x85, 0xC0,
                                             0x74, 0x0F, 0x48, 0x39, 0x88};
  const char world_chr_man_mask[] = "xxx????xxxxxxx";

  const unsigned char add_soul_pat[] = {0x44, 0x8B, 0x49, 0x6C,
                                        0x45, 0x33, 0xDB};
  const unsigned char chr_dbg_flags_pat[] = {0x80, 0x3D, 0x00, 0x00, 0x00, 0x00,
                                             0x00, 0x0F, 0x85, 0x00, 0x00, 0x00,
                                             0x00, 0x32, 0xC0, 0x48};
  const char chr_dbg_flags_mask[] = "xx????xxx????xxx";
  const unsigned char pause_game_pat[] = {0x80, 0xBB, 0x28, 0x01, 0x00,
                                          0x00, 0x00, 0x0F, 0x84};

  uintptr_t world_chr_man_scan =
      pattern_scan(base, size, world_chr_man_pat, world_chr_man_mask);
  if (world_chr_man_scan) {
    addrs->world_chr_man = resolve_relative(world_chr_man_scan, 3, 7);
  }

  uintptr_t add_soul_scan =
      pattern_scan_exact(base, size, add_soul_pat, sizeof(add_soul_pat));
  if (add_soul_scan) {
    addrs->add_soul_call = add_soul_scan;
  }

  uintptr_t chr_dbg_flags_scan =
      pattern_scan(base, size, chr_dbg_flags_pat, chr_dbg_flags_mask);
  if (chr_dbg_flags_scan) {
    addrs->chr_dbg_flags = resolve_relative(chr_dbg_flags_scan, 2, 7);
  }

  uintptr_t pause_game_scan =
      pattern_scan_exact(base, size, pause_game_pat, sizeof(pause_game_pat));
  if (pause_game_scan) {
    addrs->pause_game_flag = pause_game_scan + 6;
  }

  addrs->map_item_man_ptr = base + 0x3d67a50;
  addrs->item_give_func = base + 0x560670;

  addrs->initialized = true;
  (void)addrs;

  return addrs->world_chr_man && addrs->add_soul_call;
}

bool add_runes(GameAddrs *addrs, int amount, std::string &error) {
  if (!addrs) {
    error = "Missing game addresses.";
    return false;
  }
  if (amount <= 0) {
    error = "Amount must be positive.";
    return false;
  }

  if (!resolve_game_addrs(addrs)) {
    error = "Failed to resolve game addresses.";
    return false;
  }

  auto world_chr_man = *reinterpret_cast<uintptr_t *>(addrs->world_chr_man);
  if (!world_chr_man) {
    error = "WorldChrMan is null.";
    return false;
  }

  auto player_list = *reinterpret_cast<uintptr_t *>(world_chr_man + 0x10EF8);
  if (!player_list) {
    error = "Player list is null.";
    return false;
  }

  auto player = *reinterpret_cast<uintptr_t *>(player_list);
  if (!player) {
    error = "Player is null.";
    return false;
  }

  auto player_data = *reinterpret_cast<uintptr_t *>(player + 0x580);
  if (!player_data) {
    error = "Player data is null.";
    return false;
  }

  using AddSoulFn = void(__fastcall *)(void *ctx, int amount);
  auto add_soul = reinterpret_cast<AddSoulFn>(addrs->add_soul_call);
  add_soul(reinterpret_cast<void *>(player_data), amount);
  return true;
}

bool add_item_impl(GameAddrs *addrs, int item_id, int quantity,
                   std::string &error) {
  if (!addrs) {
    error = "Missing game addresses.";
    return false;
  }
  if (item_id <= 0 || quantity <= 0) {
    error = "Item id and quantity must be positive.";
    return false;
  }
  if (!resolve_game_addrs(addrs)) {
    error = "Failed to resolve game addresses.";
    return false;
  }
  if (!addrs->map_item_man_ptr || !addrs->item_give_func) {
    error = "Item give offsets unavailable.";
    return false;
  }

  auto map_item_man_ptr =
      reinterpret_cast<uintptr_t *>(addrs->map_item_man_ptr);
  if (!map_item_man_ptr || !*map_item_man_ptr) {
    error = "MapItemMan is null.";
    return false;
  }

  struct ItemEntry {
    int id;
    int quantity;
    int unk;
    int gem_id;
  };
  struct ItemTable {
    int count;
    ItemEntry items[10];
  };

  ItemTable table{};
  table.count = 1;
  table.items[0].id = item_id;
  table.items[0].quantity = quantity;
  table.items[0].unk = -1;
  table.items[0].gem_id = -1;

  int status[3] = {-1, 0, 0};

  using ItemGiveFn = void(__fastcall *)(void *map_item_man, void *item_table,
                                        void *status, uint64_t flags);
  auto give_fn = reinterpret_cast<ItemGiveFn>(addrs->item_give_func);
  give_fn(reinterpret_cast<void *>(*map_item_man_ptr), &table, status, 0);

  (void)status;
  return true;
}

bool toggle_god_mode(GameAddrs *addrs, bool &enabled, std::string &error) {
  if (!addrs) {
    error = "Missing game addresses.";
    return false;
  }
  if (!resolve_game_addrs(addrs)) {
    error = "Failed to resolve game addresses.";
    return false;
  }
  if (!addrs->chr_dbg_flags) {
    error = "CHR_DBG_FLAGS not found.";
    return false;
  }

  auto flags = reinterpret_cast<uint8_t *>(addrs->chr_dbg_flags);
  uint8_t value = flags[0];
  value = value ? 0 : 1;
  flags[0] = value;
  flags[4] = value;
  flags[5] = value;
  enabled = (value != 0);
  return true;
}

bool toggle_no_clip(GameAddrs *addrs, bool &enabled, std::string &error) {
  if (!addrs) {
    error = "Missing game addresses.";
    return false;
  }
  if (!resolve_game_addrs(addrs)) {
    error = "Failed to resolve game addresses.";
    return false;
  }
  if (!addrs->world_chr_man) {
    error = "WorldChrMan not found.";
    return false;
  }

  auto world_chr_man = *reinterpret_cast<uintptr_t *>(addrs->world_chr_man);
  if (!world_chr_man) {
    error = "WorldChrMan is null.";
    return false;
  }

  auto player_list = *reinterpret_cast<uintptr_t *>(world_chr_man + 0x10EF8);
  if (!player_list) {
    error = "Player list is null.";
    return false;
  }

  auto player = *reinterpret_cast<uintptr_t *>(player_list);
  if (!player) {
    error = "Player is null.";
    return false;
  }

  bool map_collision_flag = false;
  {
    auto ptr1 = *reinterpret_cast<uintptr_t *>(world_chr_man + 0x1E508);
    if (!ptr1) {
      error = "Collision flags pointer is null.";
      return false;
    }
    auto ptr2 = *reinterpret_cast<uintptr_t *>(ptr1 + 0x58);
    if (!ptr2) {
      error = "Collision flags buffer is null.";
      return false;
    }
    auto flags = reinterpret_cast<uint8_t *>(ptr2 + 0xF0);
    map_collision_flag = ((*flags) & (1 << 3)) != 0;
  }

  bool gravity_flag = false;
  {
    auto ptr1 = *reinterpret_cast<uintptr_t *>(player + 0x190);
    if (!ptr1) {
      error = "Gravity ptr1 is null.";
      return false;
    }
    auto ptr2 = *reinterpret_cast<uintptr_t *>(ptr1 + 0x68);
    if (!ptr2) {
      error = "Gravity ptr2 is null.";
      return false;
    }
    auto flag = reinterpret_cast<uint8_t *>(ptr2 + 0x1D3);
    gravity_flag = (*flag) != 0;
  }

  uintptr_t horse = 0;
  {
    uintptr_t module_container = 0;
    if (safe_read_ptr(player + 0x190, module_container) && module_container) {
      uintptr_t ride_module = 0;
      if (safe_read_ptr(module_container + 0xE8, ride_module) && ride_module) {
        uintptr_t last_mounted = 0;
        safe_read_ptr(ride_module + 0x18, last_mounted);
        horse = last_mounted;
      }
    }
  }

  bool horse_collision_flag = false;
  bool horse_gravity_flag = false;
  if (horse) {
    uintptr_t horse_ctrl = 0;
    if (safe_read_ptr(horse + 0x58, horse_ctrl) && horse_ctrl) {
      uint32_t flags = 0;
      if (safe_read_u32(horse_ctrl + 0xF0, flags)) {
        horse_collision_flag =
            ((flags & (1 << 2)) != 0) || ((flags & (1 << 3)) != 0);
      }
    }

    uintptr_t horse_gravity_ptr1 = 0;
    uintptr_t ptr2 = 0;
    if (safe_read_ptr(horse + 0x190, horse_gravity_ptr1) &&
        horse_gravity_ptr1) {
      safe_read_ptr(horse_gravity_ptr1 + 0x68, ptr2);
    }
    if (ptr2) {
      uint8_t flag = 0;
      if (safe_read_u8(ptr2 + 0x1D3, flag)) {
        horse_gravity_flag = (flag != 0);
      }
    }
  }

  bool new_value = !(map_collision_flag || gravity_flag ||
                     horse_collision_flag || horse_gravity_flag);

  {
    auto ptr1 = *reinterpret_cast<uintptr_t *>(world_chr_man + 0x1E508);
    auto ptr2 = ptr1 ? *reinterpret_cast<uintptr_t *>(ptr1 + 0x58) : 0;
    if (!ptr2) {
      error = "Collision flags buffer is null.";
      return false;
    }
    auto flags = reinterpret_cast<uint8_t *>(ptr2 + 0xF0);
    if (new_value) {
      *flags |= (1 << 3);
    } else {
      *flags &= static_cast<uint8_t>(~(1 << 3));
    }
  }

  {
    auto ptr1 = *reinterpret_cast<uintptr_t *>(player + 0x190);
    auto ptr2 = ptr1 ? *reinterpret_cast<uintptr_t *>(ptr1 + 0x68) : 0;
    if (!ptr2) {
      error = "Gravity buffer is null.";
      return false;
    }
    auto flag = reinterpret_cast<uint8_t *>(ptr2 + 0x1D3);
    *flag = new_value ? 1 : 0;
  }

  if (horse) {
    uintptr_t horse_ctrl = 0;
    if (safe_read_ptr(horse + 0x58, horse_ctrl) && horse_ctrl) {
      uint32_t flags = 0;
      if (safe_read_u32(horse_ctrl + 0xF0, flags)) {
        uint32_t new_flags = flags;
        if (new_value) {
          new_flags |= (1 << 2);
          new_flags |= (1 << 3);
        } else {
          new_flags &= ~(1 << 2);
          new_flags &= ~(1 << 3);
        }
        safe_write_u32(horse_ctrl + 0xF0, new_flags);
      }
    }

    uintptr_t horse_gravity_ptr1 = 0;
    uintptr_t ptr2 = 0;
    if (safe_read_ptr(horse + 0x190, horse_gravity_ptr1) &&
        horse_gravity_ptr1) {
      safe_read_ptr(horse_gravity_ptr1 + 0x68, ptr2);
    }
    if (ptr2) {
      safe_write_u8(ptr2 + 0x1D3, new_value ? 1 : 0);
    }
  }

  enabled = new_value;
  return true;
}

bool set_game_paused(GameAddrs *addrs, bool paused, std::string &error) {
  if (!addrs) {
    error = "Missing game addresses.";
    return false;
  }
  if (!resolve_game_addrs(addrs)) {
    error = "Failed to resolve game addresses.";
    return false;
  }
  if (!addrs->pause_game_flag) {
    error = "Pause flag not found.";
    return false;
  }

  if (!safe_write_u8(addrs->pause_game_flag, paused ? 1 : 0)) {
    error = "Failed to write pause flag.";
    return false;
  }
  return true;
}
