#pragma once

#include <float.h>                  // FLT_MIN, FLT_MAX
#include <stdarg.h>                 // va_list, va_start, va_end
#include <stddef.h>                 // ptrdiff_t, NULL
#include <string.h>                 // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp


#include <ImGui/ImGuiDefine.h>

#include <ImGui/ImGuiType.h>

#include <ImGui/ImGuiWindow.h>
//-----------------------------------------------------------------------------
// [SECTION] Dear ImGui end-user API functions
// (Note that ImGui:: being a namespace, you can add extra ImGui:: functions in your own separate file. Please don't
// modify imgui source files!)
//-----------------------------------------------------------------------------

namespace ImGui
{

    
// Context creation and access
// - Each context create its own ImFontAtlas by default. You may instance one yourself and pass it to CreateContext() to
// share a font atlas between contexts.
// - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() +
// SetAllocatorFunctions()
//   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of imgui.cpp for
//   details.

/**
 * Create a Context object
 * 
 * @param shared_font_atlas 
 * @return ImGuiContext * 
 */
IMGUI_API ImGuiContext *CreateContext(ImFontAtlas *shared_font_atlas = NULL);

/**
 * 
 * 
 * @param ctx 
 */
IMGUI_API void DestroyContext(ImGuiContext *ctx = NULL); // NULL = destroy current context

/**
 * Get the Current Context object
 * 
 * @return IMGUI_API* 
 */
IMGUI_API ImGuiContext *GetCurrentContext();

/**
 * Set the Current Context object
 * 
 * @param ctx 
 */
IMGUI_API void SetCurrentContext(ImGuiContext *ctx);

// Main
IMGUI_API ImGuiIO &GetIO();       // access the IO structure (mouse/keyboard/gamepad inputs, time, various configuration
                                  // options/flags)
IMGUI_API ImGuiStyle &GetStyle(); // access the Style structure (colors, sizes). Always use PushStyleCol(),
                                  // PushStyleVar() to modify style mid-frame!
IMGUI_API void NewFrame();        // start a new Dear ImGui frame, you can submit any command from this point until
                                  // Render()/EndFrame().
IMGUI_API void EndFrame(); // ends the Dear ImGui frame. automatically called by Render(). If you don't need to render
                           // data (skipping rendering) you may call EndFrame() without Render()... but you'll have
                           // wasted CPU already! If you don't need to render, better to not create any windows and not
                           // call NewFrame() at all!
IMGUI_API void Render();   // ends the Dear ImGui frame, finalize the draw data. You can then get call GetDrawData().
IMGUI_API ImDrawData *GetDrawData(); // valid after Render() and until the next call to NewFrame(). this is what you
                                     // have to render.

// Demo, Debug, Information
IMGUI_API void ShowDemoWindow(
    bool *p_open = NULL); // create Demo window. demonstrate most ImGui features. call this to learn about the library!
                          // try to make it always available in your application!
IMGUI_API void ShowMetricsWindow(bool *p_open = NULL); // create Metrics/Debugger window. display Dear ImGui internals:
                                                       // windows, draw commands, various internal state, etc.
IMGUI_API void ShowAboutWindow(
    bool *p_open = NULL); // create About window. display Dear ImGui version, credits and build/system information.
IMGUI_API void ShowStyleEditor(
    ImGuiStyle *ref = NULL); // add style editor block (not a window). you can pass in a reference ImGuiStyle structure
                             // to compare to, revert to and save to (else it uses the default style)
IMGUI_API bool ShowStyleSelector(
    const char *label); // add style selector block (not a window), essentially a combo listing the default styles.
IMGUI_API void ShowFontSelector(
    const char *label);         // add font selector block (not a window), essentially a combo listing the loaded fonts.
IMGUI_API void ShowUserGuide(); // add basic help/info block (not a window): how to manipulate ImGui as a end-user
                                // (mouse/keyboard controls).
IMGUI_API const char *GetVersion(); // get the compiled version string e.g. "1.80 WIP" (essentially the value for
                                    // IMGUI_VERSION from the compiled version of imgui.cpp)

// // Styles
// IMGUI_API void StyleColorsDark(ImGuiStyle *dst = NULL);    // new, recommended style (default)
// IMGUI_API void StyleColorsLight(ImGuiStyle *dst = NULL);   // best used with borders and a custom, thicker font
// IMGUI_API void StyleColorsClassic(ImGuiStyle *dst = NULL); // classic imgui style

// Windows
// - Begin() = push window to the stack and start appending to it. End() = pop window from the stack.
// - Passing 'bool* p_open != NULL' shows a window-closing widget in the upper-right corner of the window,
//   which clicking will set the boolean to false when clicked.
// - You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple
// times.
//   Some information such as 'flags' or 'p_open' will only be considered by the first call to Begin().
// - Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
//   anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
//   [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu,
//    BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function
//    returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
// - Note that the bottom of window stack always contains a window called "Debug".
IMGUI_API bool Begin(const char *name, bool *p_open = NULL, ImGuiWindowFlags flags = 0);

/**
 * 
 */
IMGUI_API void End();

// Child Windows
// - Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window. Child
// windows can embed their own child.
// - For each independent axis of 'size': ==0.0f: use remaining host window size / >0.0f: fixed size / <0.0f: use
// remaining window size minus abs(size) / Each axis can use a different mode, e.g. ImVec2(0,400).
// - BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may early out and omit
// submitting anything to the window.
//   Always call a matching EndChild() for each BeginChild() call, regardless of its return value.
//   [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu,
//    BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function
//    returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
IMGUI_API bool BeginChild(const char *str_id, const ImVec2 &size = ImVec2(0, 0), bool border = false,
                          ImGuiWindowFlags flags = 0);
IMGUI_API bool BeginChild(ImGuiID id, const ImVec2 &size = ImVec2(0, 0), bool border = false,
                          ImGuiWindowFlags flags = 0);
IMGUI_API void EndChild();


// Content region
// - Retrieve available space from a given point. GetContentRegionAvail() is frequently useful.
// - Those functions are bound to be redesigned (they are confusing, incomplete and the Min/Max return values are in
// local window coordinates which increases confusion)
IMGUI_API ImVec2 GetContentRegionAvail(); // == GetContentRegionMax() - GetCursorPos()
IMGUI_API ImVec2 GetContentRegionMax(); // current content boundaries (typically window boundaries including scrolling,
                                        // or current column boundaries), in windows coordinates
IMGUI_API ImVec2 GetWindowContentRegionMin();  // content boundaries min (roughly (0,0)-Scroll), in window coordinates
IMGUI_API ImVec2 GetWindowContentRegionMax();  // content boundaries max (roughly (0,0)+Size-Scroll) where Size can be
                                               // override with SetNextWindowContentSize(), in window coordinates
IMGUI_API float GetWindowContentRegionWidth(); //

// Windows Scrolling
IMGUI_API float GetScrollX();              // get scrolling amount [0 .. GetScrollMaxX()]
IMGUI_API float GetScrollY();              // get scrolling amount [0 .. GetScrollMaxY()]
IMGUI_API void SetScrollX(float scroll_x); // set scrolling amount [0 .. GetScrollMaxX()]
IMGUI_API void SetScrollY(float scroll_y); // set scrolling amount [0 .. GetScrollMaxY()]
IMGUI_API float GetScrollMaxX(); // get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x
IMGUI_API float GetScrollMaxY(); // get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y
IMGUI_API void SetScrollHereX(
    float center_x_ratio = 0.5f); // adjust scrolling amount to make current cursor position visible.
                                  // center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a
                                  // "default/current item" visible, consider using SetItemDefaultFocus() instead.
IMGUI_API void SetScrollHereY(
    float center_y_ratio = 0.5f); // adjust scrolling amount to make current cursor position visible.
                                  // center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a
                                  // "default/current item" visible, consider using SetItemDefaultFocus() instead.
IMGUI_API void SetScrollFromPosX(
    float local_x, float center_x_ratio = 0.5f); // adjust scrolling amount to make given position visible. Generally
                                                 // GetCursorStartPos() + offset to compute a valid position.
IMGUI_API void SetScrollFromPosY(
    float local_y, float center_y_ratio = 0.5f); // adjust scrolling amount to make given position visible. Generally
                                                 // GetCursorStartPos() + offset to compute a valid position.

// Parameters stacks (shared)
IMGUI_API void PushFont(ImFont *font); // use NULL as a shortcut to push default font
IMGUI_API void PopFont();
IMGUI_API void PushStyleColor(
    ImGuiCol idx, ImU32 col); // modify a style color. always use this if you modify the style after NewFrame().
IMGUI_API void PushStyleColor(ImGuiCol idx, const ImVec4 &col);
IMGUI_API void PopStyleColor(int count = 1);
IMGUI_API void PushStyleVar(
    ImGuiStyleVar idx,
    float val); // modify a style float variable. always use this if you modify the style after NewFrame().
IMGUI_API void PushStyleVar(
    ImGuiStyleVar idx,
    const ImVec2 &val); // modify a style ImVec2 variable. always use this if you modify the style after NewFrame().
IMGUI_API void PopStyleVar(int count = 1);
IMGUI_API void PushAllowKeyboardFocus(
    bool allow_keyboard_focus); // == tab stop enable. Allow focusing using TAB/Shift-TAB, enabled by default but you
                                // can disable it for certain widgets
IMGUI_API void PopAllowKeyboardFocus();
IMGUI_API void PushButtonRepeat(
    bool repeat); // in 'repeat' mode, Button*() functions return repeated true in a typematic manner (using
                  // io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call IsItemActive() after any
                  // Button() to tell if the button is held in the current frame.
IMGUI_API void PopButtonRepeat();

// Parameters stacks (current window)
IMGUI_API void PushItemWidth(
    float item_width); // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align
                       // xx pixels to the right of window (so -FLT_MIN always align width to the right side).
IMGUI_API void PopItemWidth();
IMGUI_API void SetNextItemWidth(
    float item_width); // set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align
                       // xx pixels to the right of window (so -FLT_MIN always align width to the right side)
IMGUI_API float CalcItemWidth(); // width of item given pushed settings and current cursor position. NOT necessarily the
                                 // width of last item unlike most 'Item' functions.
IMGUI_API void PushTextWrapPos(
    float wrap_local_pos_x =
        0.0f); // push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or
               // column); > 0.0f: wrap at 'wrap_pos_x' position in window local space
IMGUI_API void PopTextWrapPos();

// Style read access
IMGUI_API ImFont *GetFont();   // get current font
IMGUI_API float GetFontSize(); // get current font size (= height in pixels) of current font with current scale applied
IMGUI_API ImVec2
GetFontTexUvWhitePixel(); // get UV coordinate for a while pixel, useful to draw custom shapes via the ImDrawList API
IMGUI_API ImU32 GetColorU32(
    ImGuiCol idx, float alpha_mul = 1.0f); // retrieve given style color with style alpha applied and optional extra
                                           // alpha multiplier, packed as a 32-bit value suitable for ImDrawList
IMGUI_API ImU32 GetColorU32(const ImVec4 &col); // retrieve given color with style alpha applied, packed as a 32-bit
                                                // value suitable for ImDrawList
IMGUI_API ImU32 GetColorU32(
    ImU32 col); // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
IMGUI_API const ImVec4 &GetStyleColorVec4(
    ImGuiCol idx); // retrieve style color as stored in ImGuiStyle structure. use to feed back into PushStyleColor(),
                   // otherwise use GetColorU32() to get style color with style alpha baked in.

// Cursor / Layout
// - By "cursor" we mean the current output position.
// - The typical widget behavior is to output themselves at the current cursor position, then move the cursor one line
// down.
// - You can call SameLine() between widgets to undo the last carriage return and output at the right of the preceding
// widget.
// - Attention! We currently have inconsistencies between window-local and absolute positions we will aim to fix with
// future API:
//    Window-local coordinates:   SameLine(), GetCursorPos(), SetCursorPos(), GetCursorStartPos(),
//    GetContentRegionMax(), GetWindowContentRegion*(), PushTextWrapPos() Absolute coordinate: GetCursorScreenPos(),
//    SetCursorScreenPos(), all ImDrawList:: functions.
IMGUI_API void Separator(); // separator, generally horizontal. inside a menu bar or in horizontal layout mode, this
                            // becomes a vertical separator.
IMGUI_API void SameLine(float offset_from_start_x = 0.0f,
                        float spacing = -1.0f); // call between widgets or groups to layout them horizontally. X
                                                // position given in window coordinates.
IMGUI_API void NewLine();                 // undo a SameLine() or force a new line when in an horizontal-layout context.
IMGUI_API void Spacing();                 // add vertical spacing.
IMGUI_API void Dummy(const ImVec2 &size); // add a dummy item of given size. unlike InvisibleButton(), Dummy() won't
                                          // take the mouse click or be navigable into.
IMGUI_API void Indent(float indent_w = 0.0f);   // move content position toward the right, by indent_w, or
                                                // style.IndentSpacing if indent_w <= 0
IMGUI_API void Unindent(float indent_w = 0.0f); // move content position back to the left, by indent_w, or
                                                // style.IndentSpacing if indent_w <= 0
IMGUI_API void BeginGroup();                    // lock horizontal starting position
IMGUI_API void EndGroup(); // unlock horizontal starting position + capture the whole group bounding box into one "item"
                           // (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group,
                           // etc.)
IMGUI_API ImVec2 GetCursorPos(); // cursor position in window coordinates (relative to window position)
IMGUI_API float GetCursorPosX(); //   (some functions are using window-relative coordinates, such as: GetCursorPos,
                                 //   GetCursorStartPos, GetContentRegionMax, GetWindowContentRegion* etc.
IMGUI_API float GetCursorPosY(); //    other functions such as GetCursorScreenPos or everything in ImDrawList::
IMGUI_API void SetCursorPos(const ImVec2 &local_pos); //    are using the main, absolute coordinate system.
IMGUI_API void SetCursorPosX(float local_x); //    GetWindowPos() + GetCursorPos() == GetCursorScreenPos() etc.)
IMGUI_API void SetCursorPosY(float local_y); //
IMGUI_API ImVec2 GetCursorStartPos();        // initial cursor position in window coordinates
IMGUI_API ImVec2
GetCursorScreenPos(); // cursor position in absolute coordinates (useful to work with ImDrawList API). generally
                      // top-left == GetMainViewport()->Pos == (0,0) in single viewport mode, and bottom-right ==
                      // GetMainViewport()->Pos+Size == io.DisplaySize in single-viewport mode.
IMGUI_API void SetCursorScreenPos(const ImVec2 &pos); // cursor position in absolute coordinates
IMGUI_API void AlignTextToFramePadding(); // vertically align upcoming text baseline to FramePadding.y so that it will
                                          // align properly to regularly framed items (call if you have text on a line
                                          // before a framed item)
IMGUI_API float GetTextLineHeight();      // ~ FontSize
IMGUI_API float GetTextLineHeightWithSpacing(); // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2
                                                // consecutive lines of text)
IMGUI_API float GetFrameHeight();               // ~ FontSize + style.FramePadding.y * 2
IMGUI_API float GetFrameHeightWithSpacing(); // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in
                                             // pixels between 2 consecutive lines of framed widgets)

// ID stack/scopes
// - Read the FAQ for more details about how ID are handled in dear imgui. If you are creating widgets in a loop you
// most
//   likely want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
// - The resulting ID are hashes of the entire stack.
// - You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.
// - In this header file we use the "label"/"name" terminology to denote a string that will be displayed and used as an
// ID,
//   whereas "str_id" denote a string that is only used as an ID and not normally displayed.
IMGUI_API void PushID(const char *str_id); // push string into the ID stack (will hash string).
IMGUI_API void PushID(const char *str_id_begin,
                      const char *str_id_end); // push string into the ID stack (will hash string).
IMGUI_API void PushID(const void *ptr_id);     // push pointer into the ID stack (will hash pointer).
IMGUI_API void PushID(int int_id);             // push integer into the ID stack (will hash integer).
IMGUI_API void PopID();                        // pop from the ID stack.
IMGUI_API ImGuiID GetID(const char *str_id); // calculate unique ID (hash of whole ID stack + given parameter). e.g. if
                                             // you want to query into ImGuiStorage yourself
IMGUI_API ImGuiID GetID(const char *str_id_begin, const char *str_id_end);
IMGUI_API ImGuiID GetID(const void *ptr_id);

} // namespace ImGui