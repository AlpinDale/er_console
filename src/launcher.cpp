#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

namespace {

const char *kLogPath = "er_console_launcher.log";

void log_line(const char *fmt, ...) {
  FILE *f = std::fopen(kLogPath, "a");
  if (!f) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  std::vfprintf(f, fmt, args);
  va_end(args);
  std::fputc('\n', f);
  std::fclose(f);
}

bool file_exists(const char *path) {
  DWORD attrs = GetFileAttributesA(path);
  return (attrs != INVALID_FILE_ATTRIBUTES) &&
         !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

bool is_absolute_path(const std::string &path) {
  if (path.size() >= 3 && std::isalpha(static_cast<unsigned char>(path[0])) &&
      path[1] == ':' && (path[2] == '\\' || path[2] == '/')) {
    return true;
  }
  if (path.size() >= 2 && ((path[0] == '\\' && path[1] == '\\') ||
                           (path[0] == '/' && path[1] == '/'))) {
    return true;
  }
  return false;
}

std::string get_dirname(const std::string &path) {
  size_t pos = path.find_last_of("\\/");
  if (pos == std::string::npos) {
    return std::string();
  }
  return path.substr(0, pos);
}

std::string normalize_windows_path(const std::string &in) {
  std::string out;
  out.reserve(in.size());
  for (size_t i = 0; i < in.size(); ++i) {
    char c = in[i];
    if (c == '\\' && i + 1 < in.size() && in[i + 1] == '\\') {
      out.push_back('\\');
      ++i;
      continue;
    }
    out.push_back(c);
  }
  return out;
}

std::string get_full_path(const std::string &path) {
  char buffer[MAX_PATH * 4] = {};
  DWORD len = GetFullPathNameA(path.c_str(), static_cast<DWORD>(sizeof(buffer)),
                               buffer, nullptr);
  if (len == 0 || len >= sizeof(buffer)) {
    return path;
  }
  return std::string(buffer, len);
}

bool read_file(const std::string &path, std::string &out) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return false;
  }
  f.seekg(0, std::ios::end);
  std::streamoff size = f.tellg();
  f.seekg(0, std::ios::beg);
  out.resize(static_cast<size_t>(size));
  if (size > 0) {
    f.read(&out[0], size);
  }
  return true;
}

bool get_registry_string(HKEY root, const char *subkey, const char *value,
                         std::string &out) {
  char buf[4096] = {};
  DWORD type = 0;
  DWORD size = sizeof(buf);
  if (RegGetValueA(root, subkey, value, RRF_RT_REG_SZ, &type, buf, &size) !=
      ERROR_SUCCESS) {
    return false;
  }
  out.assign(buf);
  return true;
}

bool looks_like_path(const std::string &val) {
  if (val.find(":\\") != std::string::npos ||
      val.find(":/") != std::string::npos) {
    return true;
  }
  if (val.rfind("\\\\", 0) == 0 || val.rfind("//", 0) == 0) {
    return true;
  }
  return false;
}

void collect_library_paths(const std::string &vdf,
                           std::vector<std::string> &out) {
  size_t pos = 0;
  while (true) {
    size_t k1 = vdf.find('"', pos);
    if (k1 == std::string::npos)
      break;
    size_t k2 = vdf.find('"', k1 + 1);
    if (k2 == std::string::npos)
      break;
    std::string key = vdf.substr(k1 + 1, k2 - k1 - 1);
    size_t v1 = vdf.find('"', k2 + 1);
    if (v1 == std::string::npos)
      break;
    size_t v2 = vdf.find('"', v1 + 1);
    if (v2 == std::string::npos)
      break;
    std::string val = vdf.substr(v1 + 1, v2 - v1 - 1);
    pos = v2 + 1;

    if (key == "path" ||
        (std::all_of(key.begin(), key.end(),
                     [](unsigned char c) { return std::isdigit(c); }) &&
         looks_like_path(val))) {
      std::string norm = normalize_windows_path(val);
      if (!norm.empty()) {
        out.push_back(norm);
      }
    }
  }
}

bool find_elden_ring_via_steam(std::string &out_exe) {
  std::string steam_path;
  if (!get_registry_string(HKEY_CURRENT_USER, "Software\\Valve\\Steam",
                           "SteamPath", steam_path)) {
    get_registry_string(HKEY_LOCAL_MACHINE,
                        "SOFTWARE\\WOW6432Node\\Valve\\Steam", "InstallPath",
                        steam_path);
  }
  if (steam_path.empty()) {
    return false;
  }

  steam_path = normalize_windows_path(steam_path);
  std::vector<std::string> libraries;
  libraries.push_back(steam_path);

  std::string vdf;
  std::string vdf_path = steam_path + "\\steamapps\\libraryfolders.vdf";
  if (read_file(vdf_path, vdf)) {
    collect_library_paths(vdf, libraries);
  }

  const char *rel = "\\steamapps\\common\\ELDEN RING\\Game\\eldenring.exe";
  for (const auto &lib : libraries) {
    std::string candidate = lib + rel;
    if (file_exists(candidate.c_str())) {
      out_exe = candidate;
      return true;
    }
  }
  return false;
}

std::string format_win_error(DWORD err) {
  char *buffer = nullptr;
  DWORD size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&buffer), 0, nullptr);
  std::string result;
  if (size && buffer) {
    result.assign(buffer, size);
    LocalFree(buffer);
  }
  while (!result.empty() && (result.back() == '\r' || result.back() == '\n')) {
    result.pop_back();
  }
  return result;
}

void print_last_error(const char *msg) {
  DWORD err = GetLastError();
  std::string detail = format_win_error(err);
  if (!detail.empty()) {
    std::fprintf(stderr, "%s (err=%lu: %s)\n", msg,
                 static_cast<unsigned long>(err), detail.c_str());
    log_line("%s (err=%lu: %s)", msg, static_cast<unsigned long>(err),
             detail.c_str());
  } else {
    std::fprintf(stderr, "%s (err=%lu)\n", msg,
                 static_cast<unsigned long>(err));
    log_line("%s (err=%lu)", msg, static_cast<unsigned long>(err));
  }
}

bool inject_dll(HANDLE process, const char *dll_path) {
  log_line("Injecting DLL: %s", dll_path);
  const size_t len = std::strlen(dll_path) + 1;
  void *remote = VirtualAllocEx(process, nullptr, len, MEM_COMMIT | MEM_RESERVE,
                                PAGE_READWRITE);
  if (!remote) {
    print_last_error("Out of memory");
    return false;
  }

  SIZE_T written = 0;
  if (!WriteProcessMemory(process, remote, dll_path, len, &written) ||
      written != len) {
    print_last_error("Unable to write DLL path");
    VirtualFreeEx(process, remote, 0, MEM_RELEASE);
    return false;
  }

  HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
  if (!kernel32) {
    print_last_error("Unable to get kernel32 handle");
    VirtualFreeEx(process, remote, 0, MEM_RELEASE);
    return false;
  }

  auto load_library = reinterpret_cast<LPTHREAD_START_ROUTINE>(
      GetProcAddress(kernel32, "LoadLibraryA"));
  if (!load_library) {
    print_last_error("Unable to resolve LoadLibraryA");
    VirtualFreeEx(process, remote, 0, MEM_RELEASE);
    return false;
  }

  HANDLE thread =
      CreateRemoteThread(process, nullptr, 0, load_library, remote, 0, nullptr);
  if (!thread) {
    print_last_error("Unable to inject mod DLL");
    VirtualFreeEx(process, remote, 0, MEM_RELEASE);
    return false;
  }

  WaitForSingleObject(thread, INFINITE);
  DWORD exit_code = 0;
  if (GetExitCodeThread(thread, &exit_code)) {
    log_line("LoadLibraryA returned 0x%lx",
             static_cast<unsigned long>(exit_code));
    if (exit_code == 0) {
      print_last_error("LoadLibraryA returned NULL");
      CloseHandle(thread);
      VirtualFreeEx(process, remote, 0, MEM_RELEASE);
      return false;
    }
  } else {
    print_last_error("Unable to query injection thread result");
  }
  CloseHandle(thread);
  VirtualFreeEx(process, remote, 0, MEM_RELEASE);
  return true;
}

} // namespace

int main(int argc, char **argv) {
  log_line("=== Launcher start ===");
  std::string exe_path;
  if (argc > 1) {
    exe_path = argv[1];
    if (!file_exists(exe_path.c_str())) {
      std::fprintf(stderr, "Failed to find \"%s\"\n", exe_path.c_str());
      log_line("Failed to find exe: %s", exe_path.c_str());
      system("PAUSE");
      return 1;
    }
  } else {
    if (!find_elden_ring_via_steam(exe_path)) {
      std::fprintf(stderr, "Failed to find \"eldenring.exe\" (pass full path "
                           "or run from the game folder)\n");
      log_line("Failed to locate eldenring.exe via Steam");
      system("PAUSE");
      return 1;
    }
  }

  std::string dll_path =
      (argc > 2) ? argv[2] : "er_console_mod.dll";
  if (!file_exists(dll_path.c_str())) {
    if (!is_absolute_path(dll_path)) {
      std::string exe_dir = get_dirname(exe_path);
      if (!exe_dir.empty()) {
        std::string alt = exe_dir + "\\" + dll_path;
        if (file_exists(alt.c_str())) {
          dll_path = alt;
        }
      }
    }
  }

  if (!file_exists(dll_path.c_str())) {
    std::fprintf(stderr, "Failed to find \"%s\"\n", dll_path.c_str());
    log_line("Failed to find dll: %s", dll_path.c_str());
    system("PAUSE");
    return 1;
  }

  exe_path = get_full_path(exe_path);
  dll_path = get_full_path(dll_path);

  log_line("Using exe: %s", exe_path.c_str());
  log_line("Using dll: %s", dll_path.c_str());

  if (!SetEnvironmentVariableA("SteamAppId", "1245620")) {
    print_last_error("Failed to set SteamAppId");
    system("PAUSE");
    return 1;
  }

  std::string exe_dir = get_dirname(exe_path);
  STARTUPINFOA si{};
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi{};

  if (!CreateProcessA(exe_path.c_str(), nullptr, nullptr, nullptr, FALSE,
                      CREATE_SUSPENDED, nullptr,
                      exe_dir.empty() ? nullptr : exe_dir.c_str(), &si, &pi)) {
    print_last_error("Failed to launch \"eldenring.exe\"");
    system("PAUSE");
    return 1;
  }

  if (!inject_dll(pi.hProcess, dll_path.c_str())) {
    TerminateProcess(pi.hProcess, 1);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    system("PAUSE");
    return 1;
  }

  ResumeThread(pi.hThread);
  std::printf("Elden Ring is running...\n");
  log_line("Elden Ring is running...");
  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  log_line("=== Launcher end ===");
  return 0;
}
