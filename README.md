<p align="center">
  <div><a href = "https://dirkwhoffmann.github.io/vAmiga"><img src="https://dirkwhoffmann.github.io/vAmiga/images/redirect4.png"></a></div>
  <div><a href = "https://vamiganet.github.io"><img src="https://dirkwhoffmann.github.io/vAmiga/images/redirect5.png"></a></div>
</p>

## Linux / ImGui Port

This repository includes an experimental **Linux port** of vAmiga, utilizing **SDL2** and **Dear ImGui** to provide a cross-platform graphical user interface.

While the core emulation remains identical to the macOS version, the frontend has been rewritten to support Linux desktop environments. Key features of this port include:

-   **Native Linux Support:** Builds and runs on modern Linux distributions (Ubuntu, Fedora, Arch, etc.).
-   **ImGui Interface:** A completely new, lightweight, and functional UI for configuring the emulator, managing media, and inspecting the system state.
-   **Hardware Acceleration:** Uses OpenGL 3.0+ for rendering.
-   **Input Support:** Robust keyboard and mouse integration, including mouse capture/release modes and full support for **physical gamepads** (via SDL2).
-   **Tools:** Includes ported versions of the Inspector, Dashboard, and Console for debugging and monitoring.

### Building for Linux

Requirements: `cmake`, `gcc` (or `clang`), `libsdl2-dev`.

```bash
# 1. Clone the repository
git clone https://github.com/dirkwhoffmann/vAmiga.git
cd vAmiga

# 2. Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build build -j$(nproc)

# 4. Run
./build/LinuxGUI/vAmigaLinux
```