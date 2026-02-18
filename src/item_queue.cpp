#include "item_queue.h"

#include "game_actions.h"

#include <mutex>
#include <vector>

namespace {

struct ItemTask {
  int item_id = 0;
  int quantity = 0;
};

std::mutex g_item_queue_mutex;
std::vector<ItemTask> g_item_queue;

} // namespace

bool queue_item(GameAddrs *addrs, int item_id, int quantity,
                std::string &error) {
  if (!addrs) {
    error = "Missing game addresses.";
    return false;
  }
  if (!resolve_game_addrs(addrs)) {
    error = "Failed to resolve game addresses.";
    return false;
  }

  ItemTask task;
  task.item_id = item_id;
  task.quantity = quantity;
  {
    std::lock_guard<std::mutex> lock(g_item_queue_mutex);
    g_item_queue.push_back(task);
  }
  return true;
}

void process_item_queue(GameAddrs *addrs, LogFn log_fn) {
  std::lock_guard<std::mutex> lock(g_item_queue_mutex);
  if (g_item_queue.empty()) {
    return;
  }
  ItemTask task = g_item_queue.front();
  g_item_queue.erase(g_item_queue.begin());
  std::string error;
  if (!add_item_impl(addrs, task.item_id, task.quantity, error)) {
    if (log_fn) {
      log_fn("AddItem failed: %s", error.c_str());
    }
  }
}
