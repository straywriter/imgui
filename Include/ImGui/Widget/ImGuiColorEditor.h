#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{



// Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little color square that can be left-clicked to
// open a picker, and right-clicked to open an option menu.)
// - Note that in C++ a 'float v[X]' function argument is the _same_ as 'float* v', the array syntax is just a way to
// document the number of elements that are expected to be accessible.
// - You can pass the address of a first float element out of a contiguous structure, e.g. &myvector.x
IMGUI_API bool ColorEdit3(const char *label, float col[3], ImGuiColorEditFlags flags = 0);
IMGUI_API bool ColorEdit4(const char *label, float col[4], ImGuiColorEditFlags flags = 0);
IMGUI_API bool ColorPicker3(const char *label, float col[3], ImGuiColorEditFlags flags = 0);
IMGUI_API bool ColorPicker4(const char *label, float col[4], ImGuiColorEditFlags flags = 0,
                            const float *ref_col = NULL);


IMGUI_API void SetColorEditOptions(
    ImGuiColorEditFlags flags); // initialize current options (generally on application startup) if you want to select a
                                // default format, picker type, etc. User will be able to change many settings, unless
                                // you pass the _NoOptions flag to your calls.

} // namespace ImGui