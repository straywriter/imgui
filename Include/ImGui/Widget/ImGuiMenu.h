#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

// Widgets: Menus
// - Use BeginMenuBar() on a window ImGuiWindowFlags_MenuBar to append to its menu bar.
// - Use BeginMainMenuBar() to create a menu bar at the top of the screen and append to it.
// - Use BeginMenu() to create a menu. You can call BeginMenu() multiple time with the same identifier to append more
// items to it.
// - Not that MenuItem() keyboardshortcuts are displayed as a convenience but _not processed_ by Dear ImGui at the
// moment.
IMGUI_API bool BeginMenuBar(); // append to menu-bar of current window (requires ImGuiWindowFlags_MenuBar flag set on
                               // parent window).
IMGUI_API void EndMenuBar();   // only call EndMenuBar() if BeginMenuBar() returns true!
IMGUI_API bool BeginMainMenuBar(); // create and append to a full screen menu-bar.
IMGUI_API void EndMainMenuBar();   // only call EndMainMenuBar() if BeginMainMenuBar() returns true!
IMGUI_API bool BeginMenu(const char *label,
                         bool enabled = true); // create a sub-menu entry. only call EndMenu() if this returns true!
IMGUI_API void EndMenu();                      // only call EndMenu() if BeginMenu() returns true!
IMGUI_API bool MenuItem(const char *label, const char *shortcut = NULL, bool selected = false,
                        bool enabled = true); // return true when activated.
IMGUI_API bool MenuItem(const char *label, const char *shortcut, bool *p_selected,
                        bool enabled = true); // return true when activated + toggle (*p_selected) if p_selected != NULL




} // namespace ImGui