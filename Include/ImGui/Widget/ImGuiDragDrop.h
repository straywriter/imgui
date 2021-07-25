#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{


// Drag and Drop
// - On source items, call BeginDragDropSource(), if it returns true also call SetDragDropPayload() +
// EndDragDropSource().
// - On target candidates, call BeginDragDropTarget(), if it returns true also call AcceptDragDropPayload() +
// EndDragDropTarget().
// - If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we
// currently display a fallback "..." tooltip, see #1725)
// - An item can be both drag source and drop target.
IMGUI_API bool BeginDragDropSource(
    ImGuiDragDropFlags flags = 0); // call after submitting an item which may be dragged. when this return true, you can
                                   // call SetDragDropPayload() + EndDragDropSource()
IMGUI_API bool SetDragDropPayload(
    const char *type, const void *data, size_t sz,
    ImGuiCond cond = 0); // type is a user defined string of maximum 32 characters. Strings starting with '_' are
                         // reserved for dear imgui internal types. Data is copied and held by imgui.
IMGUI_API void EndDragDropSource();   // only call EndDragDropSource() if BeginDragDropSource() returns true!
IMGUI_API bool BeginDragDropTarget(); // call after submitting an item that may receive a payload. If this returns true,
                                      // you can call AcceptDragDropPayload() + EndDragDropTarget()
IMGUI_API const ImGuiPayload *AcceptDragDropPayload(
    const char *type,
    ImGuiDragDropFlags flags = 0); // accept contents of a given type. If ImGuiDragDropFlags_AcceptBeforeDelivery is set
                                   // you can peek into the payload before the mouse button is released.
IMGUI_API void EndDragDropTarget(); // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
IMGUI_API const ImGuiPayload *GetDragDropPayload(); // peek directly into the current payload from anywhere. may return
                                                    // NULL. use ImGuiPayload::IsDataType() to test for the payload
                                                    // type.


} // namespace ImGui