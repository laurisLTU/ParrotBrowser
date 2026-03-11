@echo off
setlocal enabledelayedexpansion

:: 1. Setup MSVC Environment
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
call "%VS_PATH%" > nul

:: 2. Create Folders
if not exist "buildobjs" mkdir buildobjs
if not exist "dist" mkdir dist

:: 3. Paths
set "IMGUI_DIR=..\thirdparty\imgui"
set "WEBVIEW_DIR=..\thirdparty\Microsoft.Web.WebView2.1.0.2592.51\build\native"

:: 4. Automatically find all .cpp in root
set "SOURCES="
for %%f in (*.cpp) do (
    set "SOURCES=!SOURCES! ..\%%f"
)

:: 5. ImGui Sources
set "IMGUI_SOURCES=%IMGUI_DIR%\imgui.cpp %IMGUI_DIR%\imgui_draw.cpp %IMGUI_DIR%\imgui_tables.cpp %IMGUI_DIR%\imgui_widgets.cpp %IMGUI_DIR%\backends\imgui_impl_win32.cpp %IMGUI_DIR%\backends\imgui_impl_dx11.cpp"

:: 6. Create Response File for Compiler
echo /nologo /Zi /MT /EHsc > buildobjs\args.rsp
echo /I%IMGUI_DIR% /I%IMGUI_DIR%\backends /I%WEBVIEW_DIR%\include >> buildobjs\args.rsp
echo %SOURCES% >> buildobjs\args.rsp
echo %IMGUI_SOURCES% >> buildobjs\args.rsp
echo /link d3d11.lib d3dcompiler.lib dwmapi.lib user32.lib "%WEBVIEW_DIR%\x64\WebView2Loader.dll.lib" /OUT:..\dist\ParrotBrowser.exe >> buildobjs\args.rsp

:: 7. Compile (Execute from inside buildobjs to keep it messy there, not in dist)
echo [ParrotBrowser] Compiling...
cd buildobjs
cl @args.rsp

:: 8. Clean up dist folder (Copy only what's needed)
cd ..
copy /Y "%WEBVIEW_DIR%\x64\WebView2Loader.dll" "dist\" >nul
copy /Y "C:\Windows\System32\D3DCOMPILER_47.dll" "dist\" >nul

echo.
echo ==============================================
echo SUCCESS! Your clean build is in the 'dist' folder.
echo (The 'buildobjs' folder contains the messy files).
echo ==============================================
pause