#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

// Widgets: Input with Keyboard
// - If you want to use InputText() with std::string or any custom dynamic string type, see misc/cpp/imgui_stdlib.h and
// comments in imgui_demo.cpp.
// - Most of the ImGuiInputTextFlags flags are only useful for InputText() and not for InputFloatX, InputIntX,
// InputDouble etc.
IMGUI_API bool InputText(const char *label, char *buf, size_t buf_size, ImGuiInputTextFlags flags = 0,
                         ImGuiInputTextCallback callback = NULL, void *user_data = NULL);
IMGUI_API bool InputTextMultiline(const char *label, char *buf, size_t buf_size, const ImVec2 &size = ImVec2(0, 0),
                                  ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
                                  void *user_data = NULL);
IMGUI_API bool InputTextWithHint(const char *label, const char *hint, char *buf, size_t buf_size,
                                 ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
                                 void *user_data = NULL);
IMGUI_API bool InputFloat(const char *label, float *v, float step = 0.0f, float step_fast = 0.0f,
                          const char *format = "%.3f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputFloat2(const char *label, float v[2], const char *format = "%.3f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputFloat3(const char *label, float v[3], const char *format = "%.3f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputFloat4(const char *label, float v[4], const char *format = "%.3f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputInt(const char *label, int *v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputInt2(const char *label, int v[2], ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputInt3(const char *label, int v[3], ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputInt4(const char *label, int v[4], ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputDouble(const char *label, double *v, double step = 0.0, double step_fast = 0.0,
                           const char *format = "%.6f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputScalar(const char *label, ImGuiDataType data_type, void *p_data, const void *p_step = NULL,
                           const void *p_step_fast = NULL, const char *format = NULL, ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputScalarN(const char *label, ImGuiDataType data_type, void *p_data, int components,
                            const void *p_step = NULL, const void *p_step_fast = NULL, const char *format = NULL,
                            ImGuiInputTextFlags flags = 0);


} // namespace ImGui