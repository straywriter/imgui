#pragma once


#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

IMGUI_API void ProgressBar(float fraction, const ImVec2 &size_arg = ImVec2(-FLT_MIN, 0), const char *overlay = NULL);
IMGUI_API void Bullet(); // draw a small circle + keep the cursor on the same line. advance cursor x position by
                         // GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses

// Widgets: Value() Helpers.
// - Those are merely shortcut to calling Text() with a format string. Output single value in "name: value" format (tip:
// freely declare more in your code to handle your types. you can add functions to the ImGui namespace)
IMGUI_API void Value(const char *prefix, bool b);
IMGUI_API void Value(const char *prefix, int v);
IMGUI_API void Value(const char *prefix, unsigned int v);
IMGUI_API void Value(const char *prefix, float v, const char *float_format = NULL);


// Tooltips
// - Tooltip are windows following the mouse. They do not take focus away.
IMGUI_API void BeginTooltip(); // begin/append a tooltip window. to create full-featured tooltip (with any kind of
                               // items).
IMGUI_API void EndTooltip();
IMGUI_API void SetTooltip(const char *fmt, ...)
    IM_FMTARGS(1); // set a text-only tooltip, typically use with ImGui::IsItemHovered(). override any previous call to
                   // SetTooltip().
IMGUI_API void SetTooltipV(const char *fmt, va_list args) IM_FMTLIST(1);


// Tab Bars, Tabs
IMGUI_API bool BeginTabBar(const char *str_id, ImGuiTabBarFlags flags = 0); // create and append into a TabBar
IMGUI_API void EndTabBar(); // only call EndTabBar() if BeginTabBar() returns true!
IMGUI_API bool BeginTabItem(const char *label, bool *p_open = NULL,
                            ImGuiTabItemFlags flags = 0); // create a Tab. Returns true if the Tab is selected.
IMGUI_API void EndTabItem();                              // only call EndTabItem() if BeginTabItem() returns true!
IMGUI_API bool TabItemButton(const char *label,
                             ImGuiTabItemFlags flags = 0); // create a Tab behaving like a button. return true when
                                                           // clicked. cannot be selected in the tab bar.
IMGUI_API void SetTabItemClosed(
    const char
        *tab_or_docked_window_label); // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce
                                      // visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar()
                                      // and before Tab submissions. Otherwise call with a window name.

// Logging/Capture
// - All text output from the interface can be captured into tty/file/clipboard. By default, tree nodes are
// automatically opened during logging.
IMGUI_API void LogToTTY(int auto_open_depth = -1);                               // start logging to tty (stdout)
IMGUI_API void LogToFile(int auto_open_depth = -1, const char *filename = NULL); // start logging to file
IMGUI_API void LogToClipboard(int auto_open_depth = -1);                         // start logging to OS clipboard
IMGUI_API void LogFinish();                                                      // stop logging (close file, etc.)
IMGUI_API void LogButtons(); // helper to display buttons for logging to tty/file/clipboard
IMGUI_API void LogText(const char *fmt, ...) IM_FMTARGS(1); // pass text data straight to log (without being displayed)
IMGUI_API void LogTextV(const char *fmt, va_list args) IM_FMTLIST(1);

// Clipping
// - Mouse hovering is affected by ImGui::PushClipRect() calls, unlike direct calls to ImDrawList::PushClipRect() which
// are render only.
IMGUI_API void PushClipRect(const ImVec2 &clip_rect_min, const ImVec2 &clip_rect_max,
                            bool intersect_with_current_clip_rect);
IMGUI_API void PopClipRect();

// Focus, Activation
// - Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing()) SetScrollHereY()" when applicable to signify
// "this is the default item"
IMGUI_API void SetItemDefaultFocus(); // make last item the default focused item of a window.
IMGUI_API void SetKeyboardFocusHere(
    int offset = 0); // focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple
                     // component widget. Use -1 to access previous widget.

// Item/Widgets Utilities and Query Functions
// - Most of the functions are referring to the previous Item that has been submitted.
// - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of those functions.
IMGUI_API bool IsItemHovered(ImGuiHoveredFlags flags = 0); // is the last item hovered? (and usable, aka not blocked by
                                                           // a popup, etc.). See ImGuiHoveredFlags for more options.
IMGUI_API bool IsItemActive();  // is the last item active? (e.g. button being held, text field being edited. This will
                                // continuously return true while holding mouse button on an item. Items that don't
                                // interact will always return false)
IMGUI_API bool IsItemFocused(); // is the last item focused for keyboard/gamepad navigation?
IMGUI_API bool IsItemClicked(
    ImGuiMouseButton mouse_button =
        0); // is the last item hovered and mouse clicked on? (**)  == IsMouseClicked(mouse_button) &&
            // IsItemHovered()Important. (**) this it NOT equivalent to the behavior of e.g. Button(). Read comments in
            // function definition.
IMGUI_API bool IsItemVisible();   // is the last item visible? (items may be out of sight because of clipping/scrolling)
IMGUI_API bool IsItemEdited();    // did the last item modify its underlying value this frame? or was pressed? This is
                                  // generally the same as the "bool" return value of many widgets.
IMGUI_API bool IsItemActivated(); // was the last item just made active (item was previously inactive).
IMGUI_API bool IsItemDeactivated(); // was the last item just made inactive (item was previously active). Useful for
                                    // Undo/Redo patterns with widgets that requires continuous editing.
IMGUI_API bool IsItemDeactivatedAfterEdit(); // was the last item just made inactive and made a value change when it was
                                             // active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with
                                             // widgets that requires continuous editing. Note that you may get false
                                             // positives (some widgets such as Combo()/ListBox()/Selectable() will
                                             // return true even when clicking an already selected item).
IMGUI_API bool IsItemToggledOpen();          // was the last item open state toggled? set by TreeNode().
IMGUI_API bool IsAnyItemHovered();           // is any item hovered?
IMGUI_API bool IsAnyItemActive();            // is any item active?
IMGUI_API bool IsAnyItemFocused();           // is any item focused?
IMGUI_API ImVec2 GetItemRectMin();           // get upper-left bounding rectangle of the last item (screen space)
IMGUI_API ImVec2 GetItemRectMax();           // get lower-right bounding rectangle of the last item (screen space)
IMGUI_API ImVec2 GetItemRectSize();          // get size of last item
IMGUI_API void SetItemAllowOverlap(); // allow last item to be overlapped by a subsequent item. sometimes useful with
                                      // invisible buttons, selectables, etc. to catch unused area.


} // namespace ImGui