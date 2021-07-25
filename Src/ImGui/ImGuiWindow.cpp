// #include <ImGui/ImGuiWindow.h>

#include <ImGui/ImGui.h>

#include <ImGui/ImGuiInternal.h>


bool ImGui::IsWindowAppearing()
{
    ImGuiWindow *window = GetCurrentWindowRead();
    return window->Appearing;
}

bool ImGui::IsWindowCollapsed()
{
    ImGuiWindow *window = GetCurrentWindowRead();
    return window->Collapsed;
}


bool ImGui::IsWindowFocused(ImGuiFocusedFlags flags)
{
    ImGuiContext &g = *GImGui;

    if (flags & ImGuiFocusedFlags_AnyWindow)
        return g.NavWindow != NULL;

    IM_ASSERT(g.CurrentWindow); // Not inside a Begin()/End()
    switch (flags & (ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_ChildWindows))
    {
    case ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_ChildWindows:
        return g.NavWindow && g.NavWindow->RootWindow == g.CurrentWindow->RootWindow;
    case ImGuiFocusedFlags_RootWindow:
        return g.NavWindow == g.CurrentWindow->RootWindow;
    case ImGuiFocusedFlags_ChildWindows:
        return g.NavWindow && IsWindowChildOf(g.NavWindow, g.CurrentWindow);
    default:
        return g.NavWindow == g.CurrentWindow;
    }
}


static inline bool IsWindowContentHoverable(ImGuiWindow *window, ImGuiHoveredFlags flags)
{
    // An active popup disable hovering on other windows (apart from its own children)
    // FIXME-OPT: This could be cached/stored within the window.
    ImGuiContext &g = *GImGui;
    if (g.NavWindow)
        if (ImGuiWindow *focused_root_window = g.NavWindow->RootWindow)
            if (focused_root_window->WasActive && focused_root_window != window->RootWindow)
            {
                // For the purpose of those flags we differentiate "standard popup" from "modal popup"
                // NB: The order of those two tests is important because Modal windows are also Popups.
                if (focused_root_window->Flags & ImGuiWindowFlags_Modal)
                    return false;
                if ((focused_root_window->Flags & ImGuiWindowFlags_Popup) &&
                    !(flags & ImGuiHoveredFlags_AllowWhenBlockedByPopup))
                    return false;
            }
    return true;
}

bool ImGui::IsWindowHovered(ImGuiHoveredFlags flags)
{
    IM_ASSERT((flags & ImGuiHoveredFlags_AllowWhenOverlapped) == 0); // Flags not supported by this function
    ImGuiContext &g = *GImGui;
    if (g.HoveredWindow == NULL)
        return false;

    if ((flags & ImGuiHoveredFlags_AnyWindow) == 0)
    {
        ImGuiWindow *window = g.CurrentWindow;
        switch (flags & (ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows))
        {
        case ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows:
            if (g.HoveredWindow->RootWindow != window->RootWindow)
                return false;
            break;
        case ImGuiHoveredFlags_RootWindow:
            if (g.HoveredWindow != window->RootWindow)
                return false;
            break;
        case ImGuiHoveredFlags_ChildWindows:
            if (!IsWindowChildOf(g.HoveredWindow, window))
                return false;
            break;
        default:
            if (g.HoveredWindow != window)
                return false;
            break;
        }
    }

    if (!IsWindowContentHoverable(g.HoveredWindow, flags))
        return false;
    if (!(flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        if (g.ActiveId != 0 && !g.ActiveIdAllowOverlap && g.ActiveId != g.HoveredWindow->MoveId)
            return false;
    return true;
}

ImDrawList *ImGui::GetWindowDrawList()
{
    ImGuiWindow *window = GetCurrentWindow();
    return window->DrawList;
}

void ImGui::SetNextWindowSize(const ImVec2 &size, ImGuiCond cond)
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(cond == 0 ||
              ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasSize;
    g.NextWindowData.SizeVal = size;
    g.NextWindowData.SizeCond = cond ? cond : ImGuiCond_Always;
}


void ImGui::SetNextWindowSizeConstraints(const ImVec2 &size_min, const ImVec2 &size_max,
                                         ImGuiSizeCallback custom_callback, void *custom_callback_user_data)
{
    ImGuiContext &g = *GImGui;
    g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasSizeConstraint;
    g.NextWindowData.SizeConstraintRect = ImRect(size_min, size_max);
    g.NextWindowData.SizeCallback = custom_callback;
    g.NextWindowData.SizeCallbackUserData = custom_callback_user_data;
}

// Content size = inner scrollable rectangle, padded with WindowPadding.
// SetNextWindowContentSize(ImVec2(100,100) + ImGuiWindowFlags_AlwaysAutoResize will always allow submitting a 100x100
// item.
void ImGui::SetNextWindowContentSize(const ImVec2 &size)
{
    ImGuiContext &g = *GImGui;
    g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasContentSize;
    g.NextWindowData.ContentSizeVal = ImFloor(size);
}

void ImGui::SetNextWindowScroll(const ImVec2 &scroll)
{
    ImGuiContext &g = *GImGui;
    g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasScroll;
    g.NextWindowData.ScrollVal = scroll;
}

void ImGui::SetNextWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(cond == 0 ||
              ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasCollapsed;
    g.NextWindowData.CollapsedVal = collapsed;
    g.NextWindowData.CollapsedCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowFocus()
{
    ImGuiContext &g = *GImGui;
    g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasFocus;
}

void ImGui::SetNextWindowBgAlpha(float alpha)
{
    ImGuiContext &g = *GImGui;
    g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasBgAlpha;
    g.NextWindowData.BgAlphaVal = alpha;
}


void ImGui::SetWindowPos(const ImVec2 &pos, ImGuiCond cond)
{
    ImGuiWindow *window = GetCurrentWindowRead();
    SetWindowPos(window, pos, cond);
}

void ImGui::SetWindowPos(const char *name, const ImVec2 &pos, ImGuiCond cond)
{
    if (ImGuiWindow *window = FindWindowByName(name))
        SetWindowPos(window, pos, cond);
}

ImVec2 ImGui::GetWindowSize()
{
    ImGuiWindow *window = GetCurrentWindowRead();
    return window->Size;
}

void ImGui::SetWindowSize(ImGuiWindow *window, const ImVec2 &size, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowSizeAllowFlags & cond) == 0)
        return;

    IM_ASSERT(cond == 0 ||
              ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    window->SetWindowSizeAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    // Set
    if (size.x > 0.0f)
    {
        window->AutoFitFramesX = 0;
        window->SizeFull.x = IM_FLOOR(size.x);
    }
    else
    {
        window->AutoFitFramesX = 2;
        window->AutoFitOnlyGrows = false;
    }
    if (size.y > 0.0f)
    {
        window->AutoFitFramesY = 0;
        window->SizeFull.y = IM_FLOOR(size.y);
    }
    else
    {
        window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = false;
    }
}

void ImGui::SetWindowSize(const ImVec2 &size, ImGuiCond cond)
{
    SetWindowSize(GImGui->CurrentWindow, size, cond);
}

void ImGui::SetWindowSize(const char *name, const ImVec2 &size, ImGuiCond cond)
{
    if (ImGuiWindow *window = FindWindowByName(name))
        SetWindowSize(window, size, cond);
}

void ImGui::SetWindowCollapsed(ImGuiWindow *window, bool collapsed, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowCollapsedAllowFlags & cond) == 0)
        return;
    window->SetWindowCollapsedAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    // Set
    window->Collapsed = collapsed;
}

void ImGui::SetWindowHitTestHole(ImGuiWindow *window, const ImVec2 &pos, const ImVec2 &size)
{
    IM_ASSERT(window->HitTestHoleSize.x == 0); // We don't support multiple holes/hit test filters
    window->HitTestHoleSize = ImVec2ih(size);
    window->HitTestHoleOffset = ImVec2ih(pos - window->Pos);
}

void ImGui::SetWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    SetWindowCollapsed(GImGui->CurrentWindow, collapsed, cond);
}


void ImGui::SetWindowCollapsed(const char *name, bool collapsed, ImGuiCond cond)
{
    if (ImGuiWindow *window = FindWindowByName(name))
        SetWindowCollapsed(window, collapsed, cond);
}

void ImGui::SetWindowFocus()
{
    FocusWindow(GImGui->CurrentWindow);
}

void ImGui::SetWindowFocus(const char *name)
{
    if (name)
    {
        if (ImGuiWindow *window = FindWindowByName(name))
            FocusWindow(window);
    }
    else
    {
        FocusWindow(NULL);
    }
}

void ImGui::SetNextWindowPos(const ImVec2 &pos, ImGuiCond cond, const ImVec2 &pivot)
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(cond == 0 ||
              ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasPos;
    g.NextWindowData.PosVal = pos;
    g.NextWindowData.PosPivotVal = pivot;
    g.NextWindowData.PosCond = cond ? cond : ImGuiCond_Always;
}



ImGuiID ImGui::GetWindowScrollbarID(ImGuiWindow* window, ImGuiAxis axis)
{
    return window->GetIDNoKeepAlive(axis == ImGuiAxis_X ? "#SCROLLX" : "#SCROLLY");
}

// Return scrollbar rectangle, must only be called for corresponding axis if window->ScrollbarX/Y is set.
ImRect ImGui::GetWindowScrollbarRect(ImGuiWindow* window, ImGuiAxis axis)
{
    const ImRect outer_rect = window->Rect();
    const ImRect inner_rect = window->InnerRect;
    const float border_size = window->WindowBorderSize;
    const float scrollbar_size = window->ScrollbarSizes[axis ^ 1]; // (ScrollbarSizes.x = width of Y scrollbar; ScrollbarSizes.y = height of X scrollbar)
    IM_ASSERT(scrollbar_size > 0.0f);
    if (axis == ImGuiAxis_X)
        return ImRect(inner_rect.Min.x, ImMax(outer_rect.Min.y, outer_rect.Max.y - border_size - scrollbar_size), inner_rect.Max.x, outer_rect.Max.y);
    else
        return ImRect(ImMax(outer_rect.Min.x, outer_rect.Max.x - border_size - scrollbar_size), inner_rect.Min.y, outer_rect.Max.x, inner_rect.Max.y);
}
