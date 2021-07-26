
#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

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




} // namespace ImGui