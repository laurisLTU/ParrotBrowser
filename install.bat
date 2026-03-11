@echo off
echo Installing dependencies for ParrotBrowser...

if not exist "thirdparty" mkdir thirdparty
cd thirdparty

echo [1/3] Downloading Dear ImGui...
if not exist "imgui" (
    git clone -b docking https://github.com/ocornut/imgui.git
) else (
    echo ImGui already exists.
)

echo [2/3] Downloading NuGet...
if not exist "nuget.exe" (
    curl -o nuget.exe https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
)

echo [3/3] Downloading Microsoft Edge WebView2 SDK...
nuget install Microsoft.Web.WebView2 -Version 1.0.2592.51

cd ..
echo Setup complete! You can now run compile.bat.
pause