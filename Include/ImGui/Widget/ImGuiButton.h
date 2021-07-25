#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

IMGUI_API bool Button(const char *label, const ImVec2 &size = ImVec2(0, 0)); // button

IMGUI_API bool ButtonEx(const char *label, const ImVec2 &size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = 0);

IMGUI_API bool SmallButton(const char *label); // button with FramePadding=(0,0) to easily embed within text

IMGUI_API bool InvisibleButton(
    const char *str_id, const ImVec2 &size,
    ImGuiButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom
                                 // behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
IMGUI_API bool CloseButton(ImGuiID id, const ImVec2 &pos);

IMGUI_API bool CollapseButton(ImGuiID id, const ImVec2 &pos);

IMGUI_API bool ArrowButton(const char *str_id, ImGuiDir dir); // square button with an arrow shape

IMGUI_API bool ArrowButtonEx(const char *str_id, ImGuiDir dir, ImVec2 size_arg, ImGuiButtonFlags flags = 0);

IMGUI_API bool RadioButton(const char *label,
                           bool active); // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
IMGUI_API bool RadioButton(const char *label, int *v,
                           int v_button); // shortcut to handle the above pattern when value is an integer

IMGUI_API bool ImageButtonEx(ImGuiID id, ImTextureID texture_id, const ImVec2 &size, const ImVec2 &uv0,
                             const ImVec2 &uv1, const ImVec2 &padding, const ImVec4 &bg_col, const ImVec4 &tint_col);

IMGUI_API bool ImageButton(ImTextureID user_texture_id, const ImVec2 &size, const ImVec2 &uv0 = ImVec2(0, 0),
                           const ImVec2 &uv1 = ImVec2(1, 1), int frame_padding = -1,
                           const ImVec4 &bg_col = ImVec4(0, 0, 0, 0), const ImVec4 &tint_col = ImVec4(1, 1, 1, 1));
// <0 frame_padding uses default frame padding settings. 0 for no padding

IMGUI_API bool ColorButton(
    const char *desc_id, const ImVec4 &col, ImGuiColorEditFlags flags = 0,
    ImVec2 size = ImVec2(0, 0)); // display a color square/button, hover for details, return true when pressed.


IMGUI_API bool ButtonBehavior(const ImRect &bb, ImGuiID id, bool *out_hovered, bool *out_held,
                              ImGuiButtonFlags flags = 0);

} // namespace ImGui
