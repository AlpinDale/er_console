#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "commands/command_registry.h"
#include "console_commands.h"
#include "game_actions.h"
#include "input_hooks.h"
#include "item_queue.h"

#if defined(ER_CONSOLE_BUILD)
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "imgui_stdlib.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
#else
struct ImVec2 {
  float x;
  float y;
  ImVec2(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y) {}
};
struct ImVec4 {
  float x;
  float y;
  float z;
  float w;
  ImVec4(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f)
      : x(_x), y(_y), z(_z), w(_w) {}
};
using ImU32 = unsigned int;
struct ImDrawList {
  void AddRectFilledMultiColor(ImVec2, ImVec2, ImU32, ImU32, ImU32, ImU32) {}
  void AddRectFilled(ImVec2, ImVec2, ImU32) {}
};
enum ImGuiStyleVar_ {
  ImGuiStyleVar_WindowRounding = 0,
  ImGuiStyleVar_WindowBorderSize = 1,
  ImGuiStyleVar_WindowPadding = 2,
  ImGuiStyleVar_FrameRounding = 3,
};
enum ImGuiCol_ {
  ImGuiCol_FrameBg = 0,
  ImGuiCol_FrameBgHovered = 1,
  ImGuiCol_FrameBgActive = 2,
  ImGuiCol_Border = 3,
  ImGuiCol_NavHighlight = 4,
  ImGuiCol_Text = 5
};
using ImGuiWindowFlags = int;
enum {
  ImGuiWindowFlags_NoTitleBar = 1 << 0,
  ImGuiWindowFlags_NoResize = 1 << 1,
  ImGuiWindowFlags_NoMove = 1 << 2,
  ImGuiWindowFlags_NoScrollbar = 1 << 3,
  ImGuiWindowFlags_NoScrollWithMouse = 1 << 4,
  ImGuiWindowFlags_NoCollapse = 1 << 5,
  ImGuiWindowFlags_NoSavedSettings = 1 << 6,
  ImGuiWindowFlags_AlwaysVerticalScrollbar = 1 << 7,
};
enum { ImGuiCond_FirstUseEver = 0, ImGuiCond_Always = 1 };
enum ImGuiKey {
  ImGuiKey_GraveAccent = 0,
  ImGuiKey_UpArrow = 1,
  ImGuiKey_DownArrow = 2
};
enum ImGuiConfigFlags_ { ImGuiConfigFlags_NavEnableKeyboard = 1 << 0 };
struct ImGuiIO {
  int ConfigFlags = 0;
  ImVec2 DisplaySize;
  struct ImFontAtlas *Fonts = nullptr;
  float MouseWheel = 0.0f;
};
struct ImFontAtlas {
  void *AddFontFromFileTTF(const char *, float) { return nullptr; }
};
namespace ImGui {
inline void CreateContext() {}
inline void DestroyContext() {}
inline ImGuiIO &GetIO() {
  static ImGuiIO io;
  static ImFontAtlas atlas;
  if (!io.Fonts) {
    io.Fonts = &atlas;
  }
  return io;
}
inline void StyleColorsDark() {}
inline bool IsKeyPressed(ImGuiKey) { return false; }
inline void SetNextWindowSize(ImVec2, int) {}
inline void SetNextWindowPos(ImVec2, int) {}
inline void SetNextWindowBgAlpha(float) {}
inline bool Begin(const char *, bool *, int) { return true; }
inline void End() {}
inline bool BeginChild(const char *, ImVec2, bool, int = 0) { return true; }
inline void EndChild() {}
inline void TextUnformatted(const char *) {}
inline void Separator() {}
inline void Text(const char *) {}
inline void SameLine(float = 0.0f, float = -1.0f) {}
inline void PushItemWidth(float) {}
inline void PopItemWidth() {}
inline void AlignTextToFramePadding() {}
inline float GetCursorPosY() { return 0.0f; }
inline void SetCursorPosY(float) {}
inline float GetScrollY() { return 0.0f; }
inline float GetScrollMaxY() { return 0.0f; }
inline void SetScrollHereY(float) {}
inline void SetScrollY(float) {}
inline float GetFrameHeightWithSpacing() { return 0.0f; }
inline bool InputText(const char *, std::string *, int) { return false; }
inline void PushStyleVar(int, float) {}
inline void PushStyleVar(int, ImVec2) {}
inline void PushStyleColor(int, ImVec4) {}
inline void PopStyleVar(int) {}
inline void PopStyleColor(int = 1) {}
inline void NewFrame() {}
inline void Render() {}
inline void *GetDrawData() { return nullptr; }
inline void SetWindowFocus() {}
inline void SetKeyboardFocusHere() {}
inline void SetWindowFontScale(float) {}
inline ImDrawList *GetWindowDrawList() { return nullptr; }
inline ImVec2 GetWindowPos() { return ImVec2(); }
inline ImVec2 GetWindowSize() { return ImVec2(); }
inline bool IsWindowHovered() { return false; }
} // namespace ImGui

inline bool ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM) {
  return false;
}
inline void ImGui_ImplWin32_Init(HWND) {}
inline void ImGui_ImplWin32_Shutdown() {}
inline void ImGui_ImplWin32_NewFrame() {}
inline void ImGui_ImplDX12_Init(ID3D12Device *, int, int,
                                ID3D12DescriptorHeap *,
                                D3D12_CPU_DESCRIPTOR_HANDLE,
                                D3D12_GPU_DESCRIPTOR_HANDLE) {}
inline void ImGui_ImplDX12_Shutdown() {}
inline void ImGui_ImplDX12_NewFrame() {}
inline void ImGui_ImplDX12_RenderDrawData(void *, ID3D12GraphicsCommandList *) {
}
inline void ImGui_ImplDX12_InvalidateDeviceObjects() {}
inline void ImGui_ImplDX12_CreateDeviceObjects() {}
inline void ImGui_ImplDX11_Init(ID3D11Device *, ID3D11DeviceContext *) {}
inline void ImGui_ImplDX11_Shutdown() {}
inline void ImGui_ImplDX11_NewFrame() {}
inline void ImGui_ImplDX11_RenderDrawData(void *) {}
#define IMGUI_CHECKVERSION()
#define ImGuiInputTextFlags_EnterReturnsTrue 0
#define ImGuiWindowFlags_NoCollapse 0
#define IM_COL32(R, G, B, A) ((ImU32)0)
#endif

#if defined(ER_CONSOLE_BUILD)
#include "kiero.h"
#else
namespace kiero {
struct Status {
  enum Enum { Success = 0 };
};
struct RenderType {
  enum Enum { None = 0, D3D11 = 1, D3D12 = 2, Auto = 3 };
};
inline Status::Enum init(RenderType::Enum) { return Status::Success; }
inline Status::Enum bind(int, void **, void *) { return Status::Success; }
inline RenderType::Enum getRenderType() { return RenderType::None; }
inline void shutdown() {}
} // namespace kiero
#endif

namespace {

struct FrameContext {
  ID3D12CommandAllocator *allocator = nullptr;
  ID3D12Resource *render_target = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle{};
  UINT64 fence_value = 0;
};

using PresentFn = HRESULT(__stdcall *)(IDXGISwapChain *, UINT, UINT);
using ResizeBuffersFn = HRESULT(__stdcall *)(IDXGISwapChain *, UINT, UINT, UINT,
                                             DXGI_FORMAT, UINT);
using ExecuteCommandListsFn = void(__stdcall *)(ID3D12CommandQueue *, UINT,
                                                ID3D12CommandList *const *);

HRESULT __stdcall hk_present_dx12(IDXGISwapChain *swapchain_base, UINT sync,
                                  UINT flags);
HRESULT __stdcall hk_present_dx11(IDXGISwapChain *swapchain, UINT sync,
                                  UINT flags);
HRESULT __stdcall hk_resize_buffers_dx12(IDXGISwapChain *swapchain_base,
                                         UINT count, UINT width, UINT height,
                                         DXGI_FORMAT format, UINT flags);
HRESULT __stdcall hk_resize_buffers_dx11(IDXGISwapChain *swapchain, UINT count,
                                         UINT width, UINT height,
                                         DXGI_FORMAT format, UINT flags);
void __stdcall hk_execute_command_lists(ID3D12CommandQueue *queue, UINT num,
                                        ID3D12CommandList *const *lists);

PresentFn g_present = nullptr;
ResizeBuffersFn g_resize_buffers = nullptr;
ExecuteCommandListsFn g_execute_command_lists = nullptr;

ID3D12Device *g_device = nullptr;
ID3D12CommandQueue *g_command_queue = nullptr;
ID3D12GraphicsCommandList *g_command_list = nullptr;
ID3D12DescriptorHeap *g_rtv_heap = nullptr;
ID3D12DescriptorHeap *g_srv_heap = nullptr;
FrameContext *g_frame_ctx = nullptr;
UINT g_frame_count = 0;
DXGI_FORMAT g_swapchain_format = DXGI_FORMAT_R8G8B8A8_UNORM;
unsigned int g_present_count = 0;

ID3D12Fence *g_fence = nullptr;
HANDLE g_fence_event = nullptr;
UINT64 g_fence_value = 0;

ID3D11Device *g_d3d11_device = nullptr;
ID3D11DeviceContext *g_d3d11_context = nullptr;
ID3D11RenderTargetView *g_d3d11_rtv = nullptr;

int g_d3d11_device_failures = 0;
bool g_tried_dx12_fallback = false;

HWND g_hwnd = nullptr;
WNDPROC g_wndproc = nullptr;
bool g_imgui_ready = false;
bool g_backend_dx12 = false;
bool g_direct_queue_captured = false;

std::mutex g_mutex;
bool g_console_open = false;
bool g_request_focus = false;
bool g_input_blocked = false;
HMODULE g_module_handle = nullptr;
bool g_scroll_to_bottom = true;
size_t g_last_log_size = 0;
std::string g_input;
std::string g_last_command;
std::vector<std::string> g_log = {
    "Elden Ring Console",
    "Type a command and press Enter",
};

char g_log_path[MAX_PATH * 4] = "er_console_mod.log";

GameAddrs g_game_addrs;
CommandRegistry g_command_registry;

void log_line(const char *fmt, ...) {
  FILE *f = std::fopen(g_log_path, "a");
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

bool is_input_message(UINT msg) {
  switch (msg) {
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_SYSCHAR:
  case WM_CHAR:
  case WM_INPUT:
  case WM_MOUSEMOVE:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
    return true;
  default:
    return false;
  }
}

LRESULT CALLBACK wndproc_hook(HWND hwnd, UINT msg, WPARAM wparam,
                              LPARAM lparam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
    return 1;
  }
  if (g_console_open && is_input_message(msg)) {
    return 1;
  }
  return CallWindowProc(g_wndproc, hwnd, msg, wparam, lparam);
}

void wait_for_frame(FrameContext &ctx) {
  if (!g_fence || g_fence_event == nullptr) {
    return;
  }
  if (ctx.fence_value == 0) {
    return;
  }
  if (g_fence->GetCompletedValue() < ctx.fence_value) {
    g_fence->SetEventOnCompletion(ctx.fence_value, g_fence_event);
    WaitForSingleObject(g_fence_event, INFINITE);
  }
}

void cleanup_render_targets() {
  if (!g_frame_ctx) {
    return;
  }
  for (UINT i = 0; i < g_frame_count; ++i) {
    if (g_frame_ctx[i].render_target) {
      g_frame_ctx[i].render_target->Release();
      g_frame_ctx[i].render_target = nullptr;
    }
  }
}

void create_render_targets(IDXGISwapChain3 *swapchain) {
  if (!g_frame_ctx || !g_rtv_heap || !g_device) {
    return;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle =
      g_rtv_heap->GetCPUDescriptorHandleForHeapStart();
  UINT rtv_size = g_device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  for (UINT i = 0; i < g_frame_count; ++i) {
    ID3D12Resource *rt = nullptr;
    if (g_frame_ctx[i].render_target) {
      g_frame_ctx[i].render_target->Release();
      g_frame_ctx[i].render_target = nullptr;
    }
    if (SUCCEEDED(swapchain->GetBuffer(i, IID_PPV_ARGS(&rt)))) {
      g_device->CreateRenderTargetView(rt, nullptr, rtv_handle);
      g_frame_ctx[i].render_target = rt;
      g_frame_ctx[i].rtv_handle = rtv_handle;
    }
    rtv_handle.ptr += rtv_size;
  }
}

void cleanup_d3d11_render_target() {
  if (g_d3d11_rtv) {
    g_d3d11_rtv->Release();
    g_d3d11_rtv = nullptr;
  }
}

bool create_d3d11_render_target(IDXGISwapChain *swapchain) {
  ID3D11Texture2D *back_buffer = nullptr;
  if (FAILED(swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer)))) {
    return false;
  }
  if (FAILED(g_d3d11_device->CreateRenderTargetView(back_buffer, nullptr,
                                                    &g_d3d11_rtv))) {
    back_buffer->Release();
    return false;
  }
  back_buffer->Release();
  return true;
}

void cleanup_imgui() {
  if (!g_imgui_ready) {
    return;
  }
  if (g_backend_dx12) {
    ImGui_ImplDX12_Shutdown();
  } else {
    ImGui_ImplDX11_Shutdown();
  }
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
  g_imgui_ready = false;
}

void cleanup_d3d() {
  cleanup_imgui();
  cleanup_render_targets();

  if (g_hwnd && g_wndproc) {
    SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)g_wndproc);
    g_wndproc = nullptr;
  }

  if (g_rtv_heap) {
    g_rtv_heap->Release();
    g_rtv_heap = nullptr;
  }
  if (g_srv_heap) {
    g_srv_heap->Release();
    g_srv_heap = nullptr;
  }
  if (g_command_list) {
    g_command_list->Release();
    g_command_list = nullptr;
  }
  if (g_fence) {
    g_fence->Release();
    g_fence = nullptr;
  }
  if (g_fence_event) {
    CloseHandle(g_fence_event);
    g_fence_event = nullptr;
  }
  if (g_frame_ctx) {
    delete[] g_frame_ctx;
    g_frame_ctx = nullptr;
  }
  g_frame_count = 0;
  g_device = nullptr;
  g_command_queue = nullptr;
  g_hwnd = nullptr;

  if (g_d3d11_rtv) {
    g_d3d11_rtv->Release();
    g_d3d11_rtv = nullptr;
  }
  if (g_d3d11_context) {
    g_d3d11_context->Release();
    g_d3d11_context = nullptr;
  }
  if (g_d3d11_device) {
    g_d3d11_device->Release();
    g_d3d11_device = nullptr;
  }
}

bool init_imgui(IDXGISwapChain3 *swapchain) {
  if (g_imgui_ready) {
    return true;
  }

  if (FAILED(
          swapchain->GetDevice(__uuidof(ID3D12Device), (void **)&g_device))) {
    log_line("GetDevice D3D12 failed");
    return false;
  }

  DXGI_SWAP_CHAIN_DESC desc{};
  swapchain->GetDesc(&desc);
  g_hwnd = desc.OutputWindow;
  g_frame_count = desc.BufferCount;
  g_swapchain_format = desc.BufferDesc.Format;

  if (!g_hwnd) {
    log_line("Swapchain HWND missing");
    return false;
  }

  if (!g_command_queue) {
    log_line("Command queue not captured yet");
    return false;
  }

  g_frame_ctx = new FrameContext[g_frame_count]();

  D3D12_DESCRIPTOR_HEAP_DESC rtv_desc{};
  rtv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtv_desc.NumDescriptors = g_frame_count;
  rtv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  if (FAILED(g_device->CreateDescriptorHeap(&rtv_desc,
                                            IID_PPV_ARGS(&g_rtv_heap)))) {
    log_line("CreateDescriptorHeap RTV failed");
    return false;
  }

  D3D12_DESCRIPTOR_HEAP_DESC srv_desc{};
  srv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  srv_desc.NumDescriptors = 1;
  srv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  if (FAILED(g_device->CreateDescriptorHeap(&srv_desc,
                                            IID_PPV_ARGS(&g_srv_heap)))) {
    log_line("CreateDescriptorHeap SRV failed");
    return false;
  }

  for (UINT i = 0; i < g_frame_count; ++i) {
    g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     IID_PPV_ARGS(&g_frame_ctx[i].allocator));
  }

  if (FAILED(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                         g_frame_ctx[0].allocator, nullptr,
                                         IID_PPV_ARGS(&g_command_list)))) {
    log_line("CreateCommandList failed");
    return false;
  }
  g_command_list->Close();

  if (FAILED(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                   IID_PPV_ARGS(&g_fence)))) {
    log_line("CreateFence failed");
    return false;
  }

  g_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

  create_render_targets(swapchain);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 18.0f);

  ImGui_ImplWin32_Init(g_hwnd);
  ImGui_ImplDX12_Init(g_device, g_frame_count, g_swapchain_format, g_srv_heap,
                      g_srv_heap->GetCPUDescriptorHandleForHeapStart(),
                      g_srv_heap->GetGPUDescriptorHandleForHeapStart());

  g_wndproc =
      (WNDPROC)SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)wndproc_hook);
  g_imgui_ready = true;
  g_backend_dx12 = true;
  log_line("ImGui initialized (DX12 format=%d buffers=%u)",
           static_cast<int>(g_swapchain_format), g_frame_count);
  return true;
}

bool init_imgui_dx11(IDXGISwapChain *swapchain) {
  if (g_imgui_ready) {
    return true;
  }

  HRESULT hr =
      swapchain->GetDevice(__uuidof(ID3D11Device), (void **)&g_d3d11_device);
  if (FAILED(hr)) {
    log_line("GetDevice D3D11 failed (hr=0x%08lx)",
             static_cast<unsigned long>(hr));
    return false;
  }
  g_d3d11_device->GetImmediateContext(&g_d3d11_context);

  DXGI_SWAP_CHAIN_DESC desc{};
  swapchain->GetDesc(&desc);
  g_hwnd = desc.OutputWindow;
  if (!g_hwnd) {
    log_line("Swapchain HWND missing (D3D11)");
    return false;
  }

  if (!create_d3d11_render_target(swapchain)) {
    log_line("CreateRenderTargetView D3D11 failed");
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 18.0f);

  ImGui_ImplWin32_Init(g_hwnd);
  ImGui_ImplDX11_Init(g_d3d11_device, g_d3d11_context);

  g_wndproc =
      (WNDPROC)SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)wndproc_hook);
  g_imgui_ready = true;
  g_backend_dx12 = false;
  log_line("ImGui initialized (D3D11)");
  return true;
}

void render_console() {
  if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) {
    g_console_open = !g_console_open;
    if (g_console_open) {
      g_request_focus = true;
    }
    if (g_console_open && !g_input_blocked) {
      BlockInput(TRUE);
      g_input_blocked = true;
    } else if (!g_console_open && g_input_blocked) {
      BlockInput(FALSE);
      g_input_blocked = false;
    }
  }

  if (!g_console_open) {
    if (g_input_blocked) {
      BlockInput(FALSE);
      g_input_blocked = false;
    }
    return;
  }

  ImGuiIO &io = ImGui::GetIO();
  const float width = io.DisplaySize.x;
  const float height = io.DisplaySize.y;
  const float console_height = height * 0.5f;
  const ImVec2 console_pos(0.0f, height - console_height);
  const ImVec2 console_size(width, console_height);

  ImGui::SetNextWindowPos(console_pos, ImGuiCond_Always);
  ImGui::SetNextWindowSize(console_size, ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.0f);

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoSavedSettings;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 12.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,
                        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_NavHighlight, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));

  ImGui::Begin("Console", &g_console_open, flags);
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  ImVec2 win_pos = ImGui::GetWindowPos();
  ImVec2 win_size = ImGui::GetWindowSize();
  const float fade_height = 6.0f;
  ImU32 fade_top = IM_COL32(0, 0, 0, 0);
  ImU32 fade_bottom = IM_COL32(0, 0, 0, 170);
  ImU32 solid = IM_COL32(0, 0, 0, 170);
  draw_list->AddRectFilled(
      ImVec2(win_pos.x, win_pos.y + fade_height),
      ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y), solid);
  draw_list->AddRectFilledMultiColor(
      win_pos, ImVec2(win_pos.x + win_size.x, win_pos.y + fade_height),
      fade_top, fade_top, fade_bottom, fade_bottom);

  if (g_request_focus) {
    ImGui::SetWindowFocus();
  }

  float log_height = -ImGui::GetFrameHeightWithSpacing();
  ImGui::BeginChild("ConsoleLog", ImVec2(0.0f, log_height), false,
                    ImGuiWindowFlags_NoScrollbar);
  if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
    ImGui::SetScrollY(ImGui::GetScrollY() - 30.0f);
    g_scroll_to_bottom = false;
  } else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
    ImGui::SetScrollY(ImGui::GetScrollY() + 30.0f);
    g_scroll_to_bottom = false;
  }
  if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
    g_scroll_to_bottom = false;
  }
  bool log_grew = g_log.size() > g_last_log_size;
  g_last_log_size = g_log.size();
  for (const auto &line : g_log) {
    ImGui::TextUnformatted(line.c_str());
  }
  float scroll_y = ImGui::GetScrollY();
  float scroll_max = ImGui::GetScrollMaxY();
  bool at_bottom = scroll_y >= (scroll_max - 5.0f);
  if (log_grew && g_scroll_to_bottom) {
    ImGui::SetScrollHereY(1.0f);
  }
  if (at_bottom) {
    g_scroll_to_bottom = true;
  }
  ImGui::EndChild();

  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(">");
  ImGui::SameLine(0.0f, 6.0f);
  ImGui::PushItemWidth(-1.0f);
  ImGui::SetKeyboardFocusHere();
  if (ImGui::InputText("##console_input", &g_input,
                       ImGuiInputTextFlags_EnterReturnsTrue)) {
    std::string command = g_input;
    if (command == "!!" || command == "repeat") {
      command = g_last_command;
    }

    if (!command.empty()) {
      g_last_command = command;
      g_log.push_back("> " + command);
      std::string response =
          handle_console_command(g_command_registry, g_game_addrs, command);
      if (!response.empty()) {
        std::istringstream lines(response);
        std::string line;
        while (std::getline(lines, line)) {
          g_log.push_back(line);
        }
      }
    }
    g_input.clear();
    g_request_focus = true;
  }
  ImGui::PopItemWidth();
  if (g_request_focus) {
    g_request_focus = false;
  }

  ImGui::End();
  ImGui::PopStyleColor(6);
  ImGui::PopStyleVar(4);
}

HRESULT __stdcall hk_present_dx12(IDXGISwapChain *swapchain_base, UINT sync,
                                  UINT flags) {
  std::lock_guard<std::mutex> lock(g_mutex);
  auto *swapchain = reinterpret_cast<IDXGISwapChain3 *>(swapchain_base);

  g_present_count++;
  (void)g_swapchain_format;

  if (!g_imgui_ready) {
    if (!init_imgui(swapchain)) {
      return g_present(swapchain_base, sync, flags);
    }
  }

  if (g_imgui_ready && g_backend_dx12) {
    if (!input_hooks_installed()) {
      install_input_hooks();
    }
    if (g_present_count < 120) {
      return g_present(swapchain_base, sync, flags);
    }
    if (!g_command_queue || !g_frame_ctx) {
      return g_present(swapchain_base, sync, flags);
    }

    process_item_queue(&g_game_addrs, log_line);
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    render_console();

    ImGui::Render();

    UINT index = swapchain->GetCurrentBackBufferIndex();
    FrameContext &ctx = g_frame_ctx[index];
    if (!ctx.render_target) {
      return g_present(swapchain_base, sync, flags);
    }

    wait_for_frame(ctx);
    ctx.allocator->Reset();
    g_command_list->Reset(ctx.allocator, nullptr);

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = ctx.render_target;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    g_command_list->ResourceBarrier(1, &barrier);

    g_command_list->OMSetRenderTargets(1, &ctx.rtv_handle, FALSE, nullptr);
    g_command_list->SetDescriptorHeaps(1, &g_srv_heap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_command_list);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_command_list->ResourceBarrier(1, &barrier);

    g_command_list->Close();
    ID3D12CommandList *lists[] = {g_command_list};
    g_command_queue->ExecuteCommandLists(1, lists);

    g_fence_value++;
    g_command_queue->Signal(g_fence, g_fence_value);
    ctx.fence_value = g_fence_value;
  }

  return g_present(swapchain_base, sync, flags);
}

HRESULT __stdcall hk_present_dx11(IDXGISwapChain *swapchain, UINT sync,
                                  UINT flags) {
  std::lock_guard<std::mutex> lock(g_mutex);

  if (!g_imgui_ready) {
    if (!init_imgui_dx11(swapchain)) {
      return g_present(swapchain, sync, flags);
    }
  }

  if (g_imgui_ready && !g_backend_dx12) {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    render_console();

    ImGui::Render();

    g_d3d11_context->OMSetRenderTargets(1, &g_d3d11_rtv, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  }

  return g_present(swapchain, sync, flags);
}

HRESULT __stdcall hk_resize_buffers_dx12(IDXGISwapChain *swapchain_base,
                                         UINT count, UINT width, UINT height,
                                         DXGI_FORMAT format, UINT flags) {
  std::lock_guard<std::mutex> lock(g_mutex);
  auto *swapchain = reinterpret_cast<IDXGISwapChain3 *>(swapchain_base);
  if (g_frame_ctx) {
    for (UINT i = 0; i < g_frame_count; ++i) {
      wait_for_frame(g_frame_ctx[i]);
    }
  }
  cleanup_render_targets();

  if (g_imgui_ready) {
    ImGui_ImplDX12_InvalidateDeviceObjects();
  }

  auto result =
      g_resize_buffers(swapchain_base, count, width, height, format, flags);

  if (g_imgui_ready) {
    ImGui_ImplDX12_CreateDeviceObjects();
  }

  if (count != 0) {
    g_frame_count = count;
    create_render_targets(swapchain);
  }

  return result;
}

HRESULT __stdcall hk_resize_buffers_dx11(IDXGISwapChain *swapchain, UINT count,
                                         UINT width, UINT height,
                                         DXGI_FORMAT format, UINT flags) {
  std::lock_guard<std::mutex> lock(g_mutex);
  cleanup_d3d11_render_target();

  auto result =
      g_resize_buffers(swapchain, count, width, height, format, flags);
  if (g_imgui_ready && !g_backend_dx12) {
    create_d3d11_render_target(swapchain);
  }
  return result;
}

void __stdcall hk_execute_command_lists(ID3D12CommandQueue *queue, UINT num,
                                        ID3D12CommandList *const *lists) {
  if (!g_direct_queue_captured) {
    D3D12_COMMAND_QUEUE_DESC desc = queue->GetDesc();
    if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
      g_command_queue = queue;
      g_direct_queue_captured = true;
      log_line("Captured direct command queue");
    }
  }
  g_execute_command_lists(queue, num, lists);
}

DWORD WINAPI init_thread(LPVOID) {
  log_line("Initializing hooks");
  Sleep(2000);

  LoadLibraryA("dxgi.dll");
  LoadLibraryA("d3d12.dll");
  LoadLibraryA("d3d11.dll");

  auto try_init = [](kiero::RenderType::Enum type, int max_attempts) -> bool {
    for (int attempts = 0; attempts < max_attempts; ++attempts) {
      auto status = kiero::init(type);
      if (status == kiero::Status::Success) {
        log_line("kiero init success (type=%d)", (int)type);
        return true;
      }
      if (attempts == 0 || attempts % 50 == 0) {
        log_line("kiero init retry %d (type=%d status=%d)", attempts, (int)type,
                 (int)status);
      }
      Sleep(100);
    }
    return false;
  };

  if (!try_init(kiero::RenderType::D3D12, 600)) {
    log_line("Failed to initialize kiero for D3D12");
    return 0;
  }

  auto present_status = kiero::bind(140, (void **)&g_present, hk_present_dx12);
  log_line("Bind Present (D3D12) status=%d", (int)present_status);
  auto resize_status =
      kiero::bind(145, (void **)&g_resize_buffers, hk_resize_buffers_dx12);
  log_line("Bind ResizeBuffers (D3D12) status=%d", (int)resize_status);
  auto exec_status = kiero::bind(54, (void **)&g_execute_command_lists,
                                 hk_execute_command_lists);
  log_line("Bind ExecuteCommandLists status=%d", (int)exec_status);

  log_line("Hooks installed (D3D12)");
  return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    g_module_handle = hModule;
    char module_path[MAX_PATH * 4] = {};
    if (GetModuleFileNameA(hModule, module_path, sizeof(module_path)) != 0) {
      std::string path(module_path);
      size_t pos = path.find_last_of("\\/");
      if (pos != std::string::npos) {
        path = path.substr(0, pos + 1) + "er_console_mod.log";
        std::snprintf(g_log_path, sizeof(g_log_path), "%s", path.c_str());
      }
    }
    set_game_actions_logger(log_line);
    init_input_hooks(&g_console_open, log_line);
    DisableThreadLibraryCalls(hModule);
    CreateThread(nullptr, 0, init_thread, nullptr, 0, nullptr);
    log_line("DllMain attach");
  } else if (reason == DLL_PROCESS_DETACH) {
    cleanup_d3d();
    log_line("DllMain detach");
  }
  return TRUE;
}
