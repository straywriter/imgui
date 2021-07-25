#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>


namespace ImGui
{

IMGUI_API bool Checkbox(const char *label, bool *v);
IMGUI_API bool CheckboxFlags(const char *label, int *flags, int flags_value);
IMGUI_API bool CheckboxFlags(const char *label, unsigned int *flags, unsigned int flags_value);
} // namespace ImGui