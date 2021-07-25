
#include <ImGui/ImGui.h>
#include <ImGui/ImGuiInternal.h>



// Supported flags: ImGuiPopupFlags_AnyPopupId, ImGuiPopupFlags_AnyPopupLevel
bool ImGui::IsPopupOpen(ImGuiID id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext &g = *GImGui;
    if (popup_flags & ImGuiPopupFlags_AnyPopupId)
    {
        // Return true if any popup is open at the current BeginPopup() level of the popup stack
        // This may be used to e.g. test for another popups already opened to handle popups priorities at the same
        // level.
        IM_ASSERT(id == 0);
        if (popup_flags & ImGuiPopupFlags_AnyPopupLevel)
            return g.OpenPopupStack.Size > 0;
        else
            return g.OpenPopupStack.Size > g.BeginPopupStack.Size;
    }
    else
    {
        if (popup_flags & ImGuiPopupFlags_AnyPopupLevel)
        {
            // Return true if the popup is open anywhere in the popup stack
            for (int n = 0; n < g.OpenPopupStack.Size; n++)
                if (g.OpenPopupStack[n].PopupId == id)
                    return true;
            return false;
        }
        else
        {
            // Return true if the popup is open at the current BeginPopup() level of the popup stack (this is the
            // most-common query)
            return g.OpenPopupStack.Size > g.BeginPopupStack.Size &&
                   g.OpenPopupStack[g.BeginPopupStack.Size].PopupId == id;
        }
    }
}

bool ImGui::IsPopupOpen(const char *str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext &g = *GImGui;
    ImGuiID id = (popup_flags & ImGuiPopupFlags_AnyPopupId) ? 0 : g.CurrentWindow->GetID(str_id);
    if ((popup_flags & ImGuiPopupFlags_AnyPopupLevel) && id != 0)
        IM_ASSERT(
            0 && "Cannot use IsPopupOpen() with a string id and ImGuiPopupFlags_AnyPopupLevel."); // But non-string
                                                                                                  // version is legal
                                                                                                  // and used internally
    return IsPopupOpen(id, popup_flags);
}

ImGuiWindow *ImGui::GetTopMostPopupModal()
{
    ImGuiContext &g = *GImGui;
    for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
        if (ImGuiWindow *popup = g.OpenPopupStack.Data[n].Window)
            if (popup->Flags & ImGuiWindowFlags_Modal)
                return popup;
    return NULL;
}

void ImGui::OpenPopup(const char *str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext &g = *GImGui;
    OpenPopupEx(g.CurrentWindow->GetID(str_id), popup_flags);
}

void ImGui::OpenPopup(ImGuiID id, ImGuiPopupFlags popup_flags)
{
    OpenPopupEx(id, popup_flags);
}

// Mark popup as open (toggle toward open state).
// Popups are closed when user click outside, or activate a pressable item, or CloseCurrentPopup() is called within a
// BeginPopup()/EndPopup() block. Popup identifiers are relative to the current ID-stack (so OpenPopup and BeginPopup
// needs to be at the same level). One open popup per level of the popup hierarchy (NB: when assigning we reset the
// Window member of ImGuiPopupRef to NULL)
void ImGui::OpenPopupEx(ImGuiID id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *parent_window = g.CurrentWindow;
    const int current_stack_size = g.BeginPopupStack.Size;

    if (popup_flags & ImGuiPopupFlags_NoOpenOverExistingPopup)
        if (IsPopupOpen(0u, ImGuiPopupFlags_AnyPopupId))
            return;

    ImGuiPopupData
        popup_ref; // Tagged as new ref as Window will be set back to NULL if we write this into OpenPopupStack.
    popup_ref.PopupId = id;
    popup_ref.Window = NULL;
    popup_ref.SourceWindow = g.NavWindow;
    popup_ref.OpenFrameCount = g.FrameCount;
    popup_ref.OpenParentId = parent_window->IDStack.back();
    popup_ref.OpenPopupPos = NavCalcPreferredRefPos();
    popup_ref.OpenMousePos = IsMousePosValid(&g.IO.MousePos) ? g.IO.MousePos : popup_ref.OpenPopupPos;

    IMGUI_DEBUG_LOG_POPUP("OpenPopupEx(0x%08X)\n", id);
    if (g.OpenPopupStack.Size < current_stack_size + 1)
    {
        g.OpenPopupStack.push_back(popup_ref);
    }
    else
    {
        // Gently handle the user mistakenly calling OpenPopup() every frame. It is a programming mistake! However, if
        // we were to run the regular code path, the ui would become completely unusable because the popup will always
        // be in hidden-while-calculating-size state _while_ claiming focus. Which would be a very confusing situation
        // for the programmer. Instead, we silently allow the popup to proceed, it will keep reappearing and the
        // programming error will be more obvious to understand.
        if (g.OpenPopupStack[current_stack_size].PopupId == id &&
            g.OpenPopupStack[current_stack_size].OpenFrameCount == g.FrameCount - 1)
        {
            g.OpenPopupStack[current_stack_size].OpenFrameCount = popup_ref.OpenFrameCount;
        }
        else
        {
            // Close child popups if any, then flag popup for open/reopen
            ClosePopupToLevel(current_stack_size, false);
            g.OpenPopupStack.push_back(popup_ref);
        }

        // When reopening a popup we first refocus its parent, otherwise if its parent is itself a popup it would get
        // closed by ClosePopupsOverWindow(). This is equivalent to what ClosePopupToLevel() does.
        // if (g.OpenPopupStack[current_stack_size].PopupId == id)
        //    FocusWindow(parent_window);
    }
}

// When popups are stacked, clicking on a lower level popups puts focus back to it and close popups above it.
// This function closes any popups that are over 'ref_window'.
void ImGui::ClosePopupsOverWindow(ImGuiWindow *ref_window, bool restore_focus_to_window_under_popup)
{
    ImGuiContext &g = *GImGui;
    if (g.OpenPopupStack.Size == 0)
        return;

    // Don't close our own child popup windows.
    int popup_count_to_keep = 0;
    if (ref_window)
    {
        // Find the highest popup which is a descendant of the reference window (generally reference window = NavWindow)
        for (; popup_count_to_keep < g.OpenPopupStack.Size; popup_count_to_keep++)
        {
            ImGuiPopupData &popup = g.OpenPopupStack[popup_count_to_keep];
            if (!popup.Window)
                continue;
            IM_ASSERT((popup.Window->Flags & ImGuiWindowFlags_Popup) != 0);
            if (popup.Window->Flags & ImGuiWindowFlags_ChildWindow)
                continue;

            // Trim the stack unless the popup is a direct parent of the reference window (the reference window is often
            // the NavWindow)
            // - With this stack of window, clicking/focusing Popup1 will close Popup2 and Popup3:
            //     Window -> Popup1 -> Popup2 -> Popup3
            // - Each popups may contain child windows, which is why we compare ->RootWindow!
            //     Window -> Popup1 -> Popup1_Child -> Popup2 -> Popup2_Child
            bool ref_window_is_descendent_of_popup = false;
            for (int n = popup_count_to_keep; n < g.OpenPopupStack.Size; n++)
                if (ImGuiWindow *popup_window = g.OpenPopupStack[n].Window)
                    if (popup_window->RootWindow == ref_window->RootWindow)
                    {
                        ref_window_is_descendent_of_popup = true;
                        break;
                    }
            if (!ref_window_is_descendent_of_popup)
                break;
        }
    }
    if (popup_count_to_keep <
        g.OpenPopupStack
            .Size) // This test is not required but it allows to set a convenient breakpoint on the statement below
    {
        IMGUI_DEBUG_LOG_POPUP("ClosePopupsOverWindow(\"%s\") -> ClosePopupToLevel(%d)\n", ref_window->Name,
                              popup_count_to_keep);
        ClosePopupToLevel(popup_count_to_keep, restore_focus_to_window_under_popup);
    }
}

void ImGui::ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup)
{
    ImGuiContext &g = *GImGui;
    IMGUI_DEBUG_LOG_POPUP("ClosePopupToLevel(%d), restore_focus_to_window_under_popup=%d\n", remaining,
                          restore_focus_to_window_under_popup);
    IM_ASSERT(remaining >= 0 && remaining < g.OpenPopupStack.Size);

    // Trim open popup stack
    ImGuiWindow *focus_window = g.OpenPopupStack[remaining].SourceWindow;
    ImGuiWindow *popup_window = g.OpenPopupStack[remaining].Window;
    g.OpenPopupStack.resize(remaining);

    if (restore_focus_to_window_under_popup)
    {
        if (focus_window && !focus_window->WasActive && popup_window)
        {
            // Fallback
            FocusTopMostWindowUnderOne(popup_window, NULL);
        }
        else
        {
            if (g.NavLayer == ImGuiNavLayer_Main && focus_window)
                focus_window = NavRestoreLastChildNavWindow(focus_window);
            FocusWindow(focus_window);
        }
    }
}

// Close the popup we have begin-ed into.
void ImGui::CloseCurrentPopup()
{
    ImGuiContext &g = *GImGui;
    int popup_idx = g.BeginPopupStack.Size - 1;
    if (popup_idx < 0 || popup_idx >= g.OpenPopupStack.Size ||
        g.BeginPopupStack[popup_idx].PopupId != g.OpenPopupStack[popup_idx].PopupId)
        return;

    // Closing a menu closes its top-most parent popup (unless a modal)
    while (popup_idx > 0)
    {
        ImGuiWindow *popup_window = g.OpenPopupStack[popup_idx].Window;
        ImGuiWindow *parent_popup_window = g.OpenPopupStack[popup_idx - 1].Window;
        bool close_parent = false;
        if (popup_window && (popup_window->Flags & ImGuiWindowFlags_ChildMenu))
            if (parent_popup_window == NULL || !(parent_popup_window->Flags & ImGuiWindowFlags_Modal))
                close_parent = true;
        if (!close_parent)
            break;
        popup_idx--;
    }
    IMGUI_DEBUG_LOG_POPUP("CloseCurrentPopup %d -> %d\n", g.BeginPopupStack.Size - 1, popup_idx);
    ClosePopupToLevel(popup_idx, true);

    // A common pattern is to close a popup when selecting a menu item/selectable that will open another window.
    // To improve this usage pattern, we avoid nav highlight for a single frame in the parent window.
    // Similarly, we could avoid mouse hover highlight in this window but it is less visually problematic.
    if (ImGuiWindow *window = g.NavWindow)
        window->DC.NavHideHighlightOneFrame = true;
}

// Attention! BeginPopup() adds default flags which BeginPopupEx()!
bool ImGui::BeginPopupEx(ImGuiID id, ImGuiWindowFlags flags)
{
    ImGuiContext &g = *GImGui;
    if (!IsPopupOpen(id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        return false;
    }

    char name[20];
    if (flags & ImGuiWindowFlags_ChildMenu)
        ImFormatString(name, IM_ARRAYSIZE(name), "##Menu_%02d",
                       g.BeginPopupStack.Size); // Recycle windows based on depth
    else
        ImFormatString(name, IM_ARRAYSIZE(name), "##Popup_%08x",
                       id); // Not recycling, so we can close/open during the same frame

    flags |= ImGuiWindowFlags_Popup;
    bool is_open = Begin(name, NULL, flags);
    if (!is_open) // NB: Begin can return false when the popup is completely clipped (e.g. zero size display)
        EndPopup();

    return is_open;
}


bool ImGui::BeginPopup(const char *str_id, ImGuiWindowFlags flags)
{
    ImGuiContext &g = *GImGui;
    if (g.OpenPopupStack.Size <= g.BeginPopupStack.Size) // Early out for performance
    {
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        return false;
    }
    flags |= ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;
    return BeginPopupEx(g.CurrentWindow->GetID(str_id), flags);
}

// If 'p_open' is specified for a modal popup window, the popup will have a regular close button which will close the
// popup. Note that popup visibility status is owned by Dear ImGui (and manipulated with e.g. OpenPopup) so the actual
// value of *p_open is meaningless here.
bool ImGui::BeginPopupModal(const char *name, bool *p_open, ImGuiWindowFlags flags)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    const ImGuiID id = window->GetID(name);
    if (!IsPopupOpen(id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        return false;
    }

    // Center modal windows by default for increased visibility
    // (this won't really last as settings will kick in, and is mostly for backward compatibility. user may do the same
    // themselves)
    // FIXME: Should test for (PosCond & window->SetWindowPosAllowFlags) with the upcoming window.
    if ((g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasPos) == 0)
    {
        const ImGuiViewport *viewport = GetMainViewport();
        SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    }

    flags |= ImGuiWindowFlags_Popup | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse;
    const bool is_open = Begin(name, p_open, flags);
    if (!is_open ||
        (p_open &&
         !*p_open)) // NB: is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
    {
        EndPopup();
        if (is_open)
            ClosePopupToLevel(g.BeginPopupStack.Size, true);
        return false;
    }
    return is_open;
}

void ImGui::EndPopup()
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    IM_ASSERT(window->Flags & ImGuiWindowFlags_Popup); // Mismatched BeginPopup()/EndPopup() calls
    IM_ASSERT(g.BeginPopupStack.Size > 0);

    // Make all menus and popups wrap around for now, may need to expose that policy.
    if (g.NavWindow == window)
        NavMoveRequestTryWrapping(window, ImGuiNavMoveFlags_LoopY);

    // Child-popups don't need to be laid out
    IM_ASSERT(g.WithinEndChild == false);
    if (window->Flags & ImGuiWindowFlags_ChildWindow)
        g.WithinEndChild = true;
    End();
    g.WithinEndChild = false;
}

// Helper to open a popup if mouse button is released over the item
// - This is essentially the same as BeginPopupContextItem() but without the trailing BeginPopup()
void ImGui::OpenPopupOnItemClick(const char *str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiWindow *window = GImGui->CurrentWindow;
    int mouse_button = (popup_flags & ImGuiPopupFlags_MouseButtonMask_);
    if (IsMouseReleased(mouse_button) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
    {
        ImGuiID id = str_id ? window->GetID(str_id)
                            : window->DC.LastItemId; // If user hasn't passed an ID, we can use the LastItemID. Using
                                                     // LastItemID as a Popup ID won't conflict!
        IM_ASSERT(id != 0); // You cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
        OpenPopupEx(id, popup_flags);
    }
}

// This is a helper to handle the simplest case of associating one named popup to one given widget.
// - To create a popup associated to the last item, you generally want to pass a NULL value to str_id.
// - To create a popup with a specific identifier, pass it in str_id.
//    - This is useful when using using BeginPopupContextItem() on an item which doesn't have an identifier, e.g. a
//    Text() call.
//    - This is useful when multiple code locations may want to manipulate/open the same popup, given an explicit id.
// - You may want to handle the whole on user side if you have specific needs (e.g. tweaking IsItemHovered()
// parameters).
//   This is essentially the same as:
//       id = str_id ? GetID(str_id) : GetItemID();
//       OpenPopupOnItemClick(str_id);
//       return BeginPopup(id);
//   Which is essentially the same as:
//       id = str_id ? GetID(str_id) : GetItemID();
//       if (IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Right))
//           OpenPopup(id);
//       return BeginPopup(id);
//   The main difference being that this is tweaked to avoid computing the ID twice.
bool ImGui::BeginPopupContextItem(const char *str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiWindow *window = GImGui->CurrentWindow;
    if (window->SkipItems)
        return false;
    ImGuiID id = str_id ? window->GetID(str_id)
                        : window->DC.LastItemId; // If user hasn't passed an ID, we can use the LastItemID. Using
                                                 // LastItemID as a Popup ID won't conflict!
    IM_ASSERT(id != 0); // You cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
    int mouse_button = (popup_flags & ImGuiPopupFlags_MouseButtonMask_);
    if (IsMouseReleased(mouse_button) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
                                ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::BeginPopupContextWindow(const char *str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiWindow *window = GImGui->CurrentWindow;
    if (!str_id)
        str_id = "window_context";
    ImGuiID id = window->GetID(str_id);
    int mouse_button = (popup_flags & ImGuiPopupFlags_MouseButtonMask_);
    if (IsMouseReleased(mouse_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        if (!(popup_flags & ImGuiPopupFlags_NoOpenOverItems) || !IsAnyItemHovered())
            OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
                                ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::BeginPopupContextVoid(const char *str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiWindow *window = GImGui->CurrentWindow;
    if (!str_id)
        str_id = "void_context";
    ImGuiID id = window->GetID(str_id);
    int mouse_button = (popup_flags & ImGuiPopupFlags_MouseButtonMask_);
    if (IsMouseReleased(mouse_button) && !IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
        if (GetTopMostPopupModal() == NULL)
            OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
                                ImGuiWindowFlags_NoSavedSettings);
}

// r_avoid = the rectangle to avoid (e.g. for tooltip it is a rectangle around the mouse cursor which we want to avoid.
// for popups it's a small point around the cursor.) r_outer = the visible area rectangle, minus safe area padding. If
// our popup size won't fit because of safe area padding we ignore it. (r_outer is usually equivalent to the viewport
// rectangle minus padding, but when multi-viewports are enabled and monitor
//  information are available, it may represent the entire platform monitor from the frame of reference of the current
//  viewport. this allows us to have tooltips/popups displayed out of the parent viewport.)
ImVec2 ImGui::FindBestWindowPosForPopupEx(const ImVec2 &ref_pos, const ImVec2 &size, ImGuiDir *last_dir,
                                          const ImRect &r_outer, const ImRect &r_avoid, ImGuiPopupPositionPolicy policy)
{
    ImVec2 base_pos_clamped = ImClamp(ref_pos, r_outer.Min, r_outer.Max - size);
    // GetForegroundDrawList()->AddRect(r_avoid.Min, r_avoid.Max, IM_COL32(255,0,0,255));
    // GetForegroundDrawList()->AddRect(r_outer.Min, r_outer.Max, IM_COL32(0,255,0,255));

    // Combo Box policy (we want a connecting edge)
    if (policy == ImGuiPopupPositionPolicy_ComboBox)
    {
        const ImGuiDir dir_prefered_order[ImGuiDir_COUNT] = {ImGuiDir_Down, ImGuiDir_Right, ImGuiDir_Left, ImGuiDir_Up};
        for (int n = (*last_dir != ImGuiDir_None) ? -1 : 0; n < ImGuiDir_COUNT; n++)
        {
            const ImGuiDir dir = (n == -1) ? *last_dir : dir_prefered_order[n];
            if (n != -1 && dir == *last_dir) // Already tried this direction?
                continue;
            ImVec2 pos;
            if (dir == ImGuiDir_Down)
                pos = ImVec2(r_avoid.Min.x, r_avoid.Max.y); // Below, Toward Right (default)
            if (dir == ImGuiDir_Right)
                pos = ImVec2(r_avoid.Min.x, r_avoid.Min.y - size.y); // Above, Toward Right
            if (dir == ImGuiDir_Left)
                pos = ImVec2(r_avoid.Max.x - size.x, r_avoid.Max.y); // Below, Toward Left
            if (dir == ImGuiDir_Up)
                pos = ImVec2(r_avoid.Max.x - size.x, r_avoid.Min.y - size.y); // Above, Toward Left
            if (!r_outer.Contains(ImRect(pos, pos + size)))
                continue;
            *last_dir = dir;
            return pos;
        }
    }

    // Tooltip and Default popup policy
    // (Always first try the direction we used on the last frame, if any)
    if (policy == ImGuiPopupPositionPolicy_Tooltip || policy == ImGuiPopupPositionPolicy_Default)
    {
        const ImGuiDir dir_prefered_order[ImGuiDir_COUNT] = {ImGuiDir_Right, ImGuiDir_Down, ImGuiDir_Up, ImGuiDir_Left};
        for (int n = (*last_dir != ImGuiDir_None) ? -1 : 0; n < ImGuiDir_COUNT; n++)
        {
            const ImGuiDir dir = (n == -1) ? *last_dir : dir_prefered_order[n];
            if (n != -1 && dir == *last_dir) // Already tried this direction?
                continue;

            const float avail_w = (dir == ImGuiDir_Left ? r_avoid.Min.x : r_outer.Max.x) -
                                  (dir == ImGuiDir_Right ? r_avoid.Max.x : r_outer.Min.x);
            const float avail_h = (dir == ImGuiDir_Up ? r_avoid.Min.y : r_outer.Max.y) -
                                  (dir == ImGuiDir_Down ? r_avoid.Max.y : r_outer.Min.y);

            // If there not enough room on one axis, there's no point in positioning on a side on this axis (e.g. when
            // not enough width, use a top/bottom position to maximize available width)
            if (avail_w < size.x && (dir == ImGuiDir_Left || dir == ImGuiDir_Right))
                continue;
            if (avail_h < size.y && (dir == ImGuiDir_Up || dir == ImGuiDir_Down))
                continue;

            ImVec2 pos;
            pos.x = (dir == ImGuiDir_Left)    ? r_avoid.Min.x - size.x
                    : (dir == ImGuiDir_Right) ? r_avoid.Max.x
                                              : base_pos_clamped.x;
            pos.y = (dir == ImGuiDir_Up)     ? r_avoid.Min.y - size.y
                    : (dir == ImGuiDir_Down) ? r_avoid.Max.y
                                             : base_pos_clamped.y;

            // Clamp top-left corner of popup
            pos.x = ImMax(pos.x, r_outer.Min.x);
            pos.y = ImMax(pos.y, r_outer.Min.y);

            *last_dir = dir;
            return pos;
        }
    }

    // Fallback when not enough room:
    *last_dir = ImGuiDir_None;

    // For tooltip we prefer avoiding the cursor at all cost even if it means that part of the tooltip won't be visible.
    if (policy == ImGuiPopupPositionPolicy_Tooltip)
        return ref_pos + ImVec2(2, 2);

    // Otherwise try to keep within display
    ImVec2 pos = ref_pos;
    pos.x = ImMax(ImMin(pos.x + size.x, r_outer.Max.x) - size.x, r_outer.Min.x);
    pos.y = ImMax(ImMin(pos.y + size.y, r_outer.Max.y) - size.y, r_outer.Min.y);
    return pos;
}

// Note that this is used for popups, which can overlap the non work-area of individual viewports.
ImRect ImGui::GetWindowAllowedExtentRect(ImGuiWindow *window)
{
    ImGuiContext &g = *GImGui;
    IM_UNUSED(window);
    ImRect r_screen = ((ImGuiViewportP *)(void *)GetMainViewport())->GetMainRect();
    ImVec2 padding = g.Style.DisplaySafeAreaPadding;
    r_screen.Expand(ImVec2((r_screen.GetWidth() > padding.x * 2) ? -padding.x : 0.0f,
                           (r_screen.GetHeight() > padding.y * 2) ? -padding.y : 0.0f));
    return r_screen;
}

ImVec2 ImGui::FindBestWindowPosForPopup(ImGuiWindow *window)
{
    ImGuiContext &g = *GImGui;

    ImRect r_outer = GetWindowAllowedExtentRect(window);
    if (window->Flags & ImGuiWindowFlags_ChildMenu)
    {
        // Child menus typically request _any_ position within the parent menu item, and then we move the new menu
        // outside the parent bounds. This is how we end up with child menus appearing (most-commonly) on the right of
        // the parent menu.
        IM_ASSERT(g.CurrentWindow == window);
        ImGuiWindow *parent_window = g.CurrentWindowStack[g.CurrentWindowStack.Size - 2];
        float horizontal_overlap =
            g.Style.ItemInnerSpacing.x; // We want some overlap to convey the relative depth of each menu (currently the
                                        // amount of overlap is hard-coded to style.ItemSpacing.x).
        ImRect r_avoid;
        if (parent_window->DC.MenuBarAppending)
            r_avoid =
                ImRect(-FLT_MAX, parent_window->ClipRect.Min.y, FLT_MAX,
                       parent_window->ClipRect.Max
                           .y); // Avoid parent menu-bar. If we wanted multi-line menu-bar, we may instead want to have
                                // the calling window setup e.g. a NextWindowData.PosConstraintAvoidRect field
        else
            r_avoid = ImRect(parent_window->Pos.x + horizontal_overlap, -FLT_MAX,
                             parent_window->Pos.x + parent_window->Size.x - horizontal_overlap -
                                 parent_window->ScrollbarSizes.x,
                             FLT_MAX);
        return FindBestWindowPosForPopupEx(window->Pos, window->Size, &window->AutoPosLastDirection, r_outer, r_avoid,
                                           ImGuiPopupPositionPolicy_Default);
    }
    if (window->Flags & ImGuiWindowFlags_Popup)
    {
        ImRect r_avoid = ImRect(window->Pos.x - 1, window->Pos.y - 1, window->Pos.x + 1, window->Pos.y + 1);
        return FindBestWindowPosForPopupEx(window->Pos, window->Size, &window->AutoPosLastDirection, r_outer, r_avoid,
                                           ImGuiPopupPositionPolicy_Default);
    }
    if (window->Flags & ImGuiWindowFlags_Tooltip)
    {
        // Position tooltip (always follows mouse)
        float sc = g.Style.MouseCursorScale;
        ImVec2 ref_pos = NavCalcPreferredRefPos();
        ImRect r_avoid;
        if (!g.NavDisableHighlight && g.NavDisableMouseHover &&
            !(g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos))
            r_avoid = ImRect(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 16, ref_pos.y + 8);
        else
            r_avoid = ImRect(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 24 * sc,
                             ref_pos.y + 24 * sc); // FIXME: Hard-coded based on mouse cursor shape expectation. Exact
                                                   // dimension not very important.
        return FindBestWindowPosForPopupEx(ref_pos, window->Size, &window->AutoPosLastDirection, r_outer, r_avoid,
                                           ImGuiPopupPositionPolicy_Tooltip);
    }
    IM_ASSERT(0);
    return window->Pos;
}