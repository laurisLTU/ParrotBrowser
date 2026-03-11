#include "settings.h"
#include "imgui.h"
#include <fstream>
#include <string>
#include <WebView2.h>
#include <wrl.h>

using namespace Microsoft::WRL;

bool g_ShowSettingsWindow = false;
bool g_IsDarkMode = true;
ImVec4 g_AccentColor = ImVec4(0.20f, 0.80f, 0.20f, 1.00f);

extern ComPtr<ICoreWebView2> webviewWindow;

void SaveConfig() {
    std::ofstream file("config.ini");
    if (file.is_open()) {
        file << "DarkMode " << (g_IsDarkMode ? 1 : 0) << "\n";
        file << "ColorR " << g_AccentColor.x << "\n";
        file << "ColorG " << g_AccentColor.y << "\n";
        file << "ColorB " << g_AccentColor.z << "\n";
        file.close();
    }
}

void LoadConfig() {
    std::ifstream file("config.ini");
    if (!file.is_open()) return;

    std::string key;
    while (file >> key) {
        if (key == "DarkMode") file >> g_IsDarkMode;
        else if (key == "ColorR") file >> g_AccentColor.x;
        else if (key == "ColorG") file >> g_AccentColor.y;
        else if (key == "ColorB") file >> g_AccentColor.z;
    }
    file.close();
}

void ApplyCustomStyle() {
    if (g_IsDarkMode) ImGui::StyleColorsDark();
    else ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Button]         = g_AccentColor;
    style.Colors[ImGuiCol_ButtonHovered]  = ImVec4(g_AccentColor.x + 0.1f, g_AccentColor.y + 0.1f, g_AccentColor.z + 0.1f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive]   = ImVec4(g_AccentColor.x - 0.1f, g_AccentColor.y - 0.1f, g_AccentColor.z - 0.1f, 1.0f);
    style.Colors[ImGuiCol_Header]         = g_AccentColor;
    style.Colors[ImGuiCol_CheckMark]      = g_AccentColor;
    style.Colors[ImGuiCol_SliderGrab]     = g_AccentColor;
}

void RenderSettingsWindow() {
    if (!g_ShowSettingsWindow) return;

    if (ImGui::Begin("Settings", &g_ShowSettingsWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
        bool changed = false;

        ImGui::Text("Theme Settings");
        if (ImGui::Checkbox("Dark Mode", &g_IsDarkMode)) {
            changed = true;
        }

        ImGui::Separator();
        ImGui::Text("Accent Color");
        if (ImGui::ColorPicker4("##Picker", (float*)&g_AccentColor, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoAlpha)) {
            changed = true;
        }

        if (changed) {
            ApplyCustomStyle();
            SaveConfig(); // Save every time something changes
        }
    }
    ImGui::End();
}