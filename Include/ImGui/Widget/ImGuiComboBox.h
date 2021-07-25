#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{

// Widgets: Combo Box
// - The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by
// creating e.g. Selectable() items.
// - The old Combo() api are helpers over BeginCombo()/EndCombo() which are kept available for convenience purpose. This
// is analogous to how ListBox are created.
IMGUI_API bool BeginCombo(const char *label, const char *preview_value, ImGuiComboFlags flags = 0);
IMGUI_API void EndCombo(); // only call EndCombo() if BeginCombo() returns true!
IMGUI_API bool Combo(const char *label, int *current_item, const char *const items[], int items_count,
                     int popup_max_height_in_items = -1);
IMGUI_API bool Combo(const char *label, int *current_item, const char *items_separated_by_zeros,
                     int popup_max_height_in_items = -1); // Separate items with \0 within a string, end item-list with
                                                          // \0\0. e.g. "One\0Two\0Three\0"
IMGUI_API bool Combo(const char *label, int *current_item,
                     bool (*items_getter)(void *data, int idx, const char **out_text), void *data, int items_count,
                     int popup_max_height_in_items = -1);

} // namespace ImGui