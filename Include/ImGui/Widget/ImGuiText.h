#pragma once



#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

// Widgets: Text
IMGUI_API void TextUnformatted(
    const char *text,
    const char *text_end = NULL); // raw text without formatting. Roughly equivalent to Text("%s", text) but: A) doesn't
                                  // require null terminated string if 'text_end' is specified, B) it's faster, no
                                  // memory copy is done, no buffer size limits, recommended for long chunks of text.
IMGUI_API void Text(const char *fmt, ...) IM_FMTARGS(1); // formatted text
IMGUI_API void TextV(const char *fmt, va_list args) IM_FMTLIST(1);
IMGUI_API void TextColored(const ImVec4 &col, const char *fmt, ...)
    IM_FMTARGS(2); // shortcut for PushStyleColor(ImGuiCol_Text, col); Text(fmt, ...); PopStyleColor();
IMGUI_API void TextColoredV(const ImVec4 &col, const char *fmt, va_list args) IM_FMTLIST(2);
IMGUI_API void TextDisabled(const char *fmt, ...)
    IM_FMTARGS(1); // shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...);
                   // PopStyleColor();
IMGUI_API void TextDisabledV(const char *fmt, va_list args) IM_FMTLIST(1);
IMGUI_API void TextWrapped(const char *fmt, ...)
    IM_FMTARGS(1); // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work
                   // on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to
                   // set a size using SetNextWindowSize().
IMGUI_API void TextWrappedV(const char *fmt, va_list args) IM_FMTLIST(1);
IMGUI_API void LabelText(const char *label, const char *fmt, ...)
    IM_FMTARGS(2); // display text+label aligned the same way as value+label widgets
IMGUI_API void LabelTextV(const char *label, const char *fmt, va_list args) IM_FMTLIST(2);
IMGUI_API void BulletText(const char *fmt, ...) IM_FMTARGS(1); // shortcut for Bullet()+Text()
IMGUI_API void BulletTextV(const char *fmt, va_list args) IM_FMTLIST(1);

} // namespace ImGui