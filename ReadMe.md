# MISELauncher

A custom launcher for **The Secret of Monkey Island Special Edition**. This launcher provides an easy way to configure game settings, such as language, resolution, and display mode, before launching the game.

## Why Use This Launcher?

- **Change Language Anytime:**
  - The game only allows you to select the language once, and you're stuck with your choice. This launcher makes it easy to change the language whenever you want.
  
- **Fix Resolution Issues:**
  - If you accidentally select a resolution that your screen doesn't support, this launcher allows you to change it without hassle.

- **Additional Features:**
  - Toggle subtitles and shaders.
  - Switch between windowed and full-screen modes.

## Features

- **Customizable Settings:**
  - Language selection
  - Resolution (4K UHD, 1080p, 720p)
  - Windowed or Full-Screen mode
  - Subtitles and shaders toggle
- **INI File Management:**
  - Reads and writes `settings.ini` for game configuration.
- **Steam Integration:**
  - Launches the game directly via Steam.

## Requirements

- **Operating System:** Windows 7 or above (Windows 10 recommended)
- **Architecture:** 32-bit or 64-bit
- **Steam:** Installed and logged in
- **Game:** Purchased version of **The Secret of Monkey Island Special Edition** installed via Steam
- **Standalone Launcher:** The `.exe` file is standalone and does not require installation.

## How to Build

1. Compile the resource file (`resource.rc`) into a `.res` file using `windres`:
   ```bash
   windres resource.rc -O coff -o resource.res
   ```
2. Compile the project using the following command:
   ```bash
   g++ MISELauncher.cpp resource.res -mwindows -lole32 -luuid -lshlwapi -lshell32 -o MISELauncher.exe -static
   ```

## Download

You can download the latest version of the launcher here:

[Download MISELauncher.exe](https://github.com/CurveZ/MISELauncher/releases/latest)




