#PowerShell script build.ps1
# Read the version from version.txt
$VERSION = Get-Content -Path "version.txt" -Raw
# Compile the program with the version number
g++ MISELauncher.cpp resource.res -DVERSION="`"$VERSION`"" -mwindows -lole32 -luuid -lshlwapi -lshell32 -o MISELauncher.exe -static
# Optionally, push the release to GitHub
Write-Host "Compiled Monkey Launcher with version $VERSION"