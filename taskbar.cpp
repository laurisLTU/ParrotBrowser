#include "taskbar.h"
#include "imgui.h"
#include "settings.h"
#include "history.h"
#include <string>
#include <WebView2.h>
#include <wrl.h>

using namespace Microsoft::WRL;

bool g_ShowTaskbarWindow = false;
extern ComPtr<ICoreWebView2> webviewWindow;
extern ComPtr<ICoreWebView2Controller> webviewController;

void RenderTaskbarWindow(float windowWidth, float windowHeight) {
    if (!g_ShowTaskbarWindow) {
        if (webviewController && !g_ShowSettingsWindow) webviewController->put_IsVisible(TRUE);
        return;
    }

    // Hide WebView while this window is open to prevent "Airspace" clipping
    if (webviewController) webviewController->put_IsVisible(FALSE);

    // Position the Taskbar window on the right side
    ImGui::SetNextWindowPos(ImVec2(windowWidth - 320.0f, 42.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300.0f, windowHeight - 100.0f), ImGuiCond_Always);

    if (ImGui::Begin("Parrot Taskbar", &g_ShowTaskbarWindow, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        
        if (ImGui::Button("Open Settings", ImVec2(-1, 40))) {
            g_ShowSettingsWindow = true;
            g_ShowTaskbarWindow = false; // Close taskbar when settings open
        }

        ImGui::Separator();
        ImGui::Text("Recent History");
        ImGui::BeginChild("HistoryList", ImVec2(0, 0), true);
        
        auto& history = HistoryManager::GetHistory();
        if (history.empty()) {
            ImGui::TextDisabled("No history yet.");
        } else {
            for (int i = (int)history.size() - 1; i >= 0; i--) {
                if (ImGui::Selectable(history[i].c_str())) {
                    std::wstring target(history[i].begin(), history[i].end());
                    if (webviewWindow) webviewWindow->Navigate(target.c_str());
                    g_ShowTaskbarWindow = false; // Close after clicking a link
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}