#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{


IMGUI_API bool Button(const char *label, const ImVec2 &size = ImVec2(0, 0)); // button
IMGUI_API bool SmallButton(const char *label); // button with FramePadding=(0,0) to easily embed within text
IMGUI_API bool InvisibleButton(
    const char *str_id, const ImVec2 &size,
    ImGuiButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom
                                 // behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
IMGUI_API bool ArrowButton(const char *str_id, ImGuiDir dir); // square button with an arrow shape

IMGUI_API bool RadioButton(const char *label,
                           bool active); // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
IMGUI_API bool RadioButton(const char *label, int *v,
                           int v_button); // shortcut to handle the above pattern when value is an integer

} // namespace ImGui
