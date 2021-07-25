#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{


// Widgets: Selectables
// - A selectable highlights when hovered, and can display another color when selected.
// - Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of
// selected Selectable appear contiguous.
IMGUI_API bool Selectable(
    const char *label, bool selected = false, ImGuiSelectableFlags flags = 0,
    const ImVec2 &size =
        ImVec2(0, 0)); // "bool selected" carry the selection state (read-only). Selectable() is clicked is returns true
                       // so you can modify your selection state. size.x==0.0: use remaining width, size.x>0.0: specify
                       // width. size.y==0.0: use label height, size.y>0.0: specify height
IMGUI_API bool Selectable(const char *label, bool *p_selected, ImGuiSelectableFlags flags = 0,
                          const ImVec2 &size = ImVec2(0, 0)); // "bool* p_selected" point to the selection state
                                                              // (read-write), as a convenient helper.


} // namespace ImGui