#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

// Widgets: Data Plotting
// - Consider using ImPlot (https://github.com/epezent/implot)
IMGUI_API void PlotLines(const char *label, const float *values, int values_count, int values_offset = 0,
                         const char *overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX,
                         ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
IMGUI_API void PlotLines(const char *label, float (*values_getter)(void *data, int idx), void *data, int values_count,
                         int values_offset = 0, const char *overlay_text = NULL, float scale_min = FLT_MAX,
                         float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));
IMGUI_API void PlotHistogram(const char *label, const float *values, int values_count, int values_offset = 0,
                             const char *overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX,
                             ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
IMGUI_API void PlotHistogram(const char *label, float (*values_getter)(void *data, int idx), void *data,
                             int values_count, int values_offset = 0, const char *overlay_text = NULL,
                             float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));

// Plot
IMGUI_API int PlotEx(ImGuiPlotType plot_type, const char *label, float (*values_getter)(void *data, int idx),
                     void *data, int values_count, int values_offset, const char *overlay_text, float scale_min,
                     float scale_max, ImVec2 frame_size);


} // namespace ImGui