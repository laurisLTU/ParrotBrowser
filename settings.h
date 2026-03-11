#pragma once
#include "imgui.h"

extern bool g_ShowSettingsWindow;
extern bool g_IsDarkMode;
extern ImVec4 g_AccentColor;

void RenderSettingsWindow();
void ApplyCustomStyle();

// New Save/Load functions
void SaveConfig();
void LoadConfig();