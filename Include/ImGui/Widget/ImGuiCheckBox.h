#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>


namespace ImGui
{

IMGUI_API bool Checkbox(const char *label, bool *v);
template <typename T> IMGUI_API bool CheckboxFlagsT(const char *label, T *flags, T flags_value);

IMGUI_API bool CheckboxFlags(const char *label, ImS64 *flags, ImS64 flags_value);
IMGUI_API bool CheckboxFlags(const char *label, ImU64 *flags, ImU64 flags_value);
IMGUI_API bool CheckboxFlags(const char *label, int *flags, int flags_value);
IMGUI_API bool CheckboxFlags(const char *label, unsigned int *flags, unsigned int flags_value);
} // namespace ImGui