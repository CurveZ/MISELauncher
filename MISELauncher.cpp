/*
 * MISELauncher.cpp
 * A custom launcher for The Secret of Monkey Island Special Edition.
 *
 * Author: keRveL
 * Version: version.txt/dynamic
 * Date: May 2025
 * Description: This program allows users to configure game settings
 *              such as language, resolution, and display mode before
 *              launching the game via Steam.
 * Note: "Look behind you, a three-headed monkey!"
 */

// Include necessary Windows headers for GUI and file operations

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tchar.h>
#include <string>
#include <fstream>
#include <sstream>
#include "resource.h"


// Link required libraries for Windows functionality
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

// Define an ID for the language combo box (because every pirate needs a unique ID)
#define ID_LANG_COMBO 2001 

// Define a version number for the application (because pirates love to keep track of their loot)
#ifndef VERSION
#define VERSION "vUnknown"
#endif

const std::string windowTitle = "Monkey Launcher - " VERSION " by Curvez 2025"; // Window title with version number

// Declare global variables for UI elements (because pirates don't like surprises)
HWND hLaunchBtn, hSaveBtn, hEditBox, 
        hIniPathLabel, hIniPathTextLabel, hResetBtn, hLangCombo, hExitBtn,
        hResolutionCombo, hSaveReminderLabel,
        hSubtitlesCheckbox, hShadersCheckbox;

// Track whether additional options are visible (like a treasure map hidden in plain sight)
bool optionsVisible = false; 
std::string gamePath, iniPath;

// Function to get the path to the settings.ini file
// "That's the second biggest path I've ever seen!"
std::string GetINIPath() {
    char appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata))) {
        std::string path = std::string(appdata) + "\\LucasArts\\The Secret of Monkey Island Special Edition\\settings.ini";
        return path;
    }
    return ""; // Return an empty string if the path can't be found (like a treasure chest with no gold)
}

// Function to read the content of a file
// "Never pay more than 20 pieces of eight for a file reader!"
std::string ReadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "[Failed to open file]"; // If the file can't be opened, we raise the Jolly Roger
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Normalize Windows newlines (\r\n) to Unix-style newlines (\n)
// "You fight like a dairy farmer!" - "How appropriate, you normalize like a cow!"
std::string normalizeWindowsNewlines(const std::string& text) {
    std::string result;
    size_t len = text.length();
    for (size_t i = 0; i < len; ++i) {
        if (text[i] == '\r') {
            if (i + 1 < len && text[i + 1] == '\n') {
                result += '\n';
                ++i; // Skip \n
            } else {
                result += '\n'; // Handle lone \r
            }
        } else {
            result += text[i];
        }
    }
    return result;
}

// Function to write content to a file
// "This is the second biggest file writer I've ever seen!"
bool WriteFile(const std::string& path, const std::string& contentFromEditBox) {
    // Normalize \r\n to \n before saving (because pirates like consistency)
    std::string normalized = normalizeWindowsNewlines(contentFromEditBox);

    std::ofstream file(path, std::ios::binary); // Binary avoids Windows auto newline conversion
    if (!file) return false; // If the file can't be opened, abandon ship!

    file << normalized;
    return true;
}

// Helper function to trim whitespace and line-ending characters
// "That's the cleanest trim I've ever seen on a pirate!"
std::string Trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Load settings from the INI file into the edit box
// "This is the second biggest settings loader I've ever seen!"
void LoadSettingsToEditBox() {
    std::string content = ReadFile(iniPath);
    std::string formattedContent;
    for (char c : content) {
        if (c == '\n') {
            formattedContent += "\r\n";
        } else {
            formattedContent += c;
        }
    }
    SetWindowTextA(hEditBox, formattedContent.c_str());

    // Update the language combo box
    size_t langPos = content.find("language=");
    if (langPos != std::string::npos) {
        int langValue = content[langPos + 9] - '0'; // Extract the language value
        SendMessageA(hLangCombo, CB_SETCURSEL, langValue, 0);
    }

    // Update the resolution combo box
    size_t resPos = content.find("resolution=");
    size_t windowedPos = content.find("windowed=");
    if (resPos != std::string::npos && windowedPos != std::string::npos) {
        // Extract the resolution value
        size_t resStart = resPos + 11; // "resolution=" is 11 characters long
        size_t resEnd = content.find("\r", resStart); // Find the end of the line (before \r)
        if (resEnd == std::string::npos) {
            resEnd = content.find("\n", resStart); // Fallback to \n if \r is not found
        }
        if (resEnd == std::string::npos) {
            resEnd = content.length(); // If no line ending is found, use the end of the content
        }
        std::string resolutionValue = content.substr(resStart, resEnd - resStart);

        // Trim the resolution value
        resolutionValue = Trim(resolutionValue);

        // Extract the windowed value
        bool isWindowed = content[windowedPos + 9] == '1'; // "windowed=" is 9 characters long

        // Match the resolution and windowed values to the combo box items
        int selectedIndex = -1;
        if (!isWindowed && resolutionValue == "3840x2160") {
            selectedIndex = 1; // "4K UHD  - Full Screen (3840x2160)"
        } else if (isWindowed && resolutionValue == "3840x2160") {
            selectedIndex = 2; // "4K UHD  - Windowed    (3840x2160)"
        } else if (!isWindowed && resolutionValue == "2560x1440") {
            selectedIndex = 3; // "QHD/2K  - Full Screen (2560x1440)"
        } else if (isWindowed && resolutionValue == "2560x1440") {
            selectedIndex = 4; // "QHD/2K  - Windowed    (2560x1440)"
        } else if (!isWindowed && resolutionValue == "1920x1080") {
            selectedIndex = 5; // "1080p   - Full Screen (1920x1080)"
        } else if (isWindowed && resolutionValue == "1920x1080") {
            selectedIndex = 6; // "1080p   - Windowed    (1920x1080)"
        }

        // Select the appropriate item in the combo box
        // "You fight like a dairy farmer!" - "How appropriate, you select like a cow!"
        if (selectedIndex != -1) {
            SendMessageA(hResolutionCombo, CB_SETCURSEL, selectedIndex, 0);
        } else {
            SendMessageA(hResolutionCombo, CB_SETCURSEL, -1, 0); // Leave empty if not found
        }
    }    //else {
        // Show a message box if keys are missing (debugging purposes)
        // "That's the second biggest missing key I've ever seen!"
        // MessageBoxA(NULL, "Missing resolution or windowed keys in INI file.", "Debug", MB_OK);
        //}

    // Update the shaders checkbox
    // "Shaders? What shaders? Oh, those shaders!"
    size_t shadersPos = content.find("shaders=");
    if (shadersPos != std::string::npos) {
        bool shadersEnabled = content[shadersPos + 8] == '1';
        SendMessageA(hShadersCheckbox, BM_SETCHECK, shadersEnabled ? BST_CHECKED : BST_UNCHECKED, 0);
    }

    // Update the subtitles checkbox
    // "Subtitles: Because sometimes pirates mumble."
    size_t subtitlesPos = content.find("subtitles=");
    if (subtitlesPos != std::string::npos) {
        bool subtitlesEnabled = content[subtitlesPos + 10] == '1';
        SendMessageA(hSubtitlesCheckbox, BM_SETCHECK, subtitlesEnabled ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

// Save settings from the edit box to the INI file
// "This is the second biggest save button I've ever seen!"
void SaveSettingsFromEditBox() {
    char buffer[8192];
    GetWindowTextA(hEditBox, buffer, sizeof(buffer));
    WriteFile(iniPath, buffer);
}

// Reset settings to default values
// "Resetting to defaults: It's like finding buried treasure!"
void ResetToDefaults() {
    const char* defaultSettings =
        "[localization]\r\n"
        "language=0\r\n"
        "[display]\r\n"
        "windowed=0\r\n"
        "shaders=1\r\n"
        "resolution=3840x2160\r\n"
        "[audio]\r\n"
        "music=70\r\n"
        "voice=80\r\n"
        "sfx=70\r\n"
        "subtitles=1\r\n";

    // Reset the edit box to default settings
    SetWindowTextA(hEditBox, defaultSettings);

    // Reset the language combo box to English (index 0)
    SendMessageA(hLangCombo, CB_SETCURSEL, 0, 0);

    // Reset the resolution combo box to 4K Full Screen (index 0)
    SendMessageA(hResolutionCombo, CB_SETCURSEL, 1, 0);
    SendMessageA(hSubtitlesCheckbox, BM_SETCHECK, BST_CHECKED, 0); // Default: checked
    SendMessageA(hShadersCheckbox, BM_SETCHECK, BST_CHECKED, 0);   // Default: checked
}

// Launch the game executable
// "Launching the game: It's like setting sail for Monkey Island!"
void LaunchGame() {
    // Retrieve the Steam installation path from the registry
    char steamPath[MAX_PATH] = {0};
    HKEY hKey;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD pathSize = sizeof(steamPath);
        if (RegQueryValueExA(hKey, "SteamExe", nullptr, nullptr, (LPBYTE)steamPath, &pathSize) != ERROR_SUCCESS) {
            MessageBoxA(NULL, "Failed to retrieve Steam path from the registry.", "Error", MB_ICONERROR);
            RegCloseKey(hKey);
            return;
        }
        RegCloseKey(hKey);
    } else {
        MessageBoxA(NULL, "Failed to open Steam registry key.", "Error", MB_ICONERROR);
        return;
    }

    // Ensure the Steam path is valid
    if (strlen(steamPath) == 0) {
        MessageBoxA(NULL, "Steam path is empty. Ensure Steam is installed.", "Error", MB_ICONERROR);
        return;
    }

    // Construct the command to launch the game via Steam
    std::string command = std::string(steamPath) + " steam://launch/32360";

    // Launch the game using ShellExecuteA
    HINSTANCE result = ShellExecuteA(NULL, "open", steamPath, "steam://launch/32360", NULL, SW_SHOWNORMAL);

    // Check for errors without casting to int
    if (reinterpret_cast<intptr_t>(result) <= 32) {
        MessageBoxA(NULL, "Failed to launch the game via Steam. Ensure Steam is installed and running.", "Error", MB_ICONERROR);
    } else {
        //Disable the message box for successful launch
        //MessageBoxA(NULL, "Game launched successfully via Steam!", "Info", MB_OK);
    }
}


// Function to get the current desktop resolution (primary display)
std::string GetDesktopResolution() {
    DEVMODE devMode = {};
    devMode.dmSize = sizeof(DEVMODE);

    // Get the current display settings for the primary monitor
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        int width = devMode.dmPelsWidth;
        int height = devMode.dmPelsHeight;
        return std::to_string(width) + "x" + std::to_string(height);
    }

    MessageBoxA(NULL, "Failed to retrieve desktop resolution. Using default resolution.", "Error", MB_ICONERROR);
    return "1920x1080";
}

// Window procedure for handling messages
// "Handling messages: It's like deciphering a pirate's map!"
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            if ((HWND)lParam == hEditBox && HIWORD(wParam) == EN_CHANGE) {
                // Show the "Remember to save!" label
                ShowWindow(hSaveReminderLabel, SW_SHOW);
            }
            if ((HWND)lParam == hResolutionCombo && HIWORD(wParam) == CBN_SELCHANGE) { // Handle resolution change
                // Get the selected resolution
                char resolution[64];
                int selected = SendMessageA(hResolutionCombo, CB_GETCURSEL, 0, 0);
                SendMessageA(hResolutionCombo, CB_GETLBTEXT, selected, (LPARAM)resolution);

                if (strcmp(resolution, "Autodetect/Recommend Resolution") == 0) {
                    // Autodetect the desktop resolution
                    std::string desktopResolution = GetDesktopResolution();

                    // Update the resolution and windowed fields in the edit box
                    char buffer[8192];
                    GetWindowTextA(hEditBox, buffer, sizeof(buffer));
                    std::string content(buffer);

                    // Update the resolution field
                    size_t pos = content.find("resolution=");
                    if (pos != std::string::npos) {
                        size_t start = pos + 11; // "resolution=" is 11 characters long
                        size_t end = content.find("\r\n", start);
                        content.replace(start, end - start, desktopResolution);
                    }

                    // Set windowed mode to 0 (default to fullscreen for autodetect)
                    pos = content.find("windowed=");
                    if (pos != std::string::npos) {
                        size_t start = pos + 9; // "windowed=" is 9 characters long
                        size_t end = content.find("\r\n", start);
                        content.replace(start, end - start, "0");
                    }

                    // Update the edit box with the new content
                    SetWindowTextA(hEditBox, content.c_str());
                    ShowWindow(hSaveReminderLabel, SW_SHOW);

                    // Optionally, show a message box to confirm the change
                    //MessageBoxA(hwnd, ("Resolution set to " + desktopResolution + "\nFull Screen").c_str(), "Autodetect Resolution", MB_OK);
                } else {
                    // Handle other resolution options (existing logic)
                    bool isWindowed = strstr(resolution, "Windowed") != nullptr;
                    std::string resolutionValue;
                    const char* start = strchr(resolution, '(');
                    const char* end = strchr(resolution, ')');
                    if (start && end && start < end) {
                        resolutionValue = std::string(start + 1, end - start - 1);
                    }

                    // Update the resolution and windowed fields in the edit box
                    char buffer[8192];
                    GetWindowTextA(hEditBox, buffer, sizeof(buffer));
                    std::string content(buffer);

                    size_t pos = content.find("resolution=");
                    if (pos != std::string::npos) {
                        size_t start = pos + 11;
                        size_t end = content.find("\r\n", start);
                        content.replace(start, end - start, resolutionValue);
                    }

                    pos = content.find("windowed=");
                    if (pos != std::string::npos) {
                        size_t start = pos + 9;
                        size_t end = content.find("\r\n", start);
                        content.replace(start, end - start, isWindowed ? "1" : "0");
                    }

                    SetWindowTextA(hEditBox, content.c_str());
                    ShowWindow(hSaveReminderLabel, SW_SHOW);
                }
            } else if ((HWND)lParam == hLaunchBtn) { // Launch the game
                LaunchGame();
            
            } else if ((HWND)lParam == hSaveBtn) { // Save settings
                SaveSettingsFromEditBox();
                //MessageBoxA(hwnd, "Settings saved.", "Info", MB_OK);

                // Hide the "Remember to save!" label
                ShowWindow(hSaveReminderLabel, SW_HIDE);
            
            } else if ((HWND)lParam == hResetBtn) { // Reset to defaults
                ResetToDefaults();
                //MessageBoxA(hwnd, "Defaults restored. Don't forget to save!", "Info", MB_OK);
                ShowWindow(hSaveReminderLabel, SW_SHOW);
            
            } else if ((HWND)lParam == hExitBtn) { // Exit the application
                DestroyWindow(hwnd);  // Destroy the window
                PostQuitMessage(0);   // Exit the message loop

            } else if ((HWND)lParam == hResolutionCombo && HIWORD(wParam) == CBN_SELCHANGE) { // Handle resolution change
                // Get the selected resolution
                char resolution[64];
                int selected = SendMessageA(hResolutionCombo, CB_GETCURSEL, 0, 0);
                SendMessageA(hResolutionCombo, CB_GETLBTEXT, selected, (LPARAM)resolution);
                // Determine if the selection is Full Screen or Windowed
                bool isWindowed = strstr(resolution, "Windowed") != nullptr;
                // Extract the resolution value (e.g., "3840x2160")
                std::string resolutionValue;
                const char* start = strchr(resolution, '(');
                const char* end = strchr(resolution, ')');
                if (start && end && start < end) {
                    resolutionValue = std::string(start + 1, end - start - 1);
                }
                // Update the resolution and windowed fields in the edit box
                char buffer[8192];
                GetWindowTextA(hEditBox, buffer, sizeof(buffer));
                std::string content(buffer);
                // Update the resolution field
                size_t pos = content.find("resolution=");
                if (pos != std::string::npos) {
                    size_t start = pos + 11; // "resolution=" is 11 characters long
                    size_t end = content.find("\r\n", start);
                    content.replace(start, end - start, resolutionValue);
                }
                // Update the windowed field
                pos = content.find("windowed=");
                if (pos != std::string::npos) {
                    size_t start = pos + 9; // "windowed=" is 9 characters long
                    size_t end = content.find("\r\n", start);
                    content.replace(start, end - start, isWindowed ? "1" : "0");
                }
                SetWindowTextA(hEditBox, content.c_str()); // Update the edit box with the new content
                ShowWindow(hSaveReminderLabel, SW_SHOW);

            } else if (LOWORD(wParam) == ID_LANG_COMBO && HIWORD(wParam) == CBN_SELCHANGE) { // Handle language change
                int selected = SendMessageA(hLangCombo, CB_GETCURSEL, 0, 0);
                char buffer[8192];
                GetWindowTextA(hEditBox, buffer, sizeof(buffer));
                std::string content(buffer);
                size_t pos = content.find("language=");
                if (pos != std::string::npos) {
                size_t start = pos + 9;  // "language=" is 9 characters long
                size_t end = start + 1;  // The language number is 1 character long
                content.replace(start, end - start, std::to_string(selected));
                }
                SetWindowTextA(hEditBox, content.c_str());
                ShowWindow(hSaveReminderLabel, SW_SHOW);

            } else if ((HWND)lParam == hSubtitlesCheckbox) { // Handle subtitles checkbox
                // Toggle subtitles setting
                bool isChecked = SendMessageA(hSubtitlesCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED;
                char buffer[8192];
                GetWindowTextA(hEditBox, buffer, sizeof(buffer));
                std::string content(buffer);
                size_t pos = content.find("subtitles=");
                if (pos != std::string::npos) {
                    size_t start = pos + 10; // "subtitles=" is 10 characters long
                    size_t end = content.find("\r\n", start);
                    content.replace(start, end - start, isChecked ? "1" : "0");
                }
                SetWindowTextA(hEditBox, content.c_str());
                ShowWindow(hSaveReminderLabel, SW_SHOW);

            } else if ((HWND)lParam == hShadersCheckbox) { // Handle shaders checkbox
                // Toggle shaders setting
                bool isChecked = SendMessageA(hShadersCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED;
                char buffer[8192];
                GetWindowTextA(hEditBox, buffer, sizeof(buffer));
                std::string content(buffer);
                size_t pos = content.find("shaders=");
                if (pos != std::string::npos) {
                    size_t start = pos + 8; // "shaders=" is 8 characters long
                    size_t end = content.find("\r\n", start);
                    content.replace(start, end - start, isChecked ? "1" : "0");
                }
                SetWindowTextA(hEditBox, content.c_str());
                ShowWindow(hSaveReminderLabel, SW_SHOW);
            }
            break;

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hStatic = (HWND)lParam;

            if (
                hStatic == hSaveReminderLabel ||
                hStatic == hIniPathLabel ||
                hStatic == hIniPathTextLabel ||
                hStatic == hSubtitlesCheckbox ||
                hStatic == hShadersCheckbox
            ) {
                if (hStatic == hSaveReminderLabel) {
                    SetTextColor(hdcStatic, RGB(255, 0, 0)); // Red text for the reminder
                } else {
                    SetTextColor(hdcStatic, RGB(0, 0, 0));   // Black text for others
                }
                SetBkMode(hdcStatic, TRANSPARENT); // Transparent background for all
                return (LRESULT)GetStockObject(NULL_BRUSH);
            }
            // For all other static controls, use the default background.
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_SETCURSOR: // Set the cursor to the default arrow cursor
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return TRUE;

        case WM_CLOSE:
            DestroyWindow(hwnd);  // Destroy the window
            PostQuitMessage(0);   // Exit the message loop
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Entry point
// "This is the second biggest entry point I've ever seen!"
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    iniPath = GetINIPath();
    if (iniPath.empty()) {
        MessageBoxA(NULL, "settings.ini file not found!", "Error", MB_ICONERROR);
        return 1;
    }

    // Construct the window title with the version number
    //const std::string windowTitle = std::string("Monkey Launcher - by Curvez 2025 " VERSION "").c_str();


    const char CLASS_NAME[] = "MonkeyLauncher";
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MYICON));

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, windowTitle.c_str(),
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 792, 480, // Adjusted window size
        NULL, NULL, hInst, NULL
    );

    HFONT hFontLarge = CreateFontA(
        18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas"
    );

    HFONT hFontSmall = CreateFontA(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas"
    );

    HFONT hFontEmoji = CreateFontA(
        18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI Emoji"
    );

    hIniPathTextLabel = CreateWindowA("STATIC", "Settings INI Path:", WS_VISIBLE | WS_CHILD, 20, 20, 120, 20, hwnd, NULL, hInst, NULL);
    hIniPathLabel = CreateWindowA("STATIC", iniPath.c_str(), WS_VISIBLE | WS_CHILD | SS_LEFT | SS_NOPREFIX, 150, 20, 600, 40, hwnd, NULL, hInst, NULL);
    SendMessageA(hIniPathLabel, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

    hEditBox = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                             20, 80, 740, 250, hwnd, NULL, hInst, NULL); // Text box remains at y = 80
    SendMessageA(hEditBox, WM_SETFONT, (WPARAM)hFontLarge, TRUE);

    hLangCombo = CreateWindowA("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL,
                               20, 340, 200, 150, hwnd, (HMENU)ID_LANG_COMBO, hInst, NULL); // Moved closer to the text box
    SendMessageA(hLangCombo, WM_SETFONT, (WPARAM)hFontLarge, TRUE);

    SendMessageA(hLangCombo, CB_ADDSTRING, 0, (LPARAM)"English");
    SendMessageA(hLangCombo, CB_ADDSTRING, 0, (LPARAM)"French");
    SendMessageA(hLangCombo, CB_ADDSTRING, 0, (LPARAM)"Italian");
    SendMessageA(hLangCombo, CB_ADDSTRING, 0, (LPARAM)"German");
    SendMessageA(hLangCombo, CB_ADDSTRING, 0, (LPARAM)"Spanish");

    // Create the resolution combo box
    hResolutionCombo = CreateWindowA("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL,
                                     240, 340, 300, 230, hwnd, NULL, hInst, NULL); // Same y position as language combo box
    SendMessageA(hResolutionCombo, WM_SETFONT, (WPARAM)hFontLarge, TRUE);

    // Add resolutions to the combo box
    SendMessageA(hResolutionCombo, CB_ADDSTRING, 0, (LPARAM)"Autodetect/Recommend Resolution");
    SendMessageA(hResolutionCombo, CB_ADDSTRING, 0, (LPARAM)"4K UHD  - Full Screen (3840x2160)");
    SendMessageA(hResolutionCombo, CB_ADDSTRING, 0, (LPARAM)"4K UHD  - Windowed    (3840x2160)");
    SendMessageA(hResolutionCombo, CB_ADDSTRING, 0, (LPARAM)"QHD/2K  - Full Screen (2560x1440)");
    SendMessageA(hResolutionCombo, CB_ADDSTRING, 0, (LPARAM)"QHD/2K  - Windowed    (2560x1440)");
    SendMessageA(hResolutionCombo, CB_ADDSTRING, 0, (LPARAM)"Full HD - Full Screen (1920x1080)");
    SendMessageA(hResolutionCombo, CB_ADDSTRING, 0, (LPARAM)"Full HD - Windowed    (1920x1080)");

    // Subtitles checkbox
    hSubtitlesCheckbox = CreateWindowA("BUTTON", "Enable Subtitles", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                       20, 365, 200, 30, hwnd, NULL, hInst, NULL); // Moved to y = 385
    SendMessageA(hSubtitlesCheckbox, WM_SETFONT, (WPARAM)hFontLarge, TRUE);
    SendMessageA(hSubtitlesCheckbox, BM_SETCHECK, BST_CHECKED, 0); // Default value: checked

    // Shaders checkbox
    hShadersCheckbox = CreateWindowA("BUTTON", "Enable Shaders", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                     240, 365, 200, 30, hwnd, NULL, hInst, NULL); // Moved to y = 385
    SendMessageA(hShadersCheckbox, WM_SETFONT, (WPARAM)hFontLarge, TRUE);
    SendMessageA(hShadersCheckbox, BM_SETCHECK, BST_CHECKED, 0); // Default value: checked

    hLaunchBtn = CreateWindowW(L"BUTTON", L"🙊 Launch Game", WS_VISIBLE | WS_CHILD, 20, 405, 150, 30, hwnd, NULL, hInst, NULL); 
    SendMessageW(hLaunchBtn, WM_SETFONT, (WPARAM)hFontEmoji, TRUE);

    hSaveBtn = CreateWindowW(L"BUTTON", L"🙈 Save Settings", WS_VISIBLE | WS_CHILD, 205, 405, 150, 30, hwnd, NULL, hInst, NULL); 
    SendMessageW(hSaveBtn, WM_SETFONT, (WPARAM)hFontEmoji, TRUE);

    hResetBtn = CreateWindowW(L"BUTTON", L"🙉 Reset Defaults", WS_VISIBLE | WS_CHILD, 390, 405, 150, 30, hwnd, NULL, hInst, NULL); 
    SendMessageW(hResetBtn, WM_SETFONT, (WPARAM)hFontEmoji, TRUE);
    
    // Exit button
    hExitBtn = CreateWindowW(L"BUTTON", L"❌ Exit!", WS_VISIBLE | WS_CHILD , 610, 405, 150, 30, hwnd, NULL, hInst, NULL);
    SendMessageW(hExitBtn, WM_SETFONT, (WPARAM)hFontEmoji, TRUE);
    
    // Create a label for the Save Reminder
    hSaveReminderLabel = CreateWindowA(
        "STATIC", "Remember to save!", WS_VISIBLE | WS_CHILD | SS_RIGHT,
        600, 340, 160, 20, hwnd, NULL, hInst, NULL
    );
        SendMessageA(hSaveReminderLabel, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
        ShowWindow(hSaveReminderLabel, SW_HIDE); // Initially hidden

        

    LoadSettingsToEditBox();
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hFontLarge); // Clean up font object
    DeleteObject(hFontSmall); // Clean up font object
    DeleteObject(hFontEmoji); // Clean up button font
    return 0;
}
