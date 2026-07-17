# GromEngine Roadmap

> *Building a better-than-Unreal game engine*

---

## Vision

Cross-platform, multi-threaded AAA game engine with a custom UI framework, modular render abstraction, and professional editor tools. GromEngine prioritizes performance, modularity, and developer experience from the ground up — designed from first principles with data-oriented architecture and no legacy baggage.

---

## Current Status (v0.5.0-pre)

| Layer | Status |
|---|---|
| Engine Core | Types, Assert, Memory Allocators, Containers, Math Library |
| Platform Abstraction | Win32 window, Input System, Platform Queries |
| RHI Abstraction | Abstract Device, Buffer, Texture, Shader, Pipeline Interfaces |
| D3D11 Backend | Full Device, Swap Chain, Shader Compilation, Buffers, Textures, Pipeline States |
| **D3D12 Backend** | **Device, Command Queues, Root Signatures, Descriptor Heaps, PSOs** |
| **Vulkan Backend** | **Device, Swap Chain, Pipeline, Shaders, Buffers, Textures** |
| Job System | Lock-free Multi-threaded Job Queue with Work Stealing, Parallel-For Support |
| UI Framework | Canvas, Elements, Widgets, Buttons, Panels, Text (Retained-Mode, Slate-Inspired) |
| Build System | CMake with Modular Subdirectories, Include/Source Convention, Precompiled Headers |
| Asset System | AssetManager, Texture loading, Shader permutations |
| File System | IFileSystem abstraction, Native + Package implementations |

---

## Roadmap

### Phase 1 — Foundation (v0.1.0 – v0.3.0)

#### v0.1.0 — Core Engine (Current)

- [x] Core type system and math library
- [x] Memory management (heap, linear, pool allocators)
- [x] Custom containers (dynamic array, string, static array)
- [x] Win32 platform layer
- [x] RHI abstraction interface
- [x] DirectX 11 backend
- [x] Multi-threaded job system
- [x] Custom retained-mode UI framework
- [x] CMake build system with modular architecture

#### v0.2.0 — Rendering Foundation

- [x] Material system with hot-reload
- [x] PBR shading pipeline
- [x] Deferred rendering
- [x] Shadow mapping (cascaded shadow maps)
- [x] HDR rendering with tone mapping
- [x] Post-processing stack (bloom)
- [x] Mesh import and rendering (glTF)
- [x] Scene graph and transform hierarchy

#### v0.3.0 — Asset Pipeline

- [x] Asset system with cooking and importing
- [x] Texture compression (BCn) via DirectXTex
- [x] Shader library with permutations
- [x] File system abstraction
- [x] Package/archive format

---

### Phase 2 — Platforms & APIs (v0.4.0 – v0.6.0)

#### v0.4.0 — Vulkan Backend

- [x] Vulkan device, swap chain, pipeline
- [x] SPIR-V shader compilation
- [ ] Memory management (VMA)
- [ ] Descriptor set management
- [ ] Render graph integration

#### v0.5.0 — DirectX 12 Backend

- [x] D3D12 device and command queues
- [x] Root signature and descriptor heaps
- [ ] ExecuteIndirect support
- [ ] GPU-driven rendering preparation

#### v0.6.0 — OpenGL & Cross-Platform

- [ ] OpenGL 4.6 backend (fallback)
- [ ] Linux platform (X11 / Wayland)
- [ ] macOS / iOS platform (MoltenVK)
- [ ] Android platform
- [ ] Console platform considerations

---

### Phase 3 — Editor (v0.7.0 – v0.9.0)

#### v0.7.0 — Editor Foundation

- [ ] Editor docking system
- [ ] Viewport rendering (multiple cameras)
- [ ] Property panel
- [ ] Content browser
- [ ] Scene outliner
- [ ] Toolbar and menus
- [ ] Undo/redo system

#### v0.8.0 — Level Editor

- [ ] Transform gizmos (translate / rotate / scale)
- [ ] Actor placement
- [ ] Terrain editing
- [ ] Lighting visualization
- [ ] Blueprint / visual scripting prototype

#### v0.9.0 — Animation & Physics

- [ ] Skeletal animation system
- [ ] Blend spaces and state machines
- [ ] Physics integration (PhysX / Bullet / Jolt)
- [ ] Ragdoll physics
- [ ] Audio system integration

---

### Phase 4 — Production (v1.0.0)

#### v1.0.0 — AAA Ready

- [ ] Render graph / frame graph system
- [ ] GPU particle systems
- [ ] Volumetric lighting / fog
- [ ] Global illumination (light probes, Lumen-style)
- [ ] Nanite-style virtual geometry
- [ ] Network replication layer
- [ ] Dedicated server support
- [ ] Profiling and optimization tools
- [ ] Crash reporting and analytics
- [ ] Documentation and tutorials

---

## Architecture Principles

| Principle | Description |
|---|---|
| **Data-Oriented Design** | Cache-friendly, struct-of-arrays where beneficial. No inheritance-heavy god objects. |
| **Job System Parallelism** | No main thread bottlenecks. Work is dispatched across cores with a lock-free job queue. |
| **Render API Agnostic** | Game code targets a single RHI abstraction. Backends are swappable without touching game logic. |
| **Fast Iteration** | Hot-reload for shaders, materials, and gameplay code. Sub-second iteration loops. |
| **Editor = Tool** | The editor is a standalone application that launches and manages game instances, not a plugin. |

---

## Long-Term Goals (Beyond v1.0)

- In-editor multiplayer testing
- World partition system for large open worlds
- Procedural content generation framework
- Machine learning integration for tooling and runtime
- VR / AR support
- Cloud rendering and streaming

---

## How to Contribute

This is a solo project by [Sirrsuh](https://github.com/Sirrsuh).

Check the repository for issues and project boards. Contributions, bug reports, and feature requests are welcome.

---

## Build Instructions

```bash
git clone --recursive https://github.com/Sirrsuh/GromEngine.git
cd GromEngine
cmake -S . -B Build -G "Visual Studio 17 2022" -A x64
cmake --build Build --config Debug
```

---

*GromEngine — Built from scratch, designed for the future.*
