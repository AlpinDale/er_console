#include "search.h"

#include "command_context.h"
#include "command_registry.h"

#include <algorithm>
#include <cctype>
#include <cstring>
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

struct JsonReader {
  const std::string &input;
  size_t pos = 0;

  explicit JsonReader(const std::string &data) : input(data) {}

  void skip_ws() {
    while (pos < input.size() &&
           std::isspace(static_cast<unsigned char>(input[pos]))) {
      ++pos;
    }
  }

  char peek() {
    skip_ws();
    if (pos >= input.size()) {
      return '\0';
    }
    return input[pos];
  }

  bool consume(char c) {
    skip_ws();
    if (pos < input.size() && input[pos] == c) {
      ++pos;
      return true;
    }
    return false;
  }

  bool parse_string(std::string &out) {
    skip_ws();
    if (pos >= input.size() || input[pos] != '"') {
      return false;
    }
    ++pos;
    out.clear();
    while (pos < input.size()) {
      char c = input[pos++];
      if (c == '"') {
        return true;
      }
      if (c == '\\') {
        if (pos >= input.size()) {
          return false;
        }
        char esc = input[pos++];
        switch (esc) {
        case '"':
          out.push_back('"');
          break;
        case '\\':
          out.push_back('\\');
          break;
        case '/':
          out.push_back('/');
          break;
        case 'b':
          out.push_back('\b');
          break;
        case 'f':
          out.push_back('\f');
          break;
        case 'n':
          out.push_back('\n');
          break;
        case 'r':
          out.push_back('\r');
          break;
        case 't':
          out.push_back('\t');
          break;
        case 'u': {
          if (pos + 3 >= input.size()) {
            return false;
          }
          unsigned int code = 0;
          for (int i = 0; i < 4; ++i) {
            char h = input[pos++];
            code <<= 4;
            if (h >= '0' && h <= '9') {
              code |= static_cast<unsigned int>(h - '0');
            } else if (h >= 'a' && h <= 'f') {
              code |= static_cast<unsigned int>(10 + (h - 'a'));
            } else if (h >= 'A' && h <= 'F') {
              code |= static_cast<unsigned int>(10 + (h - 'A'));
            } else {
              return false;
            }
          }
          if (code <= 0x7F) {
            out.push_back(static_cast<char>(code));
          } else {
            out.push_back('?');
          }
          break;
        }
        default:
          out.push_back(esc);
          break;
        }
      } else {
        out.push_back(c);
      }
    }
    return false;
  }

  bool parse_int(int &out) {
    skip_ws();
    if (pos >= input.size()) {
      return false;
    }
    size_t start = pos;
    if (input[pos] == '-') {
      ++pos;
    }
    if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(input[pos]))) {
      pos = start;
      return false;
    }
    while (pos < input.size() &&
           std::isdigit(static_cast<unsigned char>(input[pos]))) {
      ++pos;
    }
    try {
      out = std::stoi(input.substr(start, pos - start));
    } catch (...) {
      pos = start;
      return false;
    }
    return true;
  }

  bool parse_literal(const char *lit) {
    skip_ws();
    size_t len = std::strlen(lit);
    if (pos + len > input.size()) {
      return false;
    }
    if (input.compare(pos, len, lit) == 0) {
      pos += len;
      return true;
    }
    return false;
  }

  bool skip_value() {
    skip_ws();
    char c = peek();
    if (c == '{') {
      consume('{');
      skip_ws();
      if (consume('}')) {
        return true;
      }
      while (true) {
        std::string key;
        if (!parse_string(key)) {
          return false;
        }
        if (!consume(':')) {
          return false;
        }
        if (!skip_value()) {
          return false;
        }
        if (consume(',')) {
          continue;
        }
        if (consume('}')) {
          return true;
        }
        return false;
      }
    }
    if (c == '[') {
      consume('[');
      skip_ws();
      if (consume(']')) {
        return true;
      }
      while (true) {
        if (!skip_value()) {
          return false;
        }
        if (consume(',')) {
          continue;
        }
        if (consume(']')) {
          return true;
        }
        return false;
      }
    }
    if (c == '"') {
      std::string tmp;
      return parse_string(tmp);
    }
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
      int tmp = 0;
      return parse_int(tmp);
    }
    if (parse_literal("true") || parse_literal("false") ||
        parse_literal("null")) {
      return true;
    }
    return false;
  }
};

bool parse_entries_array(JsonReader &reader, std::string &name_out) {
  if (!reader.consume('[')) {
    return false;
  }
  while (true) {
    reader.skip_ws();
    if (reader.consume(']')) {
      return true;
    }
    char c = reader.peek();
    if (c == '"') {
      std::string value;
      if (!reader.parse_string(value)) {
        return false;
      }
      if (name_out.empty() && !value.empty()) {
        name_out = value;
      }
    } else {
      if (!reader.skip_value()) {
        return false;
      }
    }
    reader.skip_ws();
    if (reader.consume(',')) {
      continue;
    }
  }
}

bool parse_entry_object(JsonReader &reader, int &id_out,
                        std::string &name_out, bool &has_id_out) {
  if (!reader.consume('{')) {
    return false;
  }
  bool has_id = false;
  while (true) {
    reader.skip_ws();
    if (reader.consume('}')) {
      break;
    }
    std::string key;
    if (!reader.parse_string(key)) {
      return false;
    }
    if (!reader.consume(':')) {
      return false;
    }
    if (key == "ID") {
      int value = 0;
      if (!reader.parse_int(value)) {
        return false;
      }
      id_out = value;
      has_id = true;
    } else if (key == "Entries") {
      std::string name;
      if (!parse_entries_array(reader, name)) {
        return false;
      }
      if (!name.empty()) {
        name_out = name;
      }
    } else {
      if (!reader.skip_value()) {
        return false;
      }
    }
    reader.skip_ws();
    if (reader.consume(',')) {
      continue;
    }
  }
  has_id_out = has_id;
  return true;
}

bool parse_entries_root(JsonReader &reader, std::vector<IdEntry> &entries,
                        const std::string &category) {
  if (!reader.consume('[')) {
    return false;
  }
  while (true) {
    reader.skip_ws();
    if (reader.consume(']')) {
      return true;
    }
    int id = 0;
    std::string name;
    bool has_id = false;
    if (!parse_entry_object(reader, id, name, has_id)) {
      return false;
    }
    if (has_id && !name.empty()) {
      entries.push_back({id, name, category});
    }
    reader.skip_ws();
    if (reader.consume(',')) {
      continue;
    }
  }
}

bool load_resource_json(int resource_id, const std::string &category) {
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
  JsonReader reader(content);
  if (!reader.consume('{')) {
    return false;
  }

  while (true) {
    reader.skip_ws();
    if (reader.consume('}')) {
      break;
    }
    std::string key;
    if (!reader.parse_string(key)) {
      return false;
    }
    if (!reader.consume(':')) {
      return false;
    }
    if (key == "Entries") {
      if (!parse_entries_root(reader, g_state.entries, category)) {
        return false;
      }
    } else {
      if (!reader.skip_value()) {
        return false;
      }
    }
    reader.skip_ws();
    if (reader.consume(',')) {
      continue;
    }
  }

  return true;
}

void ensure_loaded() {
  if (g_state.loaded)
    return;
  g_state.loaded = true;

  load_resource_json(IDR_JSON_WEAPONS, "weapon");
  load_resource_json(IDR_JSON_CUSTOM_WEAPONS, "weapon");
  load_resource_json(IDR_JSON_ARMOR, "armor");
  load_resource_json(IDR_JSON_TALISMANS, "talisman");
  load_resource_json(IDR_JSON_GOODS, "goods");
  load_resource_json(IDR_JSON_ASHES, "ash");
  load_resource_json(IDR_JSON_NPCS, "npc");
}

std::string extract_quoted(const std::string &input, size_t start) {
  size_t first = input.find('"', start);
  if (first == std::string::npos)
    return std::string();
  size_t last = input.find('"', first + 1);
  if (last == std::string::npos)
    return std::string();
  return input.substr(first + 1, last - first - 1);
}

bool parse_positive_int(const std::string &s, int &out) {
  if (s.empty()) {
    return false;
  }
  size_t i = 0;
  if (s[0] == '+') {
    i = 1;
  }
  if (i >= s.size()) {
    return false;
  }
  for (; i < s.size(); ++i) {
    if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
      return false;
    }
  }
  try {
    out = std::stoi(s);
  } catch (...) {
    return false;
  }
  return out > 0;
}

bool read_token(const std::string &input, size_t &pos, std::string &out) {
  while (pos < input.size() &&
         std::isspace(static_cast<unsigned char>(input[pos]))) {
    ++pos;
  }
  if (pos >= input.size()) {
    return false;
  }
  size_t start = pos;
  while (pos < input.size() &&
         !std::isspace(static_cast<unsigned char>(input[pos]))) {
    ++pos;
  }
  out = input.substr(start, pos - start);
  return true;
}

} // namespace

std::string handle_search_command(CommandContext &, const std::string &input) {
  ensure_loaded();
  if (g_state.entries.empty()) {
    return "No ID data found (missing src\\id JSONs?).";
  }

  size_t pos = 0;
  std::string verb;
  std::string filter;
  if (!read_token(input, pos, verb) || !read_token(input, pos, filter)) {
    return "Usage: search <type> \"name\"";
  }
  filter = to_lower(filter);

  std::string query = extract_quoted(input, pos);
  if (query.empty()) {
    return "Usage: search <type> \"name\"";
  }

  size_t last_quote = input.find('"', input.find('"', pos) + 1);
  size_t tail_pos = (last_quote == std::string::npos)
                        ? input.size()
                        : last_quote + 1;
  std::string page_token;
  int page = 1;
  if (read_token(input, tail_pos, page_token)) {
    if (!parse_positive_int(page_token, page)) {
      return "Usage: search <type> \"name\" [page]";
    }
  }

  if (filter == "ashes" || filter == "aow") {
    filter = "ash";
  }

  if (filter != "weapon" && filter != "armor" && filter != "talisman" &&
      filter != "goods" && filter != "npc" && filter != "ash") {
    return "Usage: search <type> \"name\" [page]";
  }

  std::string qnorm = normalize(query);
  struct Match {
    int score;
    IdEntry entry;
  };
  std::vector<Match> matches;

  for (const auto &entry : g_state.entries) {
    if (entry.category != filter) {
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

  const size_t page_size = 10;
  const size_t total = matches.size();
  const size_t total_pages = (total + page_size - 1) / page_size;
  if (page < 1 || static_cast<size_t>(page) > total_pages) {
    return "Page out of range.";
  }

  size_t start = (static_cast<size_t>(page) - 1) * page_size;
  size_t end = std::min(start + page_size, total);
  std::string out = "Matches (" + std::to_string(total) + " total) page " +
                    std::to_string(page) + "/" +
                    std::to_string(total_pages) + ":";
  for (size_t i = start; i < end; ++i) {
    const auto &m = matches[i].entry;
    out += "\n" + m.name + " | " + std::to_string(m.id) + " | " + m.category;
  }
  if (total > end) {
    out += "\n";
    out += "...and " + std::to_string(total - end) +
           " more. Try: search " + filter + " \"" + query + "\" " +
           std::to_string(page + 1);
  }
  return out;
}

CommandInfo build_search_command() {
  CommandInfo info;
  info.name = "search";
  info.usage = "search <type> \"name\" [page]";
  info.description =
      "Fuzzy search IDs by name (type: weapon, armor, talisman, goods, npc, "
      "ash).";
  info.handler = handle_search_command;
  return info;
}
