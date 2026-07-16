cmake_minimum_required(VERSION 3.20)
project(GromEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Options ──────────────────────────────────────────────────────────────────
option(GROM_BUILD_EDITOR    "Build the editor application" ON)
option(GROM_BUILD_GAME      "Build the game application"   ON)
option(GROM_RHI_D3D11       "Enable DirectX 11 backend"   ON)
option(GROM_RHI_D3D12       "Enable DirectX 12 backend"   OFF)
option(GROM_RHI_VULKAN      "Enable Vulkan backend"       OFF)
option(GROM_RHI_OPENGL      "Enable OpenGL backend"       OFF)
option(GROM_PLATFORM_WIN32  "Enable Win32 platform"       ON)
option(GROM_PLATFORM_LINUX  "Enable Linux platform"       OFF)
option(GROM_USE_PCH         "Use precompiled headers"     ON)

# ── Output directories ──────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Lib")

# ── Compiler flags ───────────────────────────────────────────────────────────
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "/W4 /WX /Od /Ob0 /Gy- /Zi /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/W4 /WX /O2 /Ob2 /Gy /Gw /GL /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG:FASTLINK")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG:FULL /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG:FASTLINK")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DEBUG:FULL /LTCG")
endif()

# ── Modules ──────────────────────────────────────────────────────────────────
add_subdirectory(Engine)

if(GROM_BUILD_EDITOR)
    add_subdirectory(Editor)
endif()
if(GROM_BUILD_GAME)
    add_subdirectory(Game)
endif()

