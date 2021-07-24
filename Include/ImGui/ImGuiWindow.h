#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

// Windows Utilities
// - 'current window' = the window we are appending into while inside a Begin()/End() block. 'next window' = next window
// we will Begin() into.
IMGUI_API bool IsWindowAppearing();

IMGUI_API bool IsWindowCollapsed();

IMGUI_API bool IsWindowFocused(ImGuiFocusedFlags flags = 0); // is current window focused? or its root/child, depending
                                                             // on flags. see flags for options.
IMGUI_API bool IsWindowHovered(
    ImGuiHoveredFlags flags =
        0); // is current window hovered (and typically: not blocked by a popup/modal)? see flags for options. NB: If
            // you are trying to check whether your mouse should be dispatched to imgui or to your app, you should use
            // the 'io.WantCaptureMouse' boolean for that! Please read the FAQ!
IMGUI_API ImDrawList *GetWindowDrawList(); // get draw list associated to the current window, to append your own drawing
                                           // primitives
IMGUI_API ImVec2 GetWindowPos();   // get current window position in screen space (useful if you want to do your own
                                   // drawing via the DrawList API)
IMGUI_API ImVec2 GetWindowSize();  // get current window size
IMGUI_API float GetWindowWidth();  // get current window width (shortcut for GetWindowSize().x)
IMGUI_API float GetWindowHeight(); // get current window height (shortcut for GetWindowSize().y)

// Prefer using SetNextXXX functions (before Begin) rather that SetXXX functions (after Begin).
IMGUI_API void SetNextWindowPos(
    const ImVec2 &pos, ImGuiCond cond = 0,
    const ImVec2 &pivot = ImVec2(
        0, 0)); // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
IMGUI_API void SetNextWindowSize(const ImVec2 &size,
                                 ImGuiCond cond = 0); // set next window size. set axis to 0.0f to force an auto-fit on
                                                      // this axis. call before Begin()
IMGUI_API void SetNextWindowSizeConstraints(
    const ImVec2 &size_min, const ImVec2 &size_max, ImGuiSizeCallback custom_callback = NULL,
    void *custom_callback_data =
        NULL); // set next window size limits. use -1,-1 on either X/Y axis to preserve the current size. Sizes will be
               // rounded down. Use callback to apply non-trivial programmatic constraints.
IMGUI_API void SetNextWindowContentSize(
    const ImVec2 &size); // set next window content size (~ scrollable client area, which enforce the range of
                         // scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding.
                         // set an axis to 0.0f to leave it automatic. call before Begin()
IMGUI_API void SetNextWindowCollapsed(bool collapsed,
                                      ImGuiCond cond = 0); // set next window collapsed state. call before Begin()
IMGUI_API void SetNextWindowFocus(); // set next window to be focused / top-most. call before Begin()
IMGUI_API void SetNextWindowBgAlpha(
    float alpha); // set next window background color alpha. helper to easily override the Alpha component of
                  // ImGuiCol_WindowBg/ChildBg/PopupBg. you may also use ImGuiWindowFlags_NoBackground.
IMGUI_API void SetWindowPos(
    const ImVec2 &pos,
    ImGuiCond cond = 0); // (not recommended) set current window position - call within Begin()/End(). prefer using
                         // SetNextWindowPos(), as this may incur tearing and side-effects.
IMGUI_API void SetWindowSize(
    const ImVec2 &size,
    ImGuiCond cond =
        0); // (not recommended) set current window size - call within Begin()/End(). set to ImVec2(0, 0) to force an
            // auto-fit. prefer using SetNextWindowSize(), as this may incur tearing and minor side-effects.
IMGUI_API void SetWindowCollapsed(
    bool collapsed,
    ImGuiCond cond = 0); // (not recommended) set current window collapsed state. prefer using SetNextWindowCollapsed().
IMGUI_API void SetWindowFocus(); // (not recommended) set current window to be focused / top-most. prefer using
                                 // SetNextWindowFocus().
IMGUI_API void SetWindowFontScale(
    float scale); // set font scale. Adjust IO.FontGlobalScale if you want to scale all windows. This is an old API! For
                  // correct scaling, prefer to reload font + rebuild ImFontAtlas + call style.ScaleAllSizes().
IMGUI_API void SetWindowPos(const char *name, const ImVec2 &pos, ImGuiCond cond = 0); // set named window position.
IMGUI_API void SetWindowSize(
    const char *name, const ImVec2 &size,
    ImGuiCond cond = 0); // set named window size. set axis to 0.0f to force an auto-fit on this axis.
IMGUI_API void SetWindowCollapsed(const char *name, bool collapsed,
                                  ImGuiCond cond = 0); // set named window collapsed state
IMGUI_API void SetWindowFocus(const char *name); // set named window to be focused / top-most. use NULL to remove focus.

} // namespace ImGui
