cmake_minimum_required(VERSION 3.20)
project(er_console_cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)

include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.90.9
)

FetchContent_Declare(
  kiero
  GIT_REPOSITORY https://github.com/Rebzzel/kiero.git
  GIT_TAG master
)

FetchContent_MakeAvailable(imgui kiero)

set(KIERO_HEADER "${kiero_SOURCE_DIR}/kiero.h")
if (EXISTS "${KIERO_HEADER}")
  file(READ "${KIERO_HEADER}" KIERO_HEADER_CONTENT)
  string(REPLACE "#define KIERO_INCLUDE_D3D11  0" "#define KIERO_INCLUDE_D3D11  1" KIERO_HEADER_CONTENT "${KIERO_HEADER_CONTENT}")
  string(REPLACE "#define KIERO_INCLUDE_D3D12  0" "#define KIERO_INCLUDE_D3D12  1" KIERO_HEADER_CONTENT "${KIERO_HEADER_CONTENT}")
  string(REPLACE "#define KIERO_USE_MINHOOK    0" "#define KIERO_USE_MINHOOK    1" KIERO_HEADER_CONTENT "${KIERO_HEADER_CONTENT}")
  file(WRITE "${KIERO_HEADER}" "${KIERO_HEADER_CONTENT}")
endif()

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${imgui_SOURCE_DIR}/misc/cpp
)

add_library(kiero STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/kiero_wrapper.cpp
  ${kiero_SOURCE_DIR}/minhook/src/buffer.c
  ${kiero_SOURCE_DIR}/minhook/src/hook.c
  ${kiero_SOURCE_DIR}/minhook/src/trampoline.c
  ${kiero_SOURCE_DIR}/minhook/src/hde/hde32.c
  ${kiero_SOURCE_DIR}/minhook/src/hde/hde64.c
)

target_include_directories(kiero PUBLIC
  ${kiero_SOURCE_DIR}
  ${kiero_SOURCE_DIR}/minhook/include
)

target_compile_definitions(kiero PRIVATE
  MINHOOK_STATIC
  WIN32_LEAN_AND_MEAN
  ER_CONSOLE_BUILD
)

add_library(er_console_mod SHARED
  src/dllmain.cpp
  src/console_commands.cpp
  src/game_actions.cpp
  src/input_hooks.cpp
  src/item_queue.cpp
  src/memory_utils.cpp
  src/commands/coords.cpp
  src/commands/teleport.cpp
  src/commands/command_context.h
  src/commands/command_registry.cpp
  src/commands/command_registry.h
  src/commands/help.cpp
  src/commands/help.h
  src/commands/coords.h
  src/commands/teleport.h
  src/commands/items.cpp
  src/commands/items.h
  src/commands/search.cpp
  src/commands/search.h
  src/commands/runes.cpp
  src/commands/runes.h
  src/resources.rc
)
target_include_directories(er_console_mod PRIVATE src)
target_include_directories(er_console_mod PRIVATE src/commands)
target_include_directories(er_console_mod PRIVATE ${kiero_SOURCE_DIR}/minhook/include)
target_link_libraries(er_console_mod PRIVATE imgui kiero d3d12 d3d11 dxgi d3dcompiler)
target_compile_definitions(er_console_mod PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX ER_CONSOLE_BUILD)
set_target_properties(er_console_mod PROPERTIES OUTPUT_NAME "er_console_mod")

add_executable(er_console_launcher src/launcher.cpp)
target_compile_definitions(er_console_launcher PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
set_target_properties(er_console_launcher PROPERTIES OUTPUT_NAME "er_console_launcher")
