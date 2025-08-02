# TF2 SecretiveRendering - Visual Studio 2022 Build

## Quick Start

1. **Initialize submodules (IMPORTANT):**
   ```bash
   git submodule update --init --recursive
   ```

2. **Open the solution:**
   - Double-click `TF2SecretiveRendering.sln`
   - Or open in Visual Studio 2022

3. **Select configuration:**
   - Set to **Release** and **x64**
   - Build → Build Solution (F7)

4. **Output:**
   - `bin\x64\Release\TF2SecretiveRendering.dll`

## Requirements

- **Visual Studio 2022** (Community/Professional/Enterprise)
- **Windows SDK 10.0** (included with VS2022)
- **DirectX SDK** (optional, for d3dx9.lib)

## DirectX SDK Setup (Optional)

If you have DirectX SDK installed:
- Set environment variable: `DXSDK_DIR=C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)`
- Or it will be auto-detected if installed in default location

If you don't have DirectX SDK:
- Remove `d3dx9.lib` from project dependencies
- The core DirectX 9 functionality will still work

## Build Configurations

### Debug x64
- Output: `bin\x64\Debug\TF2SecretiveRendering_d.dll`
- Includes debug symbols and console logging

### Release x64  
- Output: `bin\x64\Release\TF2SecretiveRendering.dll`
- Optimized for production use

## Troubleshooting

**"Cannot open include file 'd3dx9.h'":**
- Install DirectX SDK or remove d3dx9 dependencies

**"LNK2019: unresolved external symbol":**
- Check that all libraries are linked properly
- Ensure x64 platform is selected

**"Project out of date":**
- Clean Solution → Rebuild Solution

## Project Structure

```
TF2SecretiveRendering/
├── TF2SecretiveRendering.sln     # Solution file
├── TF2SecretiveRendering.vcxproj # Project file
├── .gitmodules                   # Git submodules configuration
├── SecretiveRendering/           # Main source code
├── imgui/                        # ImGui library (git submodule)
├── minhook/                      # MinHook library (git submodule)
└── bin/x64/Release/              # Output directory
```

## Git Submodules

This project uses git submodules for dependencies:

- **ImGui**: https://github.com/ocornut/imgui.git
- **MinHook**: https://github.com/TsudaKageyu/minhook.git

**IMPORTANT**: Always run `git submodule update --init --recursive` after cloning or the build will fail!

## Usage

1. Launch Team Fortress 2 through Steam (with overlay enabled)
2. Inject `TF2SecretiveRendering.dll` into tf2.exe process
3. Press F1 to toggle overlay, DELETE to exit