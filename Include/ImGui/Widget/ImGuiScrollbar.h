#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

// Scrolling
IMGUI_API void SetNextWindowScroll(const ImVec2 &scroll); // Use -1.0f on one axis to leave as-is

IMGUI_API void SetScrollX(ImGuiWindow *window, float scroll_x);
IMGUI_API void SetScrollY(ImGuiWindow *window, float scroll_y);
IMGUI_API void SetScrollFromPosX(ImGuiWindow *window, float local_x, float center_x_ratio);
IMGUI_API void SetScrollFromPosY(ImGuiWindow *window, float local_y, float center_y_ratio);
IMGUI_API ImVec2 ScrollToBringRectIntoView(ImGuiWindow *window, const ImRect &item_rect);

IMGUI_API void Scrollbar(ImGuiAxis axis);
IMGUI_API bool ScrollbarEx(const ImRect &bb, ImGuiID id, ImGuiAxis axis, float *p_scroll_v, float avail_v,
                           float contents_v, ImDrawFlags flags);

IMGUI_API ImRect GetWindowScrollbarRect(ImGuiWindow *window, ImGuiAxis axis);

IMGUI_API ImGuiID GetWindowScrollbarID(ImGuiWindow *window, ImGuiAxis axis);

} // namespace ImGui