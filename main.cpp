#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <string>
#include <wrl.h>
#include <WebView2.h>

// Parrot Modules
#include "settings.h"
#include "search.h"
#include "history.h"
#include "taskbar.h"

using namespace Microsoft::WRL;

// --- DirectX 11 Globals ---
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// --- WebView2 Globals ---
ComPtr<ICoreWebView2Controller> webviewController;
ComPtr<ICoreWebView2>           webviewWindow;

// --- Forward Declarations ---
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main(int, char**) {
    // 1. Setup Win32 Window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ParrotBrowserClass", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"ParrotBrowser", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // 2. Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    // Load user preferences (Theme/Colors)
    LoadConfig(); 

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    
    // Apply the custom color wheel accent
    ApplyCustomStyle();

    // 3. Initialize WebView2
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(hwnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [hwnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                        if (controller != nullptr) {
                            webviewController = controller;
                            webviewController->get_CoreWebView2(&webviewWindow);
                            webviewWindow->Navigate(L"https://www.google.com");
                        }
                        return S_OK;
                    }).Get());
                return S_OK;
            }).Get());

    // 4. Main Loop
    bool done = false;
    static char urlBuffer[1024] = "https://www.google.com";

    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Layout Calculations
        RECT r;
        GetClientRect(hwnd, &r);
        float winW = (float)(r.right - r.left);
        float winH = (float)(r.bottom - r.top);
        float topH = 42.0f;
        float botH = 48.0f;

        // --- TOP NAVIGATION BAR ---
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(winW, topH));
        ImGui::Begin("TopBar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
        
        if (ImGui::Button("Back") && webviewWindow)    webviewWindow->GoBack();
        ImGui::SameLine();
        if (ImGui::Button("Forward") && webviewWindow) webviewWindow->GoForward();
        ImGui::SameLine();
        if (ImGui::Button("Refresh") && webviewWindow) webviewWindow->Reload();

        // Right-aligned Taskbar Button
        ImGui::SameLine(winW - 100.0f);
        if (ImGui::Button("Taskbar", ImVec2(90, 0))) {
            g_ShowTaskbarWindow = !g_ShowTaskbarWindow;
        }
        ImGui::End();

        // --- TASKBAR WINDOW (SIDEBAR) ---
        // This function handles WebView visibility logic internally
        RenderTaskbarWindow(winW, winH);

        // --- BOTTOM SEARCH BAR ---
        ImGui::SetNextWindowPos(ImVec2(0, winH - botH));
        ImGui::SetNextWindowSize(ImVec2(winW, botH));
        ImGui::Begin("BottomBar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Search:"); ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f); 
        if (ImGui::InputTextWithHint("##URL", "Search or enter URL...", urlBuffer, IM_ARRAYSIZE(urlBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (webviewWindow) {
                std::wstring target = SearchEngine::ProcessInput(urlBuffer);
                webviewWindow->Navigate(target.c_str());
                
                // Add to history
                std::string urlStr(target.begin(), target.end());
                HistoryManager::AddEntry(urlStr);
            }
        }
        ImGui::End();

        // --- BROWSER VIEWPORT MANAGEMENT ---
        if (webviewController != nullptr) {
            // Hide browser if Taskbar OR Settings are open to avoid clipping issues
            if (g_ShowTaskbarWindow || g_ShowSettingsWindow) {
                webviewController->put_IsVisible(FALSE);
            } else {
                webviewController->put_IsVisible(TRUE);
                RECT webBounds = { 0, (LONG)topH, (LONG)winW, (LONG)(winH - botH) };
                webviewController->put_Bounds(webBounds);
            }
        }

        // Render Settings Window overlay
        RenderSettingsWindow();

        // --- FINAL RENDERING ---
        ImGui::Render();
        const float clear_color[4] = { 0.12f, 0.12f, 0.12f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0); 
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// --- DX11 Helper Functions ---
bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    UINT flags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL fl[] = { D3D_FEATURE_LEVEL_11_0 };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, fl, 1, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;
    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND: if ((wParam & 0xfff0) == SC_KEYMENU) return 0; break;
    case WM_DESTROY: ::PostQuitMessage(0); return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}