#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{


// Miscellaneous Utilities
IMGUI_API bool IsRectVisible(
    const ImVec2 &size); // test if rectangle (of given size, starting from cursor position) is visible / not clipped.
IMGUI_API bool IsRectVisible(const ImVec2 &rect_min,
                             const ImVec2 &rect_max); // test if rectangle (in screen space) is visible / not clipped.
                                                      // to perform coarse clipping on user's side.
IMGUI_API double GetTime();                           // get global imgui time. incremented by io.DeltaTime every frame.
IMGUI_API int GetFrameCount();                        // get global imgui frame count. incremented by 1 every frame.
IMGUI_API ImDrawList *GetBackgroundDrawList(); // this draw list will be the first rendering one. Useful to quickly draw
                                               // shapes/text behind dear imgui contents.
IMGUI_API ImDrawList *GetForegroundDrawList(); // this draw list will be the last rendered one. Useful to quickly draw
                                               // shapes/text over dear imgui contents.
IMGUI_API ImDrawListSharedData *GetDrawListSharedData(); // you may use this when creating your own ImDrawList
                                                         // instances.
IMGUI_API const char *GetStyleColorName(
    ImGuiCol idx); // get a string corresponding to the enum value (for display, saving, etc.).
IMGUI_API void SetStateStorage(ImGuiStorage *storage); // replace current window storage with our own (if you want to
                                                       // manipulate it yourself, typically clear subsection of it)
IMGUI_API ImGuiStorage *GetStateStorage();
IMGUI_API void CalcListClipping(
    int items_count, float items_height, int *out_items_display_start,
    int *out_items_display_end); // calculate coarse clipping for large list of evenly sized items. Prefer using the
                                 // ImGuiListClipper higher-level helper if you can.
IMGUI_API bool BeginChildFrame(ImGuiID id, const ImVec2 &size,
                               ImGuiWindowFlags flags = 0); // helper to create a child window / scrolling region that
                                                            // looks like a normal widget frame
IMGUI_API void EndChildFrame(); // always call EndChildFrame() regardless of BeginChildFrame() return values (which
                                // indicates a collapsed/clipped window)

// Text Utilities
IMGUI_API ImVec2 CalcTextSize(const char *text, const char *text_end = NULL, bool hide_text_after_double_hash = false,
                              float wrap_width = -1.0f);

// Color Utilities
IMGUI_API ImVec4 ColorConvertU32ToFloat4(ImU32 in);
IMGUI_API ImU32 ColorConvertFloat4ToU32(const ImVec4 &in);
IMGUI_API void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v);
IMGUI_API void ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b);

// Inputs Utilities: Keyboard
// - For 'int user_key_index' you can use your own indices/enums according to how your backend/engine stored them in
// io.KeysDown[].
// - We don't know the meaning of those value. You can use GetKeyIndex() to map a ImGuiKey_ value into the user index.
IMGUI_API int GetKeyIndex(ImGuiKey imgui_key); // map ImGuiKey_* values into user's key index. == io.KeyMap[key]
IMGUI_API bool IsKeyDown(int user_key_index);  // is key being held. == io.KeysDown[user_key_index].
IMGUI_API bool IsKeyPressed(int user_key_index,
                            bool repeat = true);  // was key pressed (went from !Down to Down)? if repeat=true, uses
                                                  // io.KeyRepeatDelay / KeyRepeatRate
IMGUI_API bool IsKeyReleased(int user_key_index); // was key released (went from Down to !Down)?
IMGUI_API int GetKeyPressedAmount(int key_index, float repeat_delay,
                                  float rate); // uses provided repeat rate/delay. return a count, most often 0 or 1 but
                                               // might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate
IMGUI_API void CaptureKeyboardFromApp(
    bool want_capture_keyboard_value =
        true); // attention: misleading name! manually override io.WantCaptureKeyboard flag next frame (said flag is
               // entirely left for your application to handle). e.g. force capture keyboard when your widget is being
               // hovered. This is equivalent to setting "io.WantCaptureKeyboard = want_capture_keyboard_value"; after
               // the next NewFrame() call.

// Inputs Utilities: Mouse
// - To refer to a mouse button, you may use named enums in your code e.g. ImGuiMouseButton_Left,
// ImGuiMouseButton_Right.
// - You can also use regular integer: it is forever guaranteed that 0=Left, 1=Right, 2=Middle.
// - Dragging operations are only reported after mouse has moved a certain distance away from the initial clicking
// position (see 'lock_threshold' and 'io.MouseDraggingThreshold')
IMGUI_API bool IsMouseDown(ImGuiMouseButton button); // is mouse button held?
IMGUI_API bool IsMouseClicked(ImGuiMouseButton button,
                              bool repeat = false);           // did mouse button clicked? (went from !Down to Down)
IMGUI_API bool IsMouseReleased(ImGuiMouseButton button);      // did mouse button released? (went from Down to !Down)
IMGUI_API bool IsMouseDoubleClicked(ImGuiMouseButton button); // did mouse button double-clicked? (note that a
                                                              // double-click will also report IsMouseClicked() == true)
IMGUI_API bool IsMouseHoveringRect(
    const ImVec2 &r_min, const ImVec2 &r_max,
    bool clip = true); // is mouse hovering given bounding rect (in screen space). clipped by current clipping settings,
                       // but disregarding of other consideration of focus/window ordering/popup-block.
IMGUI_API bool IsMousePosValid(const ImVec2 *mouse_pos = NULL); // by convention we use (-FLT_MAX,-FLT_MAX) to denote
                                                                // that there is no mouse available
IMGUI_API bool IsAnyMouseDown();                                // is any mouse button held?
IMGUI_API ImVec2
GetMousePos(); // shortcut to ImGui::GetIO().MousePos provided by user, to be consistent with other calls
IMGUI_API ImVec2
GetMousePosOnOpeningCurrentPopup(); // retrieve mouse position at the time of opening popup we have BeginPopup() into
                                    // (helper to avoid user backing that value themselves)
IMGUI_API bool IsMouseDragging(
    ImGuiMouseButton button,
    float lock_threshold = -1.0f); // is mouse dragging? (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
IMGUI_API ImVec2 GetMouseDragDelta(
    ImGuiMouseButton button = 0,
    float lock_threshold =
        -1.0f); // return the delta from the initial clicking position while the mouse button is pressed or was just
                // released. This is locked and return 0.0f until the mouse moves past a distance threshold at least
                // once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
IMGUI_API void ResetMouseDragDelta(ImGuiMouseButton button = 0); //
IMGUI_API ImGuiMouseCursor GetMouseCursor(); // get desired cursor type, reset in ImGui::NewFrame(), this is updated
                                             // during the frame. valid before Render(). If you use software rendering
                                             // by setting io.MouseDrawCursor ImGui will render those for you
IMGUI_API void SetMouseCursor(ImGuiMouseCursor cursor_type); // set desired cursor type
IMGUI_API void CaptureMouseFromApp(
    bool want_capture_mouse_value =
        true); // attention: misleading name! manually override io.WantCaptureMouse flag next frame (said flag is
               // entirely left for your application to handle). This is equivalent to setting "io.WantCaptureMouse =
               // want_capture_mouse_value;" after the next NewFrame() call.

// Clipboard Utilities
// - Also see the LogToClipboard() function to capture GUI into clipboard, or easily output text data to the clipboard.
IMGUI_API const char *GetClipboardText();
IMGUI_API void SetClipboardText(const char *text);

// Settings/.Ini Utilities
// - The disk functions are automatically called if io.IniFilename != NULL (default is "imgui.ini").
// - Set io.IniFilename to NULL to load/save manually. Read io.WantSaveIniSettings description about handling .ini
// saving manually.
IMGUI_API void LoadIniSettingsFromDisk(
    const char *ini_filename); // call after CreateContext() and before the first call to NewFrame(). NewFrame()
                               // automatically calls LoadIniSettingsFromDisk(io.IniFilename).
IMGUI_API void LoadIniSettingsFromMemory(
    const char *ini_data, size_t ini_size = 0); // call after CreateContext() and before the first call to NewFrame() to
                                                // provide .ini data from your own data source.
IMGUI_API void SaveIniSettingsToDisk(
    const char *ini_filename); // this is automatically called (if io.IniFilename is not empty) a few seconds after any
                               // modification that should be reflected in the .ini file (and also by DestroyContext).
IMGUI_API const char *SaveIniSettingsToMemory(
    size_t *out_ini_size =
        NULL); // return a zero-terminated string with the .ini data which you can save by your own mean. call when
               // io.WantSaveIniSettings is set, then save data by your own mean and clear io.WantSaveIniSettings.

// Debug Utilities
// - This is used by the IMGUI_CHECKVERSION() macro.
IMGUI_API bool DebugCheckVersionAndDataLayout(const char *version_str, size_t sz_io, size_t sz_style, size_t sz_vec2,
                                              size_t sz_vec4, size_t sz_drawvert,
                                              size_t sz_drawidx); // This is called by IMGUI_CHECKVERSION() macro.

// Memory Allocators
// - Those functions are not reliant on the current context.
// - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() +
// SetAllocatorFunctions()
//   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of imgui.cpp for
//   more details.
IMGUI_API void SetAllocatorFunctions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void *user_data = NULL);
IMGUI_API void GetAllocatorFunctions(ImGuiMemAllocFunc *p_alloc_func, ImGuiMemFreeFunc *p_free_func,
                                     void **p_user_data);
IMGUI_API void *MemAlloc(size_t size);
IMGUI_API void MemFree(void *ptr);


} // namespace ImGui