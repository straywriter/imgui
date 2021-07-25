#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

// Popups, Modals
//  - They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
//  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
//  - Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with
//  regular Begin*() calls.
//  - The 3 properties above are related: we need to retain popup visibility state in the library because popups may be
//  closed as any time.
//  - You can bypass the hovering restriction by using ImGuiHoveredFlags_AllowWhenBlockedByPopup when calling
//  IsItemHovered() or IsWindowHovered().
//  - IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and BeginPopup generally needs to
//  be at the same level of the stack.
//    This is sometimes leading to confusing mistakes. May rework this in the future.
// Popups: begin/end functions
//  - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup() afterwards.
//  ImGuiWindowFlags are forwarded to the window.
//  - BeginPopupModal(): block every interactions behind the window, cannot be closed by user, add a dimming background,
//  has a title bar.
IMGUI_API bool BeginPopup(
    const char *str_id,
    ImGuiWindowFlags flags = 0); // return true if the popup is open, and you can start outputting to it.
IMGUI_API bool BeginPopupModal(
    const char *name, bool *p_open = NULL,
    ImGuiWindowFlags flags = 0); // return true if the modal is open, and you can start outputting to it.
IMGUI_API void EndPopup();       // only call EndPopup() if BeginPopupXXX() returns true!
// Popups: open/close functions
//  - OpenPopup(): set popup state to open. ImGuiPopupFlags are available for opening options.
//  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
//  - CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close manually.
//  - CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activated (FIXME: need some options).
//  - Use ImGuiPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level.
//  This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to OpenPopup().
//  - Use IsWindowAppearing() after BeginPopup() to tell if a window just opened.
IMGUI_API void OpenPopup(const char *str_id,
                         ImGuiPopupFlags popup_flags = 0); // call to mark popup as open (don't call every frame!).
IMGUI_API void OpenPopup(ImGuiID id,
                         ImGuiPopupFlags popup_flags = 0); // id overload to facilitate calling from nested stacks
IMGUI_API void OpenPopupOnItemClick(
    const char *str_id = NULL,
    ImGuiPopupFlags popup_flags =
        1); // helper to open popup when clicked on last item. Default to ImGuiPopupFlags_MouseButtonRight == 1. (note:
            // actually triggers on the mouse _released_ event to be consistent with popup behaviors)
IMGUI_API void CloseCurrentPopup(); // manually close the popup we have begin-ed into.
// Popups: open+begin combined functions helpers
//  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
//  - They are convenient to easily create context menus, hence the name.
//  - IMPORTANT: Notice that BeginPopupContextXXX takes ImGuiPopupFlags just like OpenPopup() and unlike BeginPopup().
//  For full consistency, we may add ImGuiWindowFlags to the BeginPopupContextXXX functions in the future.
//  - IMPORTANT: we exceptionally default their flags to 1 (== ImGuiPopupFlags_MouseButtonRight) for backward
//  compatibility with older API taking 'int mouse_button = 1' parameter, so if you add other flags remember to re-add
//  the ImGuiPopupFlags_MouseButtonRight.
IMGUI_API bool BeginPopupContextItem(
    const char *str_id = NULL,
    ImGuiPopupFlags popup_flags = 1); // open+begin popup when clicked on last item. Use str_id==NULL to associate the
                                      // popup to previous item. If you want to use that on a non-interactive item such
                                      // as Text() you need to pass in an explicit ID here. read comments in .cpp!
IMGUI_API bool BeginPopupContextWindow(
    const char *str_id = NULL, ImGuiPopupFlags popup_flags = 1); // open+begin popup when clicked on current window.
IMGUI_API bool BeginPopupContextVoid(
    const char *str_id = NULL,
    ImGuiPopupFlags popup_flags = 1); // open+begin popup when clicked in void (where there are no windows).
// Popups: query functions
//  - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the popup stack.
//  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level
//  of the popup stack.
//  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId + ImGuiPopupFlags_AnyPopupLevel: return true if any popup is open.
IMGUI_API bool IsPopupOpen(const char *str_id, ImGuiPopupFlags flags = 0); // return true if the popup is open.

IMGUI_API void OpenPopupEx(ImGuiID id, ImGuiPopupFlags popup_flags = ImGuiPopupFlags_None);
IMGUI_API void ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup);
IMGUI_API void ClosePopupsOverWindow(ImGuiWindow *ref_window, bool restore_focus_to_window_under_popup);
IMGUI_API bool IsPopupOpen(ImGuiID id, ImGuiPopupFlags popup_flags);
IMGUI_API bool BeginPopupEx(ImGuiID id, ImGuiWindowFlags extra_flags);
IMGUI_API void BeginTooltipEx(ImGuiWindowFlags extra_flags, ImGuiTooltipFlags tooltip_flags);
IMGUI_API ImGuiWindow *GetTopMostPopupModal();
IMGUI_API ImVec2 FindBestWindowPosForPopup(ImGuiWindow *window);
IMGUI_API ImVec2 FindBestWindowPosForPopupEx(const ImVec2 &ref_pos, const ImVec2 &size, ImGuiDir *last_dir,
                                             const ImRect &r_outer, const ImRect &r_avoid,
                                             ImGuiPopupPositionPolicy policy);



} // namespace ImGui