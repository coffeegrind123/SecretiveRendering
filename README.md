![cpp](https://img.shields.io/badge/C%2B%2B-17-%23ff40d9.svg?style=flat)
![cmake](https://img.shields.io/badge/cmake-3.16-yellow)
![License](http://img.shields.io/badge/license-MIT-yellowgreen.svg?style=flat)
![Platform](https://img.shields.io/badge/platform-x64--only-red.svg?style=flat)
![Target](https://img.shields.io/badge/target-Team%20Fortress%202-orange.svg?style=flat)

# ![icon](https://img.icons8.com/clouds/40/000000/steam.png) TF2 SecretiveRendering (x64)

A modern, 64-bit Steam overlay renderer specifically designed for **Team Fortress 2** with advanced pattern-based hooking and OBS streaming support.

Originally inspired by [@aixxe's article](https://aixxe.net/2017/09/steam-overlay-rendering), this project has been completely rewritten for 64-bit TF2 compatibility with modern Steam overlay architecture (2025).

## ⚠️ Security Notice

**This is a defensive security research tool for understanding overlay hooking techniques. Use responsibly and only for legitimate purposes.**

## 🎯 Features

- **64-bit TF2 Compatibility**: Specifically designed for Team Fortress 2 x64 architecture
- **Modern Steam Overlay Hooking**: Uses advanced pattern scanning and LEA instruction analysis  
- **Enhanced Pattern Recognition**: Robust pattern detection with fallback mechanisms
- **Architecture Validation**: Compile-time and runtime 64-bit enforcement
- **TF2 Process Detection**: Automatic validation of TF2 game process
- **Steam Overlay Verification**: Ensures `gameoverlayrenderer64.dll` is available
- **Stream-Proof Technology**: Invisible to OBS and other streaming software
- **ImGui Integration**: Modern immediate-mode GUI for overlay rendering
- **Comprehensive Error Handling**: Detailed logging and graceful failure recovery

## 🔧 Prerequisites

- **DirectX 9 SDK** installed on your system
- **Visual Studio 2019/2022** with C++17 support
- **64-bit Windows** (32-bit not supported)
- **Team Fortress 2** running through Steam with overlay enabled

## 🚀 Building

### Requirements Validation
```bash
# Ensure you have 64-bit tools
cmake --version
# Ensure DirectX SDK is installed
echo %DXSDK_DIR%
```

### Build Steps
```shell
$ git clone --recursive https://github.com/s3pt3mb3r/SecretiveRendering.git
$ cd SecretiveRendering
$ mkdir build && cd build
$ cmake .. -A x64  # IMPORTANT: x64 platform required
$ cmake --build . --config Release
```

**Note**: The project will **fail to compile** on 32-bit platforms by design.

## 🎮 Usage

### Steam Overlay Setup
1. Enable Steam overlay in Steam settings
2. Enable Steam overlay specifically for Team Fortress 2
3. Launch TF2 through Steam (overlay must be active)

### Injection
1. Build the project (generates `SecretiveRendering.dll`)
2. Use a DLL injector to inject into the TF2 process
3. Verify console output shows successful initialization

### Controls
- **F1**: Toggle overlay visibility
- **DELETE**: Exit and unload the hook

## 🏗️ Architecture

### Modern Steam Overlay Hooking (2025)
```
Game Process (TF2 x64)
├── gameoverlayrenderer64.dll (Steam Overlay)
│   ├── Present Function (Pattern: 48 8B ? 88 00 00 00 E8)
│   └── Reset Function (Pattern: 48 8B ? 80 00 00 00 E8)
├── SecretiveRendering.dll (Our Hook)
│   ├── LEA Instruction Analysis
│   ├── Pattern-Based Function Extraction
│   └── ImGui Overlay Rendering
└── Steam Integration
    ├── Process Validation (tf2.exe, hl2.exe, tf_win64.exe)
    └── Overlay Module Verification
```

### Key Technical Improvements
- **LEA Instruction Analysis**: Extracts function addresses from call patterns
- **Multiple Pattern Fallback**: Tries various offset positions for robustness
- **Memory Protection Validation**: Ensures hooked addresses are executable
- **64-bit Pointer Handling**: Proper uintptr_t usage throughout

## 🎯 Target Patterns (TF2 Steam Overlay)

### Current Working Patterns (2025)
```cpp
// Present function hook registration call
"48 8B ? 88 00 00 00 E8"  // MOV + CALL with 0x88 offset

// Reset function hook registration call  
"48 8B ? 80 00 00 00 E8"  // MOV + CALL with 0x80 offset
```

### Pattern Extraction Process
1. Find pattern in `gameoverlayrenderer64.dll`
2. Locate LEA instruction at offset -7 (or scan -15 to -3)
3. Extract relative offset from LEA RDX, [RIP+offset]
4. Calculate absolute function address
5. Validate address is executable memory

## 🔍 Debugging

### Console Output
The hook provides detailed logging:
```
=== TF2 SecretiveRendering v2.0.0 (x64) ===
Target Game: Team Fortress 2
Architecture: x64
Process validation: 1
Steam overlay module found: 0x7FF8A2340000
Pattern found for Present: 0x7FF8A2367B20
Extracted Steam function: 0x7FF8A234E890
TF2 Steam Overlay hooks installed successfully: 2
```

### Common Issues

**Pattern not found:**
- Steam overlay updated - patterns need updating
- Use Ghidra to analyze `gameoverlayrenderer64.dll`
- Look for new offset values in hook registration calls

**Hook creation failed:**
- Target function not in executable memory
- Pattern extracted invalid address
- Steam overlay not properly initialized

**Process validation failed:**
- Not running in TF2 process
- Use supported TF2 executables (tf2.exe, hl2.exe, tf_win64.exe)

## 🔒 Security & Compliance

- **Read-Only Scanning**: Only scans for patterns, no arbitrary memory modification
- **Steam Compliance**: Works within Steam's overlay architecture  
- **Validation**: All memory accesses validated before use
- **Exception Handling**: Graceful error handling prevents crashes

## 📋 Version History

- **v2.0.0** (2025): Complete rewrite for x64 TF2
  - 64-bit architecture enforcement
  - Modern Steam overlay pattern support
  - LEA instruction analysis for function extraction
  - TF2-specific process validation
  - Enhanced error handling and logging
- **v1.x**: Legacy 32-bit CSGO versions (deprecated)

## 🙏 Credits

- Original technique by [@aixxe](https://github.com/aixxe) - [Steam Overlay Rendering](https://aixxe.net/2017/09/steam-overlay-rendering)
- Pattern scanning by [learn_more](https://github.com/learn-more/findpattern-bench)
- MinHook library for function hooking
- ImGui for overlay interface

## ⚖️ License

MIT License - Use responsibly for legitimate security research purposes only.
