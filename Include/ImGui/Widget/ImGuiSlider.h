#pragma once

// #include <ImGui/ImGuiDefine.h>
// #include <ImGui/ImGuiType.h>

namespace ImGui
{


// Widgets: Drag Sliders
// - CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped and can go
// off-bounds.
// - For all the Float2/Float3/Float4/Int2/Int3/Int4 versions of every functions, note that a 'float v[X]' function
// argument is the same as 'float* v', the array syntax is just a way to document the number of elements that are
// expected to be accessible. You can pass address of your first element out of a contiguous set, e.g. &myvector.x
// - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g.
// "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
// - Format string may also be set to NULL or use the default format ("%f" or "%d").
// - Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For
// gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
// - Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits.
// - Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid
// clamping to a minimum.
// - We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it
// easier to swap them.
// - Legacy: Pre-1.78 there are DragXXX() function signatures that takes a final `float power=1.0f' argument instead of
// the `ImGuiSliderFlags flags=0' argument.
//   If you get a warning converting a float to ImGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361
IMGUI_API bool DragFloat(const char *label, float *v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                         const char *format = "%.3f", ImGuiSliderFlags flags = 0); // If v_min >= v_max we have no bound
IMGUI_API bool DragFloat2(const char *label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                          const char *format = "%.3f", ImGuiSliderFlags flags = 0);
IMGUI_API bool DragFloat3(const char *label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                          const char *format = "%.3f", ImGuiSliderFlags flags = 0);
IMGUI_API bool DragFloat4(const char *label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                          const char *format = "%.3f", ImGuiSliderFlags flags = 0);
IMGUI_API bool DragFloatRange2(const char *label, float *v_current_min, float *v_current_max, float v_speed = 1.0f,
                               float v_min = 0.0f, float v_max = 0.0f, const char *format = "%.3f",
                               const char *format_max = NULL, ImGuiSliderFlags flags = 0);
IMGUI_API bool DragInt(const char *label, int *v, float v_speed = 1.0f, int v_min = 0, int v_max = 0,
                       const char *format = "%d", ImGuiSliderFlags flags = 0); // If v_min >= v_max we have no bound
IMGUI_API bool DragInt2(const char *label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0,
                        const char *format = "%d", ImGuiSliderFlags flags = 0);
IMGUI_API bool DragInt3(const char *label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0,
                        const char *format = "%d", ImGuiSliderFlags flags = 0);
IMGUI_API bool DragInt4(const char *label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0,
                        const char *format = "%d", ImGuiSliderFlags flags = 0);
IMGUI_API bool DragIntRange2(const char *label, int *v_current_min, int *v_current_max, float v_speed = 1.0f,
                             int v_min = 0, int v_max = 0, const char *format = "%d", const char *format_max = NULL,
                             ImGuiSliderFlags flags = 0);
IMGUI_API bool DragScalar(const char *label, ImGuiDataType data_type, void *p_data, float v_speed = 1.0f,
                          const void *p_min = NULL, const void *p_max = NULL, const char *format = NULL,
                          ImGuiSliderFlags flags = 0);
IMGUI_API bool DragScalarN(const char *label, ImGuiDataType data_type, void *p_data, int components,
                           float v_speed = 1.0f, const void *p_min = NULL, const void *p_max = NULL,
                           const char *format = NULL, ImGuiSliderFlags flags = 0);



// Widgets: Regular Sliders
// - CTRL+Click on any slider to turn them into an input box. Manually input values aren't clamped and can go
// off-bounds.
// - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g.
// "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
// - Format string may also be set to NULL or use the default format ("%f" or "%d").
// - Legacy: Pre-1.78 there are SliderXXX() function signatures that takes a final `float power=1.0f' argument instead
// of the `ImGuiSliderFlags flags=0' argument.
//   If you get a warning converting a float to ImGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361
IMGUI_API bool SliderFloat(const char *label, float *v, float v_min, float v_max, const char *format = "%.3f",
                           ImGuiSliderFlags flags = 0); // adjust format to decorate the value with a prefix or a suffix
                                                        // for in-slider labels or unit display.
IMGUI_API bool SliderFloat2(const char *label, float v[2], float v_min, float v_max, const char *format = "%.3f",
                            ImGuiSliderFlags flags = 0);
IMGUI_API bool SliderFloat3(const char *label, float v[3], float v_min, float v_max, const char *format = "%.3f",
                            ImGuiSliderFlags flags = 0);
IMGUI_API bool SliderFloat4(const char *label, float v[4], float v_min, float v_max, const char *format = "%.3f",
                            ImGuiSliderFlags flags = 0);
IMGUI_API bool SliderAngle(const char *label, float *v_rad, float v_degrees_min = -360.0f,
                           float v_degrees_max = +360.0f, const char *format = "%.0f deg", ImGuiSliderFlags flags = 0);
IMGUI_API bool SliderInt(const char *label, int *v, int v_min, int v_max, const char *format = "%d",
                         ImGuiSliderFlags flags = 0);
IMGUI_API bool SliderInt2(const char *label, int v[2], int v_min, int v_max, const char *format = "%d",
                          ImGuiSliderFlags flags = 0);
IMGUI_API bool SliderInt3(const char *label, int v[3], int v_min, int v_max, const char *format = "%d",
                          ImGuiSliderFlags flags = 0);
IMGUI_API bool SliderInt4(const char *label, int v[4], int v_min, int v_max, const char *format = "%d",
                          ImGuiSliderFlags flags = 0);
IMGUI_API bool SliderScalar(const char *label, ImGuiDataType data_type, void *p_data, const void *p_min,
                            const void *p_max, const char *format = NULL, ImGuiSliderFlags flags = 0);
IMGUI_API bool SliderScalarN(const char *label, ImGuiDataType data_type, void *p_data, int components,
                             const void *p_min, const void *p_max, const char *format = NULL,
                             ImGuiSliderFlags flags = 0);
IMGUI_API bool VSliderFloat(const char *label, const ImVec2 &size, float *v, float v_min, float v_max,
                            const char *format = "%.3f", ImGuiSliderFlags flags = 0);
IMGUI_API bool VSliderInt(const char *label, const ImVec2 &size, int *v, int v_min, int v_max,
                          const char *format = "%d", ImGuiSliderFlags flags = 0);
IMGUI_API bool VSliderScalar(const char *label, const ImVec2 &size, ImGuiDataType data_type, void *p_data,
                             const void *p_min, const void *p_max, const char *format = NULL,
                             ImGuiSliderFlags flags = 0);


} // namespace ImGui