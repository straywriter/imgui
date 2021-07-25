#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

// Widgets: List Boxes
// - This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.
// - The BeginListBox()/EndListBox() api allows you to manage your contents and selection state however you want it, by
// creating e.g. Selectable() or any items.
// - The simplified/old ListBox() api are helpers over BeginListBox()/EndListBox() which are kept available for
// convenience purpose. This is analoguous to how Combos are created.
// - Choose frame width:   size.x > 0.0f: custom  /  size.x < 0.0f or -FLT_MIN: right-align   /  size.x = 0.0f
// (default): use current ItemWidth
// - Choose frame height:  size.y > 0.0f: custom  /  size.y < 0.0f or -FLT_MIN: bottom-align  /  size.y = 0.0f
// (default): arbitrary default height which can fit ~7 items
IMGUI_API bool BeginListBox(const char *label, const ImVec2 &size = ImVec2(0, 0)); // open a framed scrolling region
IMGUI_API void EndListBox(); // only call EndListBox() if BeginListBox() returned true!
IMGUI_API bool ListBox(const char *label, int *current_item, const char *const items[], int items_count,
                       int height_in_items = -1);
IMGUI_API bool ListBox(const char *label, int *current_item,
                       bool (*items_getter)(void *data, int idx, const char **out_text), void *data, int items_count,
                       int height_in_items = -1);

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




} // namespace ImGui