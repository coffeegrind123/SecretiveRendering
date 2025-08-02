#include "dllmain.h"

// Enforce 64-bit architecture at compile time
#ifndef _WIN64
#error "TF2 SecretiveRendering requires 64-bit compilation. Use x64 platform."
#endif

// TF2 Steam overlay configuration
namespace TF2SecretiveRendering {
    constexpr const char* PROJECT_NAME = "TF2 SecretiveRendering";
    constexpr const char* VERSION = "2.0.0-x64";
    constexpr const char* TARGET_GAME = "Team Fortress 2";
    constexpr DWORD INITIALIZATION_DELAY_MS = 3000; // Wait for TF2 to fully load
    constexpr DWORD EXIT_KEY = VK_DELETE;
}

/**
 * @brief Validate target process is TF2
 * @return true if running in Team Fortress 2 process
 */
bool ValidateTF2Process() {
    char processName[MAX_PATH];
    DWORD processNameLength = GetModuleFileNameA(nullptr, processName, MAX_PATH);
    
    if (processNameLength == 0) {
        LOGHEX("Failed to get process name", GetLastError());
        return false;
    }
    
    // Convert to lowercase for comparison
    for (DWORD i = 0; i < processNameLength; i++) {
        processName[i] = static_cast<char>(tolower(processName[i]));
    }
    
    // Check if this is TF2 process (tf2.exe, hl2.exe, or tf_win64.exe)
    const char* filename = strrchr(processName, '\\');
    if (!filename) filename = processName;
    else filename++; // Skip the backslash
    
    bool isTF2 = (strstr(filename, "tf2") != nullptr) || 
                 (strstr(filename, "hl2") != nullptr) ||
                 (strstr(filename, "tf_win64") != nullptr);
    
    LOGHEX("Process validation", isTF2);
    LOGHEX("Process name", filename);
    
    return isTF2;
}

/**
 * @brief Check if Steam overlay is available for TF2
 * @return true if gameoverlayrenderer64.dll is loaded
 */
bool ValidateSteamOverlay() {
    HMODULE overlayModule = GetModuleHandleA("gameoverlayrenderer64.dll");
    if (!overlayModule) {
        LOGHEX("Steam overlay module not found", 0);
        return false;
    }
    
    LOGHEX("Steam overlay module found", reinterpret_cast<uintptr_t>(overlayModule));
    return true;
}

/**
 * @brief Cleanup function called on DLL detach
 */
void Detach()
{
    LOGHEX("TF2 SecretiveRendering detaching", 0);
    
    // Uninitialize hooks before console cleanup
    hooks::Uninitialize();
    
    // Cleanup console last
    FREECONSOLE()
    
    LOGHEX("TF2 SecretiveRendering detach complete", 0);
}

/**
 * @brief Main thread function for TF2 SecretiveRendering
 * @param lpParameter Module handle passed from DllMain
 * @return Thread exit code
 */
DWORD WINAPI fMain(LPVOID lpParameter)
{
    // Initialize console for debugging
    ALLOCCONSOLE()
    
    LOGHEX("=== TF2 SecretiveRendering v2.0.0 (x64) ===", 0);
    LOGHEX("Target Game", TF2SecretiveRendering::TARGET_GAME);
    LOGHEX("Architecture", "x64");
    LOGHEX("Process ID", GetCurrentProcessId());
    
    // Validate we're running in TF2
    if (!ValidateTF2Process()) {
        LOGHEX("WARNING: Not running in recognized TF2 process", 0);
        LOGHEX("Hook may not work correctly", 0);
        // Continue anyway for testing purposes
    }
    
    // Wait for TF2 and Steam overlay to fully initialize
    LOGHEX("Waiting for TF2 initialization", TF2SecretiveRendering::INITIALIZATION_DELAY_MS);
    Sleep(TF2SecretiveRendering::INITIALIZATION_DELAY_MS);
    
    // Validate Steam overlay is available
    if (!ValidateSteamOverlay()) {
        LOGHEX("ERROR: Steam overlay not available", 0);
        LOGHEX("Ensure Steam overlay is enabled for TF2", 0);
        MessageBoxA(nullptr, 
                   "Steam overlay not found!\n\nPlease ensure:\n"
                   "1. Steam overlay is enabled in Steam settings\n"
                   "2. Steam overlay is enabled for Team Fortress 2\n"
                   "3. TF2 is running through Steam", 
                   "TF2 SecretiveRendering Error", 
                   MB_ICONERROR);
        
        FreeLibraryAndExitThread(static_cast<HMODULE>(lpParameter), EXIT_FAILURE);
        return EXIT_FAILURE;
    }
    
    // Initialize TF2 Steam overlay hooks
    LOGHEX("Initializing TF2 Steam overlay hooks", 0);
    hooks::Initialize();
    
    LOGHEX("TF2 SecretiveRendering initialization complete", 0);
    LOGHEX("Press DELETE to exit", TF2SecretiveRendering::EXIT_KEY);
    
    // Main loop - wait for exit key
    while (true)
    {
        if (GetAsyncKeyState(TF2SecretiveRendering::EXIT_KEY) & 1) {
            LOGHEX("Exit key pressed - shutting down", 0);
            break;
        }
        Sleep(100);
    }
    
    LOGHEX("TF2 SecretiveRendering shutting down", 0);
    FreeLibraryAndExitThread(static_cast<HMODULE>(lpParameter), EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

/**
 * @brief DLL entry point for TF2 SecretiveRendering
 * @param hModule Module handle
 * @param dwReason Reason for calling (attach/detach)
 * @param lpReserved Reserved parameter
 * @return TRUE if successful, FALSE otherwise
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
            // Disable thread library calls for performance
            DisableThreadLibraryCalls(hModule);
            
            // Create main thread to avoid blocking DLL load
            HANDLE hThread = CreateThread(nullptr, 0, fMain, hModule, 0, nullptr);
            if (hThread) {
                CloseHandle(hThread);
            } else {
                // Failed to create thread
                return FALSE;
            }
        }
        break;
        
    case DLL_PROCESS_DETACH:
        // Only cleanup if not process termination
        if (!lpReserved) {
            Detach();
        }
        break;
        
    default:
        break;
    }
    
    return TRUE;
}