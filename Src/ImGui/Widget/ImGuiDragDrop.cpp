
#include <ImGui/ImGui.h>
#include <ImGui/ImGuiInternal.h>


// When this returns true you need to: a) call SetDragDropPayload() exactly once, b) you may render the payload
// visual/description, c) call EndDragDropSource() If the item has an identifier:
// - This assume/require the item to be activated (typically via ButtonBehavior).
// - Therefore if you want to use this with a mouse button other than left mouse button, it is up to the item itself to
// activate with another button.
// - We then pull and use the mouse button that was used to activate the item and use it to carry on the drag.
// If the item has no identifier:
// - Currently always assume left mouse button.
bool ImGui::BeginDragDropSource(ImGuiDragDropFlags flags)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;

    // FIXME-DRAGDROP: While in the common-most "drag from non-zero active id" case we can tell the mouse button,
    // in both SourceExtern and id==0 cases we may requires something else (explicit flags or some heuristic).
    ImGuiMouseButton mouse_button = ImGuiMouseButton_Left;

    bool source_drag_active = false;
    ImGuiID source_id = 0;
    ImGuiID source_parent_id = 0;
    if (!(flags & ImGuiDragDropFlags_SourceExtern))
    {
        source_id = window->DC.LastItemId;
        if (source_id != 0)
        {
            // Common path: items with ID
            if (g.ActiveId != source_id)
                return false;
            if (g.ActiveIdMouseButton != -1)
                mouse_button = g.ActiveIdMouseButton;
            if (g.IO.MouseDown[mouse_button] == false)
                return false;
            g.ActiveIdAllowOverlap = false;
        }
        else
        {
            // Uncommon path: items without ID
            if (g.IO.MouseDown[mouse_button] == false)
                return false;

            // If you want to use BeginDragDropSource() on an item with no unique identifier for interaction, such as
            // Text() or Image(), you need to: A) Read the explanation below, B) Use the
            // ImGuiDragDropFlags_SourceAllowNullID flag, C) Swallow your programmer pride.
            if (!(flags & ImGuiDragDropFlags_SourceAllowNullID))
            {
                IM_ASSERT(0);
                return false;
            }

            // Early out
            if ((window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_HoveredRect) == 0 &&
                (g.ActiveId == 0 || g.ActiveIdWindow != window))
                return false;

            // Magic fallback (=somehow reprehensible) to handle items with no assigned ID, e.g. Text(), Image()
            // We build a throwaway ID based on current ID stack + relative AABB of items in window.
            // THE IDENTIFIER WON'T SURVIVE ANY REPOSITIONING OF THE WIDGET, so if your widget moves your dragging
            // operation will be canceled. We don't need to maintain/call ClearActiveID() as releasing the button will
            // early out this function and trigger !ActiveIdIsAlive.
            source_id = window->DC.LastItemId = window->GetIDFromRectangle(window->DC.LastItemRect);
            bool is_hovered = ItemHoverable(window->DC.LastItemRect, source_id);
            if (is_hovered && g.IO.MouseClicked[mouse_button])
            {
                SetActiveID(source_id, window);
                FocusWindow(window);
            }
            if (g.ActiveId == source_id) // Allow the underlying widget to display/return hovered during the mouse
                                         // release frame, else we would get a flicker.
                g.ActiveIdAllowOverlap = is_hovered;
        }
        if (g.ActiveId != source_id)
            return false;
        source_parent_id = window->IDStack.back();
        source_drag_active = IsMouseDragging(mouse_button);

        // Disable navigation and key inputs while dragging
        g.ActiveIdUsingNavDirMask = ~(ImU32)0;
        g.ActiveIdUsingNavInputMask = ~(ImU32)0;
        g.ActiveIdUsingKeyInputMask = ~(ImU64)0;
    }
    else
    {
        window = NULL;
        source_id = ImHashStr("#SourceExtern");
        source_drag_active = true;
    }

    if (source_drag_active)
    {
        if (!g.DragDropActive)
        {
            IM_ASSERT(source_id != 0);
            ClearDragDrop();
            ImGuiPayload &payload = g.DragDropPayload;
            payload.SourceId = source_id;
            payload.SourceParentId = source_parent_id;
            g.DragDropActive = true;
            g.DragDropSourceFlags = flags;
            g.DragDropMouseButton = mouse_button;
            if (payload.SourceId == g.ActiveId)
                g.ActiveIdNoClearOnFocusLoss = true;
        }
        g.DragDropSourceFrameCount = g.FrameCount;
        g.DragDropWithinSource = true;

        if (!(flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
        {
            // Target can request the Source to not display its tooltip (we use a dedicated flag to make this request
            // explicit) We unfortunately can't just modify the source flags and skip the call to BeginTooltip, as
            // caller may be emitting contents.
            BeginTooltip();
            if (g.DragDropAcceptIdPrev && (g.DragDropAcceptFlags & ImGuiDragDropFlags_AcceptNoPreviewTooltip))
            {
                ImGuiWindow *tooltip_window = g.CurrentWindow;
                tooltip_window->SkipItems = true;
                tooltip_window->HiddenFramesCanSkipItems = 1;
            }
        }

        if (!(flags & ImGuiDragDropFlags_SourceNoDisableHover) && !(flags & ImGuiDragDropFlags_SourceExtern))
            window->DC.LastItemStatusFlags &= ~ImGuiItemStatusFlags_HoveredRect;

        return true;
    }
    return false;
}

void ImGui::EndDragDropSource()
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(g.DragDropActive);
    IM_ASSERT(g.DragDropWithinSource && "Not after a BeginDragDropSource()?");

    if (!(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
        EndTooltip();

    // Discard the drag if have not called SetDragDropPayload()
    if (g.DragDropPayload.DataFrameCount == -1)
        ClearDragDrop();
    g.DragDropWithinSource = false;
}

// Use 'cond' to choose to submit payload on drag start or every frame
bool ImGui::SetDragDropPayload(const char *type, const void *data, size_t data_size, ImGuiCond cond)
{
    ImGuiContext &g = *GImGui;
    ImGuiPayload &payload = g.DragDropPayload;
    if (cond == 0)
        cond = ImGuiCond_Always;

    IM_ASSERT(type != NULL);
    IM_ASSERT(strlen(type) < IM_ARRAYSIZE(payload.DataType) && "Payload type can be at most 32 characters long");
    IM_ASSERT((data != NULL && data_size > 0) || (data == NULL && data_size == 0));
    IM_ASSERT(cond == ImGuiCond_Always || cond == ImGuiCond_Once);
    IM_ASSERT(payload.SourceId != 0); // Not called between BeginDragDropSource() and EndDragDropSource()

    if (cond == ImGuiCond_Always || payload.DataFrameCount == -1)
    {
        // Copy payload
        ImStrncpy(payload.DataType, type, IM_ARRAYSIZE(payload.DataType));
        g.DragDropPayloadBufHeap.resize(0);
        if (data_size > sizeof(g.DragDropPayloadBufLocal))
        {
            // Store in heap
            g.DragDropPayloadBufHeap.resize((int)data_size);
            payload.Data = g.DragDropPayloadBufHeap.Data;
            memcpy(payload.Data, data, data_size);
        }
        else if (data_size > 0)
        {
            // Store locally
            memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
            payload.Data = g.DragDropPayloadBufLocal;
            memcpy(payload.Data, data, data_size);
        }
        else
        {
            payload.Data = NULL;
        }
        payload.DataSize = (int)data_size;
    }
    payload.DataFrameCount = g.FrameCount;

    return (g.DragDropAcceptFrameCount == g.FrameCount) || (g.DragDropAcceptFrameCount == g.FrameCount - 1);
}

bool ImGui::BeginDragDropTargetCustom(const ImRect &bb, ImGuiID id)
{
    ImGuiContext &g = *GImGui;
    if (!g.DragDropActive)
        return false;

    ImGuiWindow *window = g.CurrentWindow;
    ImGuiWindow *hovered_window = g.HoveredWindowUnderMovingWindow;
    if (hovered_window == NULL || window->RootWindow != hovered_window->RootWindow)
        return false;
    IM_ASSERT(id != 0);
    if (!IsMouseHoveringRect(bb.Min, bb.Max) || (id == g.DragDropPayload.SourceId))
        return false;
    if (window->SkipItems)
        return false;

    IM_ASSERT(g.DragDropWithinTarget == false);
    g.DragDropTargetRect = bb;
    g.DragDropTargetId = id;
    g.DragDropWithinTarget = true;
    return true;
}

// We don't use BeginDragDropTargetCustom() and duplicate its code because:
// 1) we use LastItemRectHoveredRect which handles items that pushes a temporarily clip rectangle in their code. Calling
// BeginDragDropTargetCustom(LastItemRect) would not handle them. 2) and it's faster. as this code may be very
// frequently called, we want to early out as fast as we can. Also note how the HoveredWindow test is positioned
// differently in both functions (in both functions we optimize for the cheapest early out case)
bool ImGui::BeginDragDropTarget()
{
    ImGuiContext &g = *GImGui;
    if (!g.DragDropActive)
        return false;

    ImGuiWindow *window = g.CurrentWindow;
    if (!(window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_HoveredRect))
        return false;
    ImGuiWindow *hovered_window = g.HoveredWindowUnderMovingWindow;
    if (hovered_window == NULL || window->RootWindow != hovered_window->RootWindow)
        return false;

    const ImRect &display_rect = (window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_HasDisplayRect)
                                     ? window->DC.LastItemDisplayRect
                                     : window->DC.LastItemRect;
    ImGuiID id = window->DC.LastItemId;
    if (id == 0)
        id = window->GetIDFromRectangle(display_rect);
    if (g.DragDropPayload.SourceId == id)
        return false;

    IM_ASSERT(g.DragDropWithinTarget == false);
    g.DragDropTargetRect = display_rect;
    g.DragDropTargetId = id;
    g.DragDropWithinTarget = true;
    return true;
}

bool ImGui::IsDragDropPayloadBeingAccepted()
{
    ImGuiContext &g = *GImGui;
    return g.DragDropActive && g.DragDropAcceptIdPrev != 0;
}

const ImGuiPayload *ImGui::AcceptDragDropPayload(const char *type, ImGuiDragDropFlags flags)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    ImGuiPayload &payload = g.DragDropPayload;
    IM_ASSERT(g.DragDropActive);             // Not called between BeginDragDropTarget() and EndDragDropTarget() ?
    IM_ASSERT(payload.DataFrameCount != -1); // Forgot to call EndDragDropTarget() ?
    if (type != NULL && !payload.IsDataType(type))
        return NULL;

    // Accept smallest drag target bounding box, this allows us to nest drag targets conveniently without ordering
    // constraints. NB: We currently accept NULL id as target. However, overlapping targets requires a unique ID to
    // function!
    const bool was_accepted_previously = (g.DragDropAcceptIdPrev == g.DragDropTargetId);
    ImRect r = g.DragDropTargetRect;
    float r_surface = r.GetWidth() * r.GetHeight();
    if (r_surface <= g.DragDropAcceptIdCurrRectSurface)
    {
        g.DragDropAcceptFlags = flags;
        g.DragDropAcceptIdCurr = g.DragDropTargetId;
        g.DragDropAcceptIdCurrRectSurface = r_surface;
    }

    // Render default drop visuals
    payload.Preview = was_accepted_previously;
    flags |= (g.DragDropSourceFlags &
              ImGuiDragDropFlags_AcceptNoDrawDefaultRect); // Source can also inhibit the preview (useful for external
                                                           // sources that lives for 1 frame)
    if (!(flags & ImGuiDragDropFlags_AcceptNoDrawDefaultRect) && payload.Preview)
    {
        // FIXME-DRAGDROP: Settle on a proper default visuals for drop target.
        r.Expand(3.5f);
        bool push_clip_rect = !window->ClipRect.Contains(r);
        if (push_clip_rect)
            window->DrawList->PushClipRect(r.Min - ImVec2(1, 1), r.Max + ImVec2(1, 1));
        window->DrawList->AddRect(r.Min, r.Max, GetColorU32(ImGuiCol_DragDropTarget), 0.0f, 0, 2.0f);
        if (push_clip_rect)
            window->DrawList->PopClipRect();
    }

    g.DragDropAcceptFrameCount = g.FrameCount;
    payload.Delivery =
        was_accepted_previously &&
        !IsMouseDown(g.DragDropMouseButton); // For extern drag sources affecting os window focus, it's easier to just
                                             // test !IsMouseDown() instead of IsMouseReleased()
    if (!payload.Delivery && !(flags & ImGuiDragDropFlags_AcceptBeforeDelivery))
        return NULL;

    return &payload;
}

const ImGuiPayload *ImGui::GetDragDropPayload()
{
    ImGuiContext &g = *GImGui;
    return g.DragDropActive ? &g.DragDropPayload : NULL;
}

// We don't really use/need this now, but added it for the sake of consistency and because we might need it later.
void ImGui::EndDragDropTarget()
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(g.DragDropActive);
    IM_ASSERT(g.DragDropWithinTarget);
    g.DragDropWithinTarget = false;
}