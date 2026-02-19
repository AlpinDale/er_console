#include "search.h"

#include "command_context.h"
#include "command_registry.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

#include "../resources.h"

#undef min
#undef max

namespace {

struct IdEntry {
  int id = 0;
  std::string name;
  std::string category;
};

struct SearchState {
  bool loaded = false;
  std::vector<IdEntry> entries;
};

SearchState g_state;

std::string to_lower(std::string s) {
  for (char &c : s) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return s;
}

std::string normalize(const std::string &s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '-') {
      out.push_back(
          static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
  }
  return out;
}

std::string trim(std::string s) {
  size_t start = 0;
  while (start < s.size() &&
         std::isspace(static_cast<unsigned char>(s[start]))) {
    ++start;
  }
  size_t end = s.size();
  while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
    --end;
  }
  return s.substr(start, end - start);
}

int levenshtein(const std::string &a, const std::string &b) {
  const size_t n = a.size();
  const size_t m = b.size();
  if (n == 0)
    return static_cast<int>(m);
  if (m == 0)
    return static_cast<int>(n);

  std::vector<int> prev(m + 1), cur(m + 1);
  for (size_t j = 0; j <= m; ++j) {
    prev[j] = static_cast<int>(j);
  }
  for (size_t i = 1; i <= n; ++i) {
    cur[0] = static_cast<int>(i);
    for (size_t j = 1; j <= m; ++j) {
      int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
      int a1 = prev[j] + 1;
      int b1 = cur[j - 1] + 1;
      int c1 = prev[j - 1] + cost;
      cur[j] = (std::min)(a1, (std::min)(b1, c1));
    }
    std::swap(prev, cur);
  }
  return prev[m];
}

std::vector<std::string> parse_csv_line(const std::string &line) {
  std::vector<std::string> cols;
  std::string cur;
  bool in_quotes = false;

  for (size_t i = 0; i < line.size(); ++i) {
    char c = line[i];
    if (c == '"') {
      if (in_quotes && i + 1 < line.size() && line[i + 1] == '"') {
        cur.push_back('"');
        ++i;
      } else {
        in_quotes = !in_quotes;
      }
      continue;
    }
    if (c == ',' && !in_quotes) {
      cols.push_back(cur);
      cur.clear();
      continue;
    }
    cur.push_back(c);
  }
  cols.push_back(cur);
  return cols;
}

int find_name_column(const std::vector<std::string> &header) {
  for (size_t i = 0; i < header.size(); ++i) {
    auto col = to_lower(trim(header[i]));
    if (col == "name" || col == "english" || col == "npc") {
      return static_cast<int>(i);
    }
  }
  return -1;
}

bool load_resource_csv(int resource_id, const std::string &category) {
  HMODULE module = GetModuleHandleA("er_console_mod.dll");
  if (!module) {
    module = GetModuleHandleA(nullptr);
  }
  if (!module)
    return false;

  HRSRC res = FindResourceA(module, MAKEINTRESOURCEA(resource_id), RT_RCDATA);
  if (!res)
    return false;

  HGLOBAL data = LoadResource(module, res);
  if (!data)
    return false;

  DWORD size = SizeofResource(module, res);
  if (size == 0)
    return false;

  void *ptr = LockResource(data);
  if (!ptr)
    return false;

  std::string content(static_cast<const char *>(ptr), size);
  std::istringstream stream(content);

  std::string header_line;
  if (!std::getline(stream, header_line))
    return false;
  auto header = parse_csv_line(header_line);
  int name_col = find_name_column(header);
  if (name_col < 0)
    return false;

  std::string line;
  while (std::getline(stream, line)) {
    if (line.empty())
      continue;
    auto cols = parse_csv_line(line);
    if (cols.size() <= static_cast<size_t>(name_col))
      continue;

    int id = 0;
    try {
      id = std::stoi(trim(cols[0]));
    } catch (...) {
      continue;
    }

    std::string name = trim(cols[name_col]);
    if (name.empty())
      continue;
    g_state.entries.push_back({id, name, category});
  }

  return true;
}

void ensure_loaded() {
  if (g_state.loaded)
    return;
  g_state.loaded = true;

  load_resource_csv(IDR_CSV_WEAPONS, "weapon");
  load_resource_csv(IDR_CSV_DLC_WEAPONS, "weapon");
  load_resource_csv(IDR_CSV_ARMOR, "armor");
  load_resource_csv(IDR_CSV_DLC_ARMOR, "armor");
  load_resource_csv(IDR_CSV_TALISMANS, "talisman");
  load_resource_csv(IDR_CSV_DLC_TALISMANS, "talisman");
  load_resource_csv(IDR_CSV_GOODS, "goods");
  load_resource_csv(IDR_CSV_DLC_ITEMS, "goods");
  load_resource_csv(IDR_CSV_NPCS, "npc");
  load_resource_csv(IDR_CSV_DLC_NPCS, "npc");
}

std::string extract_quoted(const std::string &input) {
  size_t first = input.find('"');
  if (first == std::string::npos)
    return std::string();
  size_t last = input.find('"', first + 1);
  if (last == std::string::npos)
    return std::string();
  return input.substr(first + 1, last - first - 1);
}

} // namespace

std::string handle_search_command(CommandContext &, const std::string &input) {
  ensure_loaded();
  if (g_state.entries.empty()) {
    return "No ID data found (missing src\\id CSVs?).";
  }

  std::string query = extract_quoted(input);
  if (query.empty()) {
    return "Usage: search \"name\" [type]";
  }

  std::string filter = "all";
  {
    size_t last_quote = input.find('"', input.find('"') + 1);
    if (last_quote != std::string::npos) {
      std::string tail = input.substr(last_quote + 1);
      std::istringstream tail_stream(tail);
      std::string token;
      if (tail_stream >> token) {
        filter = to_lower(token);
      }
    }
  }

  if (filter != "all" && filter != "weapon" && filter != "armor" &&
      filter != "talisman" && filter != "goods" && filter != "npc") {
    return "Usage: search \"name\" [type]";
  }

  std::string qnorm = normalize(query);
  struct Match {
    int score;
    IdEntry entry;
  };
  std::vector<Match> matches;

  for (const auto &entry : g_state.entries) {
    if (filter != "all" && entry.category != filter) {
      continue;
    }
    std::string name_norm = normalize(entry.name);
    int score = 1000;
    if (name_norm == qnorm) {
      score = 0;
    } else if (name_norm.rfind(qnorm, 0) == 0) {
      score = 5;
    } else if (name_norm.find(qnorm) != std::string::npos) {
      score = 10;
    } else {
      int dist = levenshtein(qnorm, name_norm);
      if (dist <= 3) {
        score = 20 + dist;
      } else {
        continue;
      }
    }
    matches.push_back({score, entry});
  }

  if (matches.empty()) {
    return "No matches.";
  }

  std::sort(matches.begin(), matches.end(), [](const Match &a, const Match &b) {
    if (a.score != b.score)
      return a.score < b.score;
    return a.entry.name < b.entry.name;
  });

  const size_t limit = 10;
  std::string out = "Matches:";
  for (size_t i = 0; i < matches.size() && i < limit; ++i) {
    const auto &m = matches[i].entry;
    out += "\n" + m.name + " | " + std::to_string(m.id) + " | " + m.category;
  }
  return out;
}

CommandInfo build_search_command() {
  CommandInfo info;
  info.name = "search";
  info.usage = "search \"name\" [type]";
  info.description =
      "Fuzzy search IDs by name (type: weapon, armor, talisman, goods, npc).";
  info.handler = handle_search_command;
  return info;
}
