#include "basicHook.h"

#define REGISTER_HOOK(pTarget, pDetour, ppOriginal) Hook(pTarget, pDetour, ppOriginal); \
originalFunctions.push_back(pTarget);

// 64-bit function signatures for TF2 Steam overlay
using tPresent = HRESULT(STDMETHODCALLTYPE*) (IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
using tReset = HRESULT (STDMETHODCALLTYPE*) (IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

// Original function pointers for 64-bit TF2
tPresent oPresent = nullptr;
tReset oReset = nullptr;

// TF2Config constants are now defined in basicHook.h

// Global state for overlay visibility
static bool g_overlayVisible = true;
static bool g_initialized = false;

/**
 * @brief Extract Steam overlay function address from pattern using LEA instruction analysis
 * @param pattern Pattern to search for in Steam overlay
 * @param patternName Name for logging
 * @return Steam function address, or 0 if not found
 */
uintptr_t ExtractSteamFunction(const char* pattern, const char* patternName) {
    uintptr_t patternAddr = FindPattern(TF2Config::STEAM_OVERLAY_DLL, pattern);
    if (!patternAddr) {
        LOGHEX("Pattern not found for", patternName);
        return 0;
    }

    LOGHEX("Found pattern for", patternName);
    LOGHEX("Pattern address", patternAddr);

    // Extract function address using LEA instruction analysis
    uintptr_t functionAddr = ExtractFunctionFromLEA(patternAddr, -7);
    if (!functionAddr) {
        // Try alternative offsets if standard -7 doesn't work
        for (int offset = -15; offset <= -3; offset++) {
            functionAddr = ExtractFunctionFromLEA(patternAddr, offset);
            if (functionAddr) {
                LOGHEX("Found function with offset", offset);
                break;
            }
        }
    }

    if (functionAddr && IsValidExecutableAddress(functionAddr)) {
        LOGHEX("Extracted Steam function", functionAddr);
        return functionAddr;
    }

    LOGHEX("Failed to extract valid function for", patternName);
    return 0;
}

/**
 * @brief TF2 Present hook - renders overlay interface
 */
HRESULT STDMETHODCALLTYPE hkPresent(IDirect3DDevice9* thisptr, const RECT* src, const RECT* dest, HWND wnd_override, const RGNDATA* dirty_region) {
    // Initialize ImGui on first call
    if (!g_initialized && thisptr) {
        HRESULT deviceState = thisptr->TestCooperativeLevel();
        if (deviceState == D3D_OK) {
            imguiHook::InitializeImgui(thisptr);
            g_initialized = true;
            LOGHEX("ImGui initialized for TF2", reinterpret_cast<uintptr_t>(thisptr));
        } else {
            LOGHEX("D3D device not ready, state", deviceState);
            return oPresent(thisptr, src, dest, wnd_override, dirty_region);
        }
    }

    // Handle overlay toggle
    static bool toggleKeyPressed = false;
    bool toggleKeyDown = (GetAsyncKeyState(TF2Config::OVERLAY_TOGGLE_KEY) & 0x8000) != 0;
    
    if (toggleKeyDown && !toggleKeyPressed) {
        g_overlayVisible = !g_overlayVisible;
        LOGHEX("TF2 Overlay visibility toggled", g_overlayVisible);
    }
    toggleKeyPressed = toggleKeyDown;

    // Render overlay if initialized and visible
    if (g_initialized && g_overlayVisible) {
        try {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // TF2-specific overlay interface
            ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(350, 250), ImGuiCond_FirstUseEver);
            
            if (ImGui::Begin("TF2 Secretive Rendering", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Team Fortress 2 Steam Overlay Hook");
                ImGui::Separator();
                
                ImGui::Text("Architecture: x64");
                ImGui::Text("Target Game: Team Fortress 2");
                ImGui::Text("Steam Module: %s", TF2Config::STEAM_OVERLAY_DLL);
                
                ImGui::Separator();
                ImGui::Text("Frame Rate: %.1f FPS", ImGui::GetIO().Framerate);
                ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
                
                ImGui::Separator();
                ImGui::TextWrapped("This overlay is invisible to streaming software!");
                ImGui::TextWrapped("Press F1 to toggle overlay visibility");
                
                ImGui::Separator();
                if (ImGui::Button("Hide Overlay")) {
                    g_overlayVisible = false;
                }
            }
            ImGui::End();

            // Minimal HUD in corner
            ImGui::SetNextWindowPos(ImVec2(10, 10));
            ImGui::SetNextWindowBgAlpha(0.3f);
            
            if (ImGui::Begin("TF2 HUD", nullptr, 
                             ImGuiWindowFlags_NoTitleBar | 
                             ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("TF2 x64");
                ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
            }
            ImGui::End();

            ImGui::EndFrame();
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        }
        catch (...) {
            LOGHEX("Exception in TF2 overlay rendering", 0);
            g_overlayVisible = false;
        }
    }

    return oPresent(thisptr, src, dest, wnd_override, dirty_region);
}

/**
 * @brief TF2 Reset hook - handles device reset
 */
HRESULT STDMETHODCALLTYPE hkReset(IDirect3DDevice9* thisptr, D3DPRESENT_PARAMETERS* params) {
    LOGHEX("TF2 Device Reset requested", reinterpret_cast<uintptr_t>(thisptr));
    
    if (g_initialized) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }
    
    HRESULT result = oReset(thisptr, params);
    
    if (SUCCEEDED(result) && g_initialized) {
        ImGui_ImplDX9_CreateDeviceObjects();
        LOGHEX("TF2 Device Reset successful", result);
    } else if (!SUCCEEDED(result)) {
        LOGHEX("TF2 Device Reset failed", result);
    }
    
    return result;
}

std::vector<void*> originalFunctions;

void hooks::Initialize()
{
    try {
        LOGHEX("Initializing TF2 Steam Overlay Hook (x64)", 0);
        
        // Verify we're targeting the correct Steam overlay module
        HMODULE overlayModule = GetModuleHandleA(TF2Config::STEAM_OVERLAY_DLL);
        if (!overlayModule) {
            throw std::exception("TF2 Steam overlay module not found! Ensure TF2 is running with Steam overlay enabled.");
        }
        
        LOGHEX("TF2 Steam overlay module found", reinterpret_cast<uintptr_t>(overlayModule));
        
        // Initialize MinHook
        if (MH_Initialize() != MH_OK) {
            throw std::exception("MH_Initialize failed!");
        }
        
        LOGHEX("MinHook initialized for TF2", 0);

        // Extract Steam overlay function addresses using modern pattern analysis
        uintptr_t presentFunction = ExtractSteamFunction(TF2Config::PRESENT_PATTERN, "Present");
        uintptr_t resetFunction = ExtractSteamFunction(TF2Config::RESET_PATTERN, "Reset");

        if (!presentFunction) {
            throw std::exception("Failed to locate TF2 Steam Present function!");
        }

        LOGHEX("TF2 Present function", presentFunction);
        
        // Hook Present function (required)
        REGISTER_HOOK(reinterpret_cast<void*>(presentFunction), &hkPresent, &oPresent);
        
        // Hook Reset function (optional, but recommended)
        if (resetFunction) {
            LOGHEX("TF2 Reset function", resetFunction);
            REGISTER_HOOK(reinterpret_cast<void*>(resetFunction), &hkReset, &oReset);
        } else {
            LOGHEX("TF2 Reset function not found (non-critical)", 0);
        }

        LOGHEX("TF2 Steam Overlay hooks installed successfully", originalFunctions.size());

    } catch (const std::exception &ex) {
        MessageBoxA(nullptr, ex.what(), "TF2 Steam Overlay Hook Error", MB_ICONERROR);
    }
}

void hooks::Uninitialize()
{
    LOGHEX("Uninitializing TF2 Steam Overlay Hook", originalFunctions.size());
    
    // Cleanup ImGui if initialized
    if (g_initialized) {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_initialized = false;
    }
    
    // Remove all hooks
    for (auto& org : originalFunctions) {
        MH_DisableHook(org);
    }
    
    originalFunctions.clear();
    MH_Uninitialize();
    
    LOGHEX("TF2 Steam Overlay Hook cleanup complete", 0);
}

