
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>

#include <ImGui/ImGuiInternal.h>

struct ImGuiContext;
extern ImGuiContext *GImGui;

extern void CalcResizePosSizeFromAnyCorner(ImGuiWindow *window, const ImVec2 &corner_target, const ImVec2 &corner_norm,
                                           ImVec2 *out_pos, ImVec2 *out_size);

extern ImVec2 CalcWindowSizeAfterConstraint(ImGuiWindow *window, const ImVec2 &size_desired);
//----------------------------------------------------

//----------------------------------------------------

// Debug options
#define IMGUI_DEBUG_NAV_SCORING                                                                                        \
    0 // Display navigation scoring preview when hovering items. Display last moving direction matches when holding CTRL
#define IMGUI_DEBUG_NAV_RECTS 0 // Display the reference navigation rectangle for each window
#define IMGUI_DEBUG_INI_SETTINGS                                                                                       \
    0 // Save additional comments in .ini file (particularly helps for Docking, but makes saving slower)

// When using CTRL+TAB (or Gamepad Square+L/R) we delay the visual a little in order to reduce visual noise doing a fast
// switch.
static const float NAV_WINDOWING_HIGHLIGHT_DELAY =
    0.20f; // Time before the highlight and screen dimming starts fading in
static const float NAV_WINDOWING_LIST_APPEAR_DELAY = 0.15f; // Time before the window list starts to appear

// Window resizing from edges (when io.ConfigWindowsResizeFromEdges = true and ImGuiBackendFlags_HasMouseCursors is set
// in io.BackendFlags by backend)
static const float WINDOWS_HOVER_PADDING =
    4.0f; // Extend outside window for hovering/resizing (maxxed with TouchPadding) and inside windows for borders.
          // Affect FindHoveredWindow().
static const float WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER =
    0.04f; // Reduce visual noise by only highlighting the border after a certain time.
static const float WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER =
    2.00f; // Lock scrolled window (so it doesn't pick child windows that are scrolling through) for a certain time,
           // unless mouse moved.

// extern const ImGuiResizeGripDef resize_grip_def[4];

static const ImGuiResizeGripDef resize_grip_def[4] = {
    {ImVec2(1, 1), ImVec2(-1, -1), 0, 3}, // Lower-right
    {ImVec2(0, 1), ImVec2(+1, -1), 3, 6}, // Lower-left
    {ImVec2(0, 0), ImVec2(+1, +1), 6, 9}, // Upper-left (Unused)
    {ImVec2(1, 0), ImVec2(-1, +1), 9, 12} // Upper-right (Unused)
};

// extern const ImGuiResizeBorderDef resize_border_def[4];
static const ImGuiResizeBorderDef resize_border_def[4] = {
    {ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f}, // Left
    {ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f}, // Right
    {ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f}, // Up
    {ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f}  // Down
};

 void ImGui::NavUpdate()
{
    ImGuiContext &g = *GImGui;
    ImGuiIO &io = g.IO;

    io.WantSetMousePos = false;
    g.NavWrapRequestWindow = NULL;
    g.NavWrapRequestFlags = ImGuiNavMoveFlags_None;
#if 0
    if (g.NavScoringCount > 0) IMGUI_DEBUG_LOG("NavScoringCount %d for '%s' layer %d (Init:%d, Move:%d)\n", g.FrameCount, g.NavScoringCount, g.NavWindow ? g.NavWindow->Name : "NULL", g.NavLayer, g.NavInitRequest || g.NavInitResultId != 0, g.NavMoveRequest);
#endif

    // Set input source as Gamepad when buttons are pressed (as some features differs when used with Gamepad vs
    // Keyboard) (do it before we map Keyboard input!)
    bool nav_keyboard_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0;
    bool nav_gamepad_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 &&
                              (io.BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
    if (nav_gamepad_active && g.NavInputSource != ImGuiInputSource_Gamepad)
    {
        if (io.NavInputs[ImGuiNavInput_Activate] > 0.0f || io.NavInputs[ImGuiNavInput_Input] > 0.0f ||
            io.NavInputs[ImGuiNavInput_Cancel] > 0.0f || io.NavInputs[ImGuiNavInput_Menu] > 0.0f ||
            io.NavInputs[ImGuiNavInput_DpadLeft] > 0.0f || io.NavInputs[ImGuiNavInput_DpadRight] > 0.0f ||
            io.NavInputs[ImGuiNavInput_DpadUp] > 0.0f || io.NavInputs[ImGuiNavInput_DpadDown] > 0.0f)
            g.NavInputSource = ImGuiInputSource_Gamepad;
    }

    // Update Keyboard->Nav inputs mapping
    if (nav_keyboard_active)
    {
#define NAV_MAP_KEY(_KEY, _NAV_INPUT)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        if (IsKeyDown(io.KeyMap[_KEY]))                                                                                \
        {                                                                                                              \
            io.NavInputs[_NAV_INPUT] = 1.0f;                                                                           \
            g.NavInputSource = ImGuiInputSource_Keyboard;                                                              \
        }                                                                                                              \
    } while (0)
        NAV_MAP_KEY(ImGuiKey_Space, ImGuiNavInput_Activate);
        NAV_MAP_KEY(ImGuiKey_Enter, ImGuiNavInput_Input);
        NAV_MAP_KEY(ImGuiKey_Escape, ImGuiNavInput_Cancel);
        NAV_MAP_KEY(ImGuiKey_LeftArrow, ImGuiNavInput_KeyLeft_);
        NAV_MAP_KEY(ImGuiKey_RightArrow, ImGuiNavInput_KeyRight_);
        NAV_MAP_KEY(ImGuiKey_UpArrow, ImGuiNavInput_KeyUp_);
        NAV_MAP_KEY(ImGuiKey_DownArrow, ImGuiNavInput_KeyDown_);
        if (io.KeyCtrl)
            io.NavInputs[ImGuiNavInput_TweakSlow] = 1.0f;
        if (io.KeyShift)
            io.NavInputs[ImGuiNavInput_TweakFast] = 1.0f;

        // AltGR is normally Alt+Ctrl but we can't reliably detect it (not all backends/systems/layout emit it as
        // Alt+Ctrl) But also even on keyboards without AltGR we don't want Alt+Ctrl to open menu anyway.
        if (io.KeyAlt && !io.KeyCtrl)
            io.NavInputs[ImGuiNavInput_KeyMenu_] = 1.0f;

        // We automatically cancel toggling nav layer when any text has been typed while holding Alt. (See #370)
        if (io.KeyAlt && !io.KeyCtrl && g.NavWindowingToggleLayer && io.InputQueueCharacters.Size > 0)
            g.NavWindowingToggleLayer = false;

#undef NAV_MAP_KEY
    }
    memcpy(io.NavInputsDownDurationPrev, io.NavInputsDownDuration, sizeof(io.NavInputsDownDuration));
    for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++)
        io.NavInputsDownDuration[i] =
            (io.NavInputs[i] > 0.0f)
                ? (io.NavInputsDownDuration[i] < 0.0f ? 0.0f : io.NavInputsDownDuration[i] + io.DeltaTime)
                : -1.0f;

    // Process navigation init request (select first/default focus)
    if (g.NavInitResultId != 0)
        NavUpdateInitResult();
    g.NavInitRequest = false;
    g.NavInitRequestFromMove = false;
    g.NavInitResultId = 0;
    g.NavJustMovedToId = 0;

    // Process navigation move request
    if (g.NavMoveRequest)
        NavUpdateMoveResult();

    // When a forwarded move request failed, we restore the highlight that we disabled during the forward frame
    if (g.NavMoveRequestForward == ImGuiNavForward_ForwardActive)
    {
        IM_ASSERT(g.NavMoveRequest);
        if (g.NavMoveResultLocal.ID == 0 && g.NavMoveResultOther.ID == 0)
            g.NavDisableHighlight = false;
        g.NavMoveRequestForward = ImGuiNavForward_None;
    }

    // Apply application mouse position movement, after we had a chance to process move request result.
    if (g.NavMousePosDirty && g.NavIdIsAlive)
    {
        // Set mouse position given our knowledge of the navigated item position from last frame
        if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos) &&
            (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos))
            if (!g.NavDisableHighlight && g.NavDisableMouseHover && g.NavWindow)
            {
                io.MousePos = io.MousePosPrev = NavCalcPreferredRefPos();
                io.WantSetMousePos = true;
            }
        g.NavMousePosDirty = false;
    }
    g.NavIdIsAlive = false;
    g.NavJustTabbedId = 0;
    IM_ASSERT(g.NavLayer == 0 || g.NavLayer == 1);

    // Store our return window (for returning from Layer 1 to Layer 0) and clear it as soon as we step back in our own
    // Layer 0
    if (g.NavWindow)
        NavSaveLastChildNavWindowIntoParent(g.NavWindow);
    if (g.NavWindow && g.NavWindow->NavLastChildNavWindow != NULL && g.NavLayer == ImGuiNavLayer_Main)
        g.NavWindow->NavLastChildNavWindow = NULL;

    // Update CTRL+TAB and Windowing features (hold Square to move/resize/etc.)
    NavUpdateWindowing();

    // Set output flags for user application
    io.NavActive = (nav_keyboard_active || nav_gamepad_active) && g.NavWindow &&
                   !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs);
    io.NavVisible = (io.NavActive && g.NavId != 0 && !g.NavDisableHighlight) || (g.NavWindowingTarget != NULL);

    // Process NavCancel input (to close a popup, get back to parent, clear focus)
    if (IsNavInputTest(ImGuiNavInput_Cancel, ImGuiInputReadMode_Pressed))
    {
        IMGUI_DEBUG_LOG_NAV("[nav] ImGuiNavInput_Cancel\n");
        if (g.ActiveId != 0)
        {
            if (!IsActiveIdUsingNavInput(ImGuiNavInput_Cancel))
                ClearActiveID();
        }
        else if (g.NavLayer != ImGuiNavLayer_Main)
        {
            // Leave the "menu" layer
            NavRestoreLayer(ImGuiNavLayer_Main);
        }
        else if (g.NavWindow && g.NavWindow != g.NavWindow->RootWindow &&
                 !(g.NavWindow->Flags & ImGuiWindowFlags_Popup) && g.NavWindow->ParentWindow)
        {
            // Exit child window
            ImGuiWindow *child_window = g.NavWindow;
            ImGuiWindow *parent_window = g.NavWindow->ParentWindow;
            IM_ASSERT(child_window->ChildId != 0);
            ImRect child_rect = child_window->Rect();
            FocusWindow(parent_window);
            SetNavID(child_window->ChildId, ImGuiNavLayer_Main, 0,
                     ImRect(child_rect.Min - parent_window->Pos, child_rect.Max - parent_window->Pos));
        }
        else if (g.OpenPopupStack.Size > 0)
        {
            // Close open popup/menu
            if (!(g.OpenPopupStack.back().Window->Flags & ImGuiWindowFlags_Modal))
                ClosePopupToLevel(g.OpenPopupStack.Size - 1, true);
        }
        else
        {
            // Clear NavLastId for popups but keep it for regular child window so we can leave one and come back where
            // we were
            if (g.NavWindow &&
                ((g.NavWindow->Flags & ImGuiWindowFlags_Popup) || !(g.NavWindow->Flags & ImGuiWindowFlags_ChildWindow)))
                g.NavWindow->NavLastIds[0] = 0;
            g.NavId = g.NavFocusScopeId = 0;
        }
    }

    // Process manual activation request
    g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId = g.NavInputId = 0;
    if (g.NavId != 0 && !g.NavDisableHighlight && !g.NavWindowingTarget && g.NavWindow &&
        !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
    {
        bool activate_down = IsNavInputDown(ImGuiNavInput_Activate);
        bool activate_pressed = activate_down && IsNavInputTest(ImGuiNavInput_Activate, ImGuiInputReadMode_Pressed);
        if (g.ActiveId == 0 && activate_pressed)
            g.NavActivateId = g.NavId;
        if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && activate_down)
            g.NavActivateDownId = g.NavId;
        if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && activate_pressed)
            g.NavActivatePressedId = g.NavId;
        if ((g.ActiveId == 0 || g.ActiveId == g.NavId) &&
            IsNavInputTest(ImGuiNavInput_Input, ImGuiInputReadMode_Pressed))
            g.NavInputId = g.NavId;
    }
    if (g.NavWindow && (g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
        g.NavDisableHighlight = true;
    if (g.NavActivateId != 0)
        IM_ASSERT(g.NavActivateDownId == g.NavActivateId);
    g.NavMoveRequest = false;

    // Process programmatic activation request
    if (g.NavNextActivateId != 0)
        g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId = g.NavInputId = g.NavNextActivateId;
    g.NavNextActivateId = 0;

    // Initiate directional inputs request
    if (g.NavMoveRequestForward == ImGuiNavForward_None)
    {
        g.NavMoveDir = ImGuiDir_None;
        g.NavMoveRequestFlags = ImGuiNavMoveFlags_None;
        if (g.NavWindow && !g.NavWindowingTarget && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
        {
            const ImGuiInputReadMode read_mode = ImGuiInputReadMode_Repeat;
            if (!IsActiveIdUsingNavDir(ImGuiDir_Left) && (IsNavInputTest(ImGuiNavInput_DpadLeft, read_mode) ||
                                                          IsNavInputTest(ImGuiNavInput_KeyLeft_, read_mode)))
            {
                g.NavMoveDir = ImGuiDir_Left;
            }
            if (!IsActiveIdUsingNavDir(ImGuiDir_Right) && (IsNavInputTest(ImGuiNavInput_DpadRight, read_mode) ||
                                                           IsNavInputTest(ImGuiNavInput_KeyRight_, read_mode)))
            {
                g.NavMoveDir = ImGuiDir_Right;
            }
            if (!IsActiveIdUsingNavDir(ImGuiDir_Up) &&
                (IsNavInputTest(ImGuiNavInput_DpadUp, read_mode) || IsNavInputTest(ImGuiNavInput_KeyUp_, read_mode)))
            {
                g.NavMoveDir = ImGuiDir_Up;
            }
            if (!IsActiveIdUsingNavDir(ImGuiDir_Down) && (IsNavInputTest(ImGuiNavInput_DpadDown, read_mode) ||
                                                          IsNavInputTest(ImGuiNavInput_KeyDown_, read_mode)))
            {
                g.NavMoveDir = ImGuiDir_Down;
            }
        }
        g.NavMoveClipDir = g.NavMoveDir;
    }
    else
    {
        // Forwarding previous request (which has been modified, e.g. wrap around menus rewrite the requests with a
        // starting rectangle at the other side of the window) (Preserve g.NavMoveRequestFlags, g.NavMoveClipDir which
        // were set by the NavMoveRequestForward() function)
        IM_ASSERT(g.NavMoveDir != ImGuiDir_None && g.NavMoveClipDir != ImGuiDir_None);
        IM_ASSERT(g.NavMoveRequestForward == ImGuiNavForward_ForwardQueued);
        IMGUI_DEBUG_LOG_NAV("[nav] NavMoveRequestForward %d\n", g.NavMoveDir);
        g.NavMoveRequestForward = ImGuiNavForward_ForwardActive;
    }

    // Update PageUp/PageDown/Home/End scroll
    // FIXME-NAV: Consider enabling those keys even without the master ImGuiConfigFlags_NavEnableKeyboard flag?
    float nav_scoring_rect_offset_y = 0.0f;
    if (nav_keyboard_active)
        nav_scoring_rect_offset_y = NavUpdatePageUpPageDown();

    // If we initiate a movement request and have no current NavId, we initiate a InitDefautRequest that will be used as
    // a fallback if the direction fails to find a match
    if (g.NavMoveDir != ImGuiDir_None)
    {
        g.NavMoveRequest = true;
        g.NavMoveRequestKeyMods = io.KeyMods;
        g.NavMoveDirLast = g.NavMoveDir;
    }
    if (g.NavMoveRequest && g.NavId == 0)
    {
        IMGUI_DEBUG_LOG_NAV("[nav] NavInitRequest: from move, window \"%s\", layer=%d\n", g.NavWindow->Name,
                            g.NavLayer);
        g.NavInitRequest = g.NavInitRequestFromMove = true;
        // Reassigning with same value, we're being explicit here.
        g.NavInitResultId = 0; // -V1048
        g.NavDisableHighlight = false;
    }
    NavUpdateAnyRequestFlag();

    // Scrolling
    if (g.NavWindow && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs) && !g.NavWindowingTarget)
    {
        // *Fallback* manual-scroll with Nav directional keys when window has no navigable item
        ImGuiWindow *window = g.NavWindow;
        const float scroll_speed = IM_ROUND(
            window->CalcFontSize() * 100 *
            io.DeltaTime); // We need round the scrolling speed because sub-pixel scroll isn't reliably supported.
        if (window->DC.NavLayersActiveMask == 0x00 && window->DC.NavHasScroll && g.NavMoveRequest)
        {
            if (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right)
                SetScrollX(window, ImFloor(window->Scroll.x +
                                           ((g.NavMoveDir == ImGuiDir_Left) ? -1.0f : +1.0f) * scroll_speed));
            if (g.NavMoveDir == ImGuiDir_Up || g.NavMoveDir == ImGuiDir_Down)
                SetScrollY(window,
                           ImFloor(window->Scroll.y + ((g.NavMoveDir == ImGuiDir_Up) ? -1.0f : +1.0f) * scroll_speed));
        }

        // *Normal* Manual scroll with NavScrollXXX keys
        // Next movement request will clamp the NavId reference rectangle to the visible area, so navigation will resume
        // within those bounds.
        ImVec2 scroll_dir =
            GetNavInputAmount2d(ImGuiNavDirSourceFlags_PadLStick, ImGuiInputReadMode_Down, 1.0f / 10.0f, 10.0f);
        if (scroll_dir.x != 0.0f && window->ScrollbarX)
            SetScrollX(window, ImFloor(window->Scroll.x + scroll_dir.x * scroll_speed));
        if (scroll_dir.y != 0.0f)
            SetScrollY(window, ImFloor(window->Scroll.y + scroll_dir.y * scroll_speed));
    }

    // Reset search results
    g.NavMoveResultLocal.Clear();
    g.NavMoveResultLocalVisibleSet.Clear();
    g.NavMoveResultOther.Clear();

    // When using gamepad, we project the reference nav bounding box into window visible area.
    // This is to allow resuming navigation inside the visible area after doing a large amount of scrolling, since with
    // gamepad every movements are relative (can't focus a visible object like we can with the mouse).
    if (g.NavMoveRequest && g.NavInputSource == ImGuiInputSource_Gamepad && g.NavLayer == ImGuiNavLayer_Main)
    {
        ImGuiWindow *window = g.NavWindow;
        ImRect window_rect_rel(window->InnerRect.Min - window->Pos - ImVec2(1, 1),
                               window->InnerRect.Max - window->Pos + ImVec2(1, 1));
        if (!window_rect_rel.Contains(window->NavRectRel[g.NavLayer]))
        {
            IMGUI_DEBUG_LOG_NAV("[nav] NavMoveRequest: clamp NavRectRel\n");
            float pad = window->CalcFontSize() * 0.5f;
            window_rect_rel.Expand(
                ImVec2(-ImMin(window_rect_rel.GetWidth(), pad),
                       -ImMin(window_rect_rel.GetHeight(), pad))); // Terrible approximation for the intent of starting
                                                                   // navigation from first fully visible item
            window->NavRectRel[g.NavLayer].ClipWithFull(window_rect_rel);
            g.NavId = g.NavFocusScopeId = 0;
        }
    }

    // For scoring we use a single segment on the left side our current item bounding box (not touching the edge to
    // avoid box overlap with zero-spaced items)
    ImRect nav_rect_rel = g.NavWindow && !g.NavWindow->NavRectRel[g.NavLayer].IsInverted()
                              ? g.NavWindow->NavRectRel[g.NavLayer]
                              : ImRect(0, 0, 0, 0);
    g.NavScoringRect = g.NavWindow ? ImRect(g.NavWindow->Pos + nav_rect_rel.Min, g.NavWindow->Pos + nav_rect_rel.Max)
                                   : ImRect(0, 0, 0, 0);
    g.NavScoringRect.TranslateY(nav_scoring_rect_offset_y);
    g.NavScoringRect.Min.x = ImMin(g.NavScoringRect.Min.x + 1.0f, g.NavScoringRect.Max.x);
    g.NavScoringRect.Max.x = g.NavScoringRect.Min.x;
    IM_ASSERT(!g.NavScoringRect.IsInverted()); // Ensure if we have a finite, non-inverted bounding box here will allows
                                               // us to remove extraneous ImFabs() calls in NavScoreItem().
    // GetForegroundDrawList()->AddRect(g.NavScoringRectScreen.Min, g.NavScoringRectScreen.Max,
    // IM_COL32(255,200,0,255)); // [DEBUG]
    g.NavScoringCount = 0;
#if IMGUI_DEBUG_NAV_RECTS
    if (g.NavWindow)
    {
        ImDrawList *draw_list = GetForegroundDrawList(g.NavWindow);
        if (1)
        {
            for (int layer = 0; layer < 2; layer++)
                draw_list->AddRect(g.NavWindow->Pos + g.NavWindow->NavRectRel[layer].Min,
                                   g.NavWindow->Pos + g.NavWindow->NavRectRel[layer].Max, IM_COL32(255, 200, 0, 255));
        } // [DEBUG]
        if (1)
        {
            ImU32 col = (!g.NavWindow->Hidden) ? IM_COL32(255, 0, 255, 255) : IM_COL32(255, 0, 0, 255);
            ImVec2 p = NavCalcPreferredRefPos();
            char buf[32];
            ImFormatString(buf, 32, "%d", g.NavLayer);
            draw_list->AddCircleFilled(p, 3.0f, col);
            draw_list->AddText(NULL, 13.0f, p + ImVec2(8, -4), col, buf);
        }
    }
#endif
}

 ImGuiWindow *FindWindowNavFocusable(int i_start, int i_stop, int dir) // FIXME-OPT O(N)
{
    ImGuiContext &g = *GImGui;
    for (int i = i_start; i >= 0 && i < g.WindowsFocusOrder.Size && i != i_stop; i += dir)
        if (ImGui::IsWindowNavFocusable(g.WindowsFocusOrder[i]))
            return g.WindowsFocusOrder[i];
    return NULL;
}

 void NavUpdateWindowingHighlightWindow(int focus_change_dir)
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(g.NavWindowingTarget);
    if (g.NavWindowingTarget->Flags & ImGuiWindowFlags_Modal)
        return;

    const int i_current = ImGui::FindWindowFocusIndex(g.NavWindowingTarget);
    ImGuiWindow *window_target = FindWindowNavFocusable(i_current + focus_change_dir, -INT_MAX, focus_change_dir);
    if (!window_target)
        window_target = FindWindowNavFocusable((focus_change_dir < 0) ? (g.WindowsFocusOrder.Size - 1) : 0, i_current,
                                               focus_change_dir);
    if (window_target) // Don't reset windowing target if there's a single window in the list
        g.NavWindowingTarget = g.NavWindowingTargetAnim = window_target;
    g.NavWindowingToggleLayer = false;
}

// Windowing management mode
// Keyboard: CTRL+Tab (change focus/move/resize), Alt (toggle menu layer)
// Gamepad:  Hold Menu/Square (change focus/move/resize), Tap Menu/Square (toggle menu layer)
 void ImGui::NavUpdateWindowing()
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *apply_focus_window = NULL;
    bool apply_toggle_layer = false;

    ImGuiWindow *modal_window = GetTopMostPopupModal();
    bool allow_windowing = (modal_window == NULL);
    if (!allow_windowing)
        g.NavWindowingTarget = NULL;

    // Fade out
    if (g.NavWindowingTargetAnim && g.NavWindowingTarget == NULL)
    {
        g.NavWindowingHighlightAlpha = ImMax(g.NavWindowingHighlightAlpha - g.IO.DeltaTime * 10.0f, 0.0f);
        if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
            g.NavWindowingTargetAnim = NULL;
    }

    // Start CTRL-TAB or Square+L/R window selection
    bool start_windowing_with_gamepad =
        allow_windowing && !g.NavWindowingTarget && IsNavInputTest(ImGuiNavInput_Menu, ImGuiInputReadMode_Pressed);
    bool start_windowing_with_keyboard = allow_windowing && !g.NavWindowingTarget && g.IO.KeyCtrl &&
                                         IsKeyPressedMap(ImGuiKey_Tab) &&
                                         (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard);
    if (start_windowing_with_gamepad || start_windowing_with_keyboard)
        if (ImGuiWindow *window =
                g.NavWindow ? g.NavWindow : FindWindowNavFocusable(g.WindowsFocusOrder.Size - 1, -INT_MAX, -1))
        {
            g.NavWindowingTarget = g.NavWindowingTargetAnim =
                window->RootWindow; // FIXME-DOCK: Will need to use RootWindowDockStop
            g.NavWindowingTimer = g.NavWindowingHighlightAlpha = 0.0f;
            g.NavWindowingToggleLayer = start_windowing_with_keyboard ? false : true;
            g.NavInputSource = start_windowing_with_keyboard ? ImGuiInputSource_Keyboard : ImGuiInputSource_Gamepad;
        }

    // Gamepad update
    g.NavWindowingTimer += g.IO.DeltaTime;
    if (g.NavWindowingTarget && g.NavInputSource == ImGuiInputSource_Gamepad)
    {
        // Highlight only appears after a brief time holding the button, so that a fast tap on PadMenu (to toggle
        // NavLayer) doesn't add visual noise
        g.NavWindowingHighlightAlpha = ImMax(g.NavWindowingHighlightAlpha,
                                             ImSaturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) / 0.05f));

        // Select window to focus
        const int focus_change_dir = (int)IsNavInputTest(ImGuiNavInput_FocusPrev, ImGuiInputReadMode_RepeatSlow) -
                                     (int)IsNavInputTest(ImGuiNavInput_FocusNext, ImGuiInputReadMode_RepeatSlow);
        if (focus_change_dir != 0)
        {
            NavUpdateWindowingHighlightWindow(focus_change_dir);
            g.NavWindowingHighlightAlpha = 1.0f;
        }

        // Single press toggles NavLayer, long press with L/R apply actual focus on release (until then the window was
        // merely rendered top-most)
        if (!IsNavInputDown(ImGuiNavInput_Menu))
        {
            g.NavWindowingToggleLayer &=
                (g.NavWindowingHighlightAlpha <
                 1.0f); // Once button was held long enough we don't consider it a tap-to-toggle-layer press anymore.
            if (g.NavWindowingToggleLayer && g.NavWindow)
                apply_toggle_layer = true;
            else if (!g.NavWindowingToggleLayer)
                apply_focus_window = g.NavWindowingTarget;
            g.NavWindowingTarget = NULL;
        }
    }

    // Keyboard: Focus
    if (g.NavWindowingTarget && g.NavInputSource == ImGuiInputSource_Keyboard)
    {
        // Visuals only appears after a brief time after pressing TAB the first time, so that a fast CTRL+TAB doesn't
        // add visual noise
        g.NavWindowingHighlightAlpha =
            ImMax(g.NavWindowingHighlightAlpha,
                  ImSaturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) / 0.05f)); // 1.0f
        if (IsKeyPressedMap(ImGuiKey_Tab, true))
            NavUpdateWindowingHighlightWindow(g.IO.KeyShift ? +1 : -1);
        if (!g.IO.KeyCtrl)
            apply_focus_window = g.NavWindowingTarget;
    }

    // Keyboard: Press and Release ALT to toggle menu layer
    // FIXME: We lack an explicit IO variable for "is the imgui window focused", so compare mouse validity to detect the
    // common case of backend clearing releases all keys on ALT-TAB
    if (IsNavInputTest(ImGuiNavInput_KeyMenu_, ImGuiInputReadMode_Pressed))
        g.NavWindowingToggleLayer = true;
    if ((g.ActiveId == 0 || g.ActiveIdAllowOverlap) && g.NavWindowingToggleLayer &&
        IsNavInputTest(ImGuiNavInput_KeyMenu_, ImGuiInputReadMode_Released))
        if (IsMousePosValid(&g.IO.MousePos) == IsMousePosValid(&g.IO.MousePosPrev))
            apply_toggle_layer = true;

    // Move window
    if (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoMove))
    {
        ImVec2 move_delta;
        if (g.NavInputSource == ImGuiInputSource_Keyboard && !g.IO.KeyShift)
            move_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard, ImGuiInputReadMode_Down);
        if (g.NavInputSource == ImGuiInputSource_Gamepad)
            move_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_PadLStick, ImGuiInputReadMode_Down);
        if (move_delta.x != 0.0f || move_delta.y != 0.0f)
        {
            const float NAV_MOVE_SPEED = 800.0f;
            const float move_speed =
                ImFloor(NAV_MOVE_SPEED * g.IO.DeltaTime *
                        ImMin(g.IO.DisplayFramebufferScale.x,
                              g.IO.DisplayFramebufferScale.y)); // FIXME: Doesn't handle variable framerate very well
            ImGuiWindow *moving_window = g.NavWindowingTarget->RootWindow;
            SetWindowPos(moving_window, moving_window->Pos + move_delta * move_speed, ImGuiCond_Always);
            MarkIniSettingsDirty(moving_window);
            g.NavDisableMouseHover = true;
        }
    }

    // Apply final focus
    if (apply_focus_window && (g.NavWindow == NULL || apply_focus_window != g.NavWindow->RootWindow))
    {
        ClearActiveID();
        g.NavDisableHighlight = false;
        g.NavDisableMouseHover = true;
        apply_focus_window = NavRestoreLastChildNavWindow(apply_focus_window);
        ClosePopupsOverWindow(apply_focus_window, false);
        FocusWindow(apply_focus_window);
        if (apply_focus_window->NavLastIds[0] == 0)
            NavInitWindow(apply_focus_window, false);

        // If the window has ONLY a menu layer (no main layer), select it directly
        // Use NavLayersActiveMaskNext since windows didn't have a chance to be Begin()-ed on this frame,
        // so CTRL+Tab where the keys are only held for 1 frame will be able to use correct layers mask since
        // the target window as already been previewed once.
        // FIXME-NAV: This should be done in NavInit.. or in FocusWindow... However in both of those cases,
        // we won't have a guarantee that windows has been visible before and therefore NavLayersActiveMask*
        // won't be valid.
        if (apply_focus_window->DC.NavLayersActiveMaskNext == (1 << ImGuiNavLayer_Menu))
            g.NavLayer = ImGuiNavLayer_Menu;
    }
    if (apply_focus_window)
        g.NavWindowingTarget = NULL;

    // Apply menu/layer toggle
    if (apply_toggle_layer && g.NavWindow)
    {
        ClearActiveID();

        // Move to parent menu if necessary
        ImGuiWindow *new_nav_window = g.NavWindow;
        while (new_nav_window->ParentWindow &&
               (new_nav_window->DC.NavLayersActiveMask & (1 << ImGuiNavLayer_Menu)) == 0 &&
               (new_nav_window->Flags & ImGuiWindowFlags_ChildWindow) != 0 &&
               (new_nav_window->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu)) == 0)
            new_nav_window = new_nav_window->ParentWindow;
        if (new_nav_window != g.NavWindow)
        {
            ImGuiWindow *old_nav_window = g.NavWindow;
            FocusWindow(new_nav_window);
            new_nav_window->NavLastChildNavWindow = old_nav_window;
        }
        g.NavDisableHighlight = false;
        g.NavDisableMouseHover = true;

        // Reinitialize navigation when entering menu bar with the Alt key.
        const ImGuiNavLayer new_nav_layer = (g.NavWindow->DC.NavLayersActiveMask & (1 << ImGuiNavLayer_Menu))
                                                ? (ImGuiNavLayer)((int)g.NavLayer ^ 1)
                                                : ImGuiNavLayer_Main;
        if (new_nav_layer == ImGuiNavLayer_Menu)
            g.NavWindow->NavLastIds[new_nav_layer] = 0;
        NavRestoreLayer(new_nav_layer);
    }
}

// Window has already passed the IsWindowNavFocusable()
 const char *GetFallbackWindowNameForWindowingList(ImGuiWindow *window)
{
    if (window->Flags & ImGuiWindowFlags_Popup)
        return "(Popup)";
    if ((window->Flags & ImGuiWindowFlags_MenuBar) && strcmp(window->Name, "##MainMenuBar") == 0)
        return "(Main menu bar)";
    return "(Untitled)";
}
// Overlay displayed when using CTRL+TAB. Called by EndFrame().
void ImGui::NavUpdateWindowingOverlay()
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(g.NavWindowingTarget != NULL);

    if (g.NavWindowingTimer < NAV_WINDOWING_LIST_APPEAR_DELAY)
        return;

    if (g.NavWindowingListWindow == NULL)
        g.NavWindowingListWindow = FindWindowByName("###NavWindowingList");
    const ImGuiViewport *viewport = GetMainViewport();
    SetNextWindowSizeConstraints(ImVec2(viewport->Size.x * 0.20f, viewport->Size.y * 0.20f), ImVec2(FLT_MAX, FLT_MAX));
    SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    PushStyleVar(ImGuiStyleVar_WindowPadding, g.Style.WindowPadding * 2.0f);
    Begin("###NavWindowingList", NULL,
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize |
              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
              ImGuiWindowFlags_NoSavedSettings);
    for (int n = g.WindowsFocusOrder.Size - 1; n >= 0; n--)
    {
        ImGuiWindow *window = g.WindowsFocusOrder[n];
        IM_ASSERT(window != NULL); // Fix  analyzers
        if (!IsWindowNavFocusable(window))
            continue;
        const char *label = window->Name;
        if (label == FindRenderedTextEnd(label))
            label = GetFallbackWindowNameForWindowingList(window);
        Selectable(label, g.NavWindowingTarget == window);
    }
    End();
    PopStyleVar();
}

// Apply result from previous frame navigation directional move request
 void ImGui::NavUpdateMoveResult()
{
    ImGuiContext &g = *GImGui;
    if (g.NavMoveResultLocal.ID == 0 && g.NavMoveResultOther.ID == 0)
    {
        // In a situation when there is no results but NavId != 0, re-enable the Navigation highlight (because g.NavId
        // is not considered as a possible result)
        if (g.NavId != 0)
        {
            g.NavDisableHighlight = false;
            g.NavDisableMouseHover = true;
        }
        return;
    }

    // Select which result to use
    ImGuiNavItemData *result = (g.NavMoveResultLocal.ID != 0) ? &g.NavMoveResultLocal : &g.NavMoveResultOther;

    // PageUp/PageDown behavior first jumps to the bottom/top mostly visible item, _otherwise_ use the result from the
    // previous/next page.
    if (g.NavMoveRequestFlags & ImGuiNavMoveFlags_AlsoScoreVisibleSet)
        if (g.NavMoveResultLocalVisibleSet.ID != 0 && g.NavMoveResultLocalVisibleSet.ID != g.NavId)
            result = &g.NavMoveResultLocalVisibleSet;

    // Maybe entering a flattened child from the outside? In this case solve the tie using the regular scoring rules.
    if (result != &g.NavMoveResultOther && g.NavMoveResultOther.ID != 0 &&
        g.NavMoveResultOther.Window->ParentWindow == g.NavWindow)
        if ((g.NavMoveResultOther.DistBox < result->DistBox) ||
            (g.NavMoveResultOther.DistBox == result->DistBox && g.NavMoveResultOther.DistCenter < result->DistCenter))
            result = &g.NavMoveResultOther;
    IM_ASSERT(g.NavWindow && result->Window);

    // Scroll to keep newly navigated item fully into view.
    if (g.NavLayer == ImGuiNavLayer_Main)
    {
        ImVec2 delta_scroll;
        if (g.NavMoveRequestFlags & ImGuiNavMoveFlags_ScrollToEdge)
        {
            float scroll_target = (g.NavMoveDir == ImGuiDir_Up) ? result->Window->ScrollMax.y : 0.0f;
            delta_scroll.y = result->Window->Scroll.y - scroll_target;
            SetScrollY(result->Window, scroll_target);
        }
        else
        {
            ImRect rect_abs =
                ImRect(result->RectRel.Min + result->Window->Pos, result->RectRel.Max + result->Window->Pos);
            delta_scroll = ScrollToBringRectIntoView(result->Window, rect_abs);
        }

        // Offset our result position so mouse position can be applied immediately after in NavUpdate()
        result->RectRel.TranslateX(-delta_scroll.x);
        result->RectRel.TranslateY(-delta_scroll.y);
    }

    ClearActiveID();
    g.NavWindow = result->Window;
    if (g.NavId != result->ID)
    {
        // Don't set NavJustMovedToId if just landed on the same spot (which may happen with
        // ImGuiNavMoveFlags_AllowCurrentNavId)
        g.NavJustMovedToId = result->ID;
        g.NavJustMovedToFocusScopeId = result->FocusScopeId;
        g.NavJustMovedToKeyMods = g.NavMoveRequestKeyMods;
    }
    IMGUI_DEBUG_LOG_NAV("[nav] NavMoveRequest: result NavID 0x%08X in Layer %d Window \"%s\"\n", result->ID, g.NavLayer,
                        g.NavWindow->Name);
    SetNavID(result->ID, g.NavLayer, result->FocusScopeId, result->RectRel);
    g.NavDisableHighlight = false;
    g.NavDisableMouseHover = g.NavMousePosDirty = true;
}

 void ImGui::NavUpdateInitResult()
{
    // In very rare cases g.NavWindow may be null (e.g. clearing focus after requesting an init request, which does
    // happen when releasing Alt while clicking on void)
    ImGuiContext &g = *GImGui;
    if (!g.NavWindow)
        return;

    // Apply result from previous navigation init request (will typically select the first item, unless
    // SetItemDefaultFocus() has been called)
    // FIXME-NAV: On _NavFlattened windows, g.NavWindow will only be updated during subsequent frame. Not a problem
    // currently.
    IMGUI_DEBUG_LOG_NAV("[nav] NavInitRequest: result NavID 0x%08X in Layer %d Window \"%s\"\n", g.NavInitResultId,
                        g.NavLayer, g.NavWindow->Name);
    SetNavID(g.NavInitResultId, g.NavLayer, 0, g.NavInitResultRectRel);
    if (g.NavInitRequestFromMove)
    {
        g.NavDisableHighlight = false;
        g.NavDisableMouseHover = g.NavMousePosDirty = true;
    }
}

// Handle PageUp/PageDown/Home/End keys
 float ImGui::NavUpdatePageUpPageDown()
{
    ImGuiContext &g = *GImGui;
    ImGuiIO &io = g.IO;

    if (g.NavMoveDir != ImGuiDir_None || g.NavWindow == NULL)
        return 0.0f;
    if ((g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs) || g.NavWindowingTarget != NULL ||
        g.NavLayer != ImGuiNavLayer_Main)
        return 0.0f;

    ImGuiWindow *window = g.NavWindow;
    const bool page_up_held = IsKeyDown(io.KeyMap[ImGuiKey_PageUp]) && !IsActiveIdUsingKey(ImGuiKey_PageUp);
    const bool page_down_held = IsKeyDown(io.KeyMap[ImGuiKey_PageDown]) && !IsActiveIdUsingKey(ImGuiKey_PageDown);
    const bool home_pressed = IsKeyPressed(io.KeyMap[ImGuiKey_Home]) && !IsActiveIdUsingKey(ImGuiKey_Home);
    const bool end_pressed = IsKeyPressed(io.KeyMap[ImGuiKey_End]) && !IsActiveIdUsingKey(ImGuiKey_End);
    if (page_up_held != page_down_held || home_pressed != end_pressed) // If either (not both) are pressed
    {
        if (window->DC.NavLayersActiveMask == 0x00 && window->DC.NavHasScroll)
        {
            // Fallback manual-scroll when window has no navigable item
            if (IsKeyPressed(io.KeyMap[ImGuiKey_PageUp], true))
                SetScrollY(window, window->Scroll.y - window->InnerRect.GetHeight());
            else if (IsKeyPressed(io.KeyMap[ImGuiKey_PageDown], true))
                SetScrollY(window, window->Scroll.y + window->InnerRect.GetHeight());
            else if (home_pressed)
                SetScrollY(window, 0.0f);
            else if (end_pressed)
                SetScrollY(window, window->ScrollMax.y);
        }
        else
        {
            ImRect &nav_rect_rel = window->NavRectRel[g.NavLayer];
            const float page_offset_y =
                ImMax(0.0f, window->InnerRect.GetHeight() - window->CalcFontSize() * 1.0f + nav_rect_rel.GetHeight());
            float nav_scoring_rect_offset_y = 0.0f;
            if (IsKeyPressed(io.KeyMap[ImGuiKey_PageUp], true))
            {
                nav_scoring_rect_offset_y = -page_offset_y;
                g.NavMoveDir = ImGuiDir_Down; // Because our scoring rect is offset up, we request the down direction
                                              // (so we can always land on the last item)
                g.NavMoveClipDir = ImGuiDir_Up;
                g.NavMoveRequestFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_AlsoScoreVisibleSet;
            }
            else if (IsKeyPressed(io.KeyMap[ImGuiKey_PageDown], true))
            {
                nav_scoring_rect_offset_y = +page_offset_y;
                g.NavMoveDir = ImGuiDir_Up; // Because our scoring rect is offset down, we request the up direction (so
                                            // we can always land on the last item)
                g.NavMoveClipDir = ImGuiDir_Down;
                g.NavMoveRequestFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_AlsoScoreVisibleSet;
            }
            else if (home_pressed)
            {
                // FIXME-NAV: handling of Home/End is assuming that the top/bottom most item will be visible with
                // Scroll.y == 0/ScrollMax.y Scrolling will be handled via the ImGuiNavMoveFlags_ScrollToEdge flag, we
                // don't scroll immediately to avoid scrolling happening before nav result. Preserve current horizontal
                // position if we have any.
                nav_rect_rel.Min.y = nav_rect_rel.Max.y = -window->Scroll.y;
                if (nav_rect_rel.IsInverted())
                    nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
                g.NavMoveDir = ImGuiDir_Down;
                g.NavMoveRequestFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_ScrollToEdge;
            }
            else if (end_pressed)
            {
                nav_rect_rel.Min.y = nav_rect_rel.Max.y = window->ScrollMax.y + window->SizeFull.y - window->Scroll.y;
                if (nav_rect_rel.IsInverted())
                    nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
                g.NavMoveDir = ImGuiDir_Up;
                g.NavMoveRequestFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_ScrollToEdge;
            }
            return nav_scoring_rect_offset_y;
        }
    }
    return 0.0f;
}

 inline void ImGui::NavUpdateAnyRequestFlag()
{
    ImGuiContext &g = *GImGui;
    g.NavAnyRequest = g.NavMoveRequest || g.NavInitRequest || (IMGUI_DEBUG_NAV_SCORING && g.NavWindow != NULL);
    if (g.NavAnyRequest)
        IM_ASSERT(g.NavWindow != NULL);
}

 void ImGui::NavEndFrame()
{
    ImGuiContext &g = *GImGui;

    // Show CTRL+TAB list window
    if (g.NavWindowingTarget != NULL)
        NavUpdateWindowingOverlay();

    // Perform wrap-around in menus
    ImGuiWindow *window = g.NavWrapRequestWindow;
    ImGuiNavMoveFlags move_flags = g.NavWrapRequestFlags;
    if (window != NULL && g.NavWindow == window && NavMoveRequestButNoResultYet() &&
        g.NavMoveRequestForward == ImGuiNavForward_None && g.NavLayer == ImGuiNavLayer_Main)
    {
        IM_ASSERT(move_flags != 0); // No points calling this with no wrapping
        ImRect bb_rel = window->NavRectRel[0];

        ImGuiDir clip_dir = g.NavMoveDir;
        if (g.NavMoveDir == ImGuiDir_Left && (move_flags & (ImGuiNavMoveFlags_WrapX | ImGuiNavMoveFlags_LoopX)))
        {
            bb_rel.Min.x = bb_rel.Max.x =
                ImMax(window->SizeFull.x, window->ContentSize.x + window->WindowPadding.x * 2.0f) - window->Scroll.x;
            if (move_flags & ImGuiNavMoveFlags_WrapX)
            {
                bb_rel.TranslateY(-bb_rel.GetHeight());
                clip_dir = ImGuiDir_Up;
            }
            NavMoveRequestForward(g.NavMoveDir, clip_dir, bb_rel, move_flags);
        }
        if (g.NavMoveDir == ImGuiDir_Right && (move_flags & (ImGuiNavMoveFlags_WrapX | ImGuiNavMoveFlags_LoopX)))
        {
            bb_rel.Min.x = bb_rel.Max.x = -window->Scroll.x;
            if (move_flags & ImGuiNavMoveFlags_WrapX)
            {
                bb_rel.TranslateY(+bb_rel.GetHeight());
                clip_dir = ImGuiDir_Down;
            }
            NavMoveRequestForward(g.NavMoveDir, clip_dir, bb_rel, move_flags);
        }
        if (g.NavMoveDir == ImGuiDir_Up && (move_flags & (ImGuiNavMoveFlags_WrapY | ImGuiNavMoveFlags_LoopY)))
        {
            bb_rel.Min.y = bb_rel.Max.y =
                ImMax(window->SizeFull.y, window->ContentSize.y + window->WindowPadding.y * 2.0f) - window->Scroll.y;
            if (move_flags & ImGuiNavMoveFlags_WrapY)
            {
                bb_rel.TranslateX(-bb_rel.GetWidth());
                clip_dir = ImGuiDir_Left;
            }
            NavMoveRequestForward(g.NavMoveDir, clip_dir, bb_rel, move_flags);
        }
        if (g.NavMoveDir == ImGuiDir_Down && (move_flags & (ImGuiNavMoveFlags_WrapY | ImGuiNavMoveFlags_LoopY)))
        {
            bb_rel.Min.y = bb_rel.Max.y = -window->Scroll.y;
            if (move_flags & ImGuiNavMoveFlags_WrapY)
            {
                bb_rel.TranslateX(+bb_rel.GetWidth());
                clip_dir = ImGuiDir_Right;
            }
            NavMoveRequestForward(g.NavMoveDir, clip_dir, bb_rel, move_flags);
        }
    }
}

 void inline NavClampRectToVisibleAreaForMoveDir(ImGuiDir move_dir, ImRect &r, const ImRect &clip_rect)
{
    if (move_dir == ImGuiDir_Left || move_dir == ImGuiDir_Right)
    {
        r.Min.y = ImClamp(r.Min.y, clip_rect.Min.y, clip_rect.Max.y);
        r.Max.y = ImClamp(r.Max.y, clip_rect.Min.y, clip_rect.Max.y);
    }
    else
    {
        r.Min.x = ImClamp(r.Min.x, clip_rect.Min.x, clip_rect.Max.x);
        r.Max.x = ImClamp(r.Max.x, clip_rect.Min.x, clip_rect.Max.x);
    }
}

 float inline NavScoreItemDistInterval(float a0, float a1, float b0, float b1)
{
    if (a1 < b0)
        return a1 - b0;
    if (b1 < a0)
        return a0 - b1;
    return 0.0f;
}

// Scoring function for gamepad/keyboard directional navigation. Based on https://gist.github.com/rygorous/6981057
 bool ImGui::NavScoreItem(ImGuiNavItemData *result, ImRect cand)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    if (g.NavLayer != window->DC.NavLayerCurrent)
        return false;

    const ImRect &curr = g.NavScoringRect; // Current modified source rect (NB: we've applied Max.x = Min.x in
                                           // NavUpdate() to inhibit the effect of having varied item width)
    g.NavScoringCount++;

    // When entering through a NavFlattened border, we consider child window items as fully clipped for scoring
    if (window->ParentWindow == g.NavWindow)
    {
        IM_ASSERT((window->Flags | g.NavWindow->Flags) & ImGuiWindowFlags_NavFlattened);
        if (!window->ClipRect.Overlaps(cand))
            return false;
        cand.ClipWithFull(
            window->ClipRect); // This allows the scored item to not overlap other candidates in the parent window
    }

    // We perform scoring on items bounding box clipped by the current clipping rectangle on the other axis (clipping on
    // our movement axis would give us equal scores for all clipped items) For example, this ensure that items in one
    // column are not reached when moving vertically from items in another column.
    NavClampRectToVisibleAreaForMoveDir(g.NavMoveClipDir, cand, window->ClipRect);

    // Compute distance between boxes
    // FIXME-NAV: Introducing biases for vertical navigation, needs to be removed.
    float dbx = NavScoreItemDistInterval(cand.Min.x, cand.Max.x, curr.Min.x, curr.Max.x);
    float dby = NavScoreItemDistInterval(
        ImLerp(cand.Min.y, cand.Max.y, 0.2f), ImLerp(cand.Min.y, cand.Max.y, 0.8f),
        ImLerp(curr.Min.y, curr.Max.y, 0.2f),
        ImLerp(curr.Min.y, curr.Max.y,
               0.8f)); // Scale down on Y to keep using box-distance for vertically touching items
    if (dby != 0.0f && dbx != 0.0f)
        dbx = (dbx / 1000.0f) + ((dbx > 0.0f) ? +1.0f : -1.0f);
    float dist_box = ImFabs(dbx) + ImFabs(dby);

    // Compute distance between centers (this is off by a factor of 2, but we only compare center distances with each
    // other so it doesn't matter)
    float dcx = (cand.Min.x + cand.Max.x) - (curr.Min.x + curr.Max.x);
    float dcy = (cand.Min.y + cand.Max.y) - (curr.Min.y + curr.Max.y);
    float dist_center = ImFabs(dcx) + ImFabs(dcy); // L1 metric (need this for our connectedness guarantee)

    // Determine which quadrant of 'curr' our candidate item 'cand' lies in based on distance
    ImGuiDir quadrant;
    float dax = 0.0f, day = 0.0f, dist_axial = 0.0f;
    if (dbx != 0.0f || dby != 0.0f)
    {
        // For non-overlapping boxes, use distance between boxes
        dax = dbx;
        day = dby;
        dist_axial = dist_box;
        quadrant = ImGetDirQuadrantFromDelta(dbx, dby);
    }
    else if (dcx != 0.0f || dcy != 0.0f)
    {
        // For overlapping boxes with different centers, use distance between centers
        dax = dcx;
        day = dcy;
        dist_axial = dist_center;
        quadrant = ImGetDirQuadrantFromDelta(dcx, dcy);
    }
    else
    {
        // Degenerate case: two overlapping buttons with same center, break ties arbitrarily (note that LastItemId here
        // is really the _previous_ item order, but it doesn't matter)
        quadrant = (window->DC.LastItemId < g.NavId) ? ImGuiDir_Left : ImGuiDir_Right;
    }

#if IMGUI_DEBUG_NAV_SCORING
    char buf[128];
    if (IsMouseHoveringRect(cand.Min, cand.Max))
    {
        ImFormatString(buf, IM_ARRAYSIZE(buf),
                       "dbox (%.2f,%.2f->%.4f)\ndcen (%.2f,%.2f->%.4f)\nd (%.2f,%.2f->%.4f)\nnav %c, quadrant %c", dbx,
                       dby, dist_box, dcx, dcy, dist_center, dax, day, dist_axial, "WENS"[g.NavMoveDir],
                       "WENS"[quadrant]);
        ImDrawList *draw_list = GetForegroundDrawList(window);
        draw_list->AddRect(curr.Min, curr.Max, IM_COL32(255, 200, 0, 100));
        draw_list->AddRect(cand.Min, cand.Max, IM_COL32(255, 255, 0, 200));
        draw_list->AddRectFilled(cand.Max - ImVec2(4, 4), cand.Max + CalcTextSize(buf) + ImVec2(4, 4),
                                 IM_COL32(40, 0, 0, 150));
        draw_list->AddText(g.IO.FontDefault, 13.0f, cand.Max, ~0U, buf);
    }
    else if (g.IO.KeyCtrl) // Hold to preview score in matching quadrant. Press C to rotate.
    {
        if (IsKeyPressedMap(ImGuiKey_C))
        {
            g.NavMoveDirLast = (ImGuiDir)((g.NavMoveDirLast + 1) & 3);
            g.IO.KeysDownDuration[g.IO.KeyMap[ImGuiKey_C]] = 0.01f;
        }
        if (quadrant == g.NavMoveDir)
        {
            ImFormatString(buf, IM_ARRAYSIZE(buf), "%.0f/%.0f", dist_box, dist_center);
            ImDrawList *draw_list = GetForegroundDrawList(window);
            draw_list->AddRectFilled(cand.Min, cand.Max, IM_COL32(255, 0, 0, 200));
            draw_list->AddText(g.IO.FontDefault, 13.0f, cand.Min, IM_COL32(255, 255, 255, 255), buf);
        }
    }
#endif

    // Is it in the quadrant we're interesting in moving to?
    bool new_best = false;
    if (quadrant == g.NavMoveDir)
    {
        // Does it beat the current best candidate?
        if (dist_box < result->DistBox)
        {
            result->DistBox = dist_box;
            result->DistCenter = dist_center;
            return true;
        }
        if (dist_box == result->DistBox)
        {
            // Try using distance between center points to break ties
            if (dist_center < result->DistCenter)
            {
                result->DistCenter = dist_center;
                new_best = true;
            }
            else if (dist_center == result->DistCenter)
            {
                // Still tied! we need to be extra-careful to make sure everything gets linked properly. We consistently
                // break ties by symbolically moving "later" items (with higher index) to the right/downwards by an
                // infinitesimal amount since we the current "best" button already (so it must have a lower index), this
                // is fairly easy. This rule ensures that all buttons with dx==dy==0 will end up being linked in order
                // of appearance along the x axis.
                if (((g.NavMoveDir == ImGuiDir_Up || g.NavMoveDir == ImGuiDir_Down) ? dby : dbx) <
                    0.0f) // moving bj to the right/down decreases distance
                    new_best = true;
            }
        }
    }

    // Axial check: if 'curr' has no link at all in some direction and 'cand' lies roughly in that direction, add a
    // tentative link. This will only be kept if no "real" matches are found, so it only augments the graph produced by
    // the above method using extra links. (important, since it doesn't guarantee strong connectedness) This is just to
    // avoid buttons having no links in a particular direction when there's a suitable neighbor. you get good graphs
    // without this too. 2017/09/29: FIXME: This now currently only enabled inside menu bars, ideally we'd disable it
    // everywhere. Menus in particular need to catch failure. For general navigation it feels awkward. Disabling it may
    // lead to disconnected graphs when nodes are very spaced out on different axis. Perhaps consider offering this as
    // an option?
    if (result->DistBox == FLT_MAX && dist_axial < result->DistAxial) // Check axial match
        if (g.NavLayer == ImGuiNavLayer_Menu && !(g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu))
            if ((g.NavMoveDir == ImGuiDir_Left && dax < 0.0f) || (g.NavMoveDir == ImGuiDir_Right && dax > 0.0f) ||
                (g.NavMoveDir == ImGuiDir_Up && day < 0.0f) || (g.NavMoveDir == ImGuiDir_Down && day > 0.0f))
            {
                result->DistAxial = dist_axial;
                new_best = true;
            }

    return new_best;
}

 void ImGui::NavApplyItemToResult(ImGuiNavItemData *result, ImGuiWindow *window, ImGuiID id,
                                        const ImRect &nav_bb_rel)
{
    result->Window = window;
    result->ID = id;
    result->FocusScopeId = window->DC.NavFocusScopeIdCurrent;
    result->RectRel = nav_bb_rel;
}

// We get there when either NavId == id, or when g.NavAnyRequest is set (which is updated by NavUpdateAnyRequestFlag
// above)
 void ImGui::NavProcessItem(ImGuiWindow *window, const ImRect &nav_bb, const ImGuiID id)
{
    ImGuiContext &g = *GImGui;
    // if (!g.IO.NavActive)  // [2017/10/06] Removed this possibly redundant test but I am not sure of all the
    // side-effects yet. Some of the feature here will need to work regardless of using a _NoNavInputs flag.
    //    return;

    const ImGuiItemFlags item_flags = g.CurrentItemFlags;
    const ImRect nav_bb_rel(nav_bb.Min - window->Pos, nav_bb.Max - window->Pos);

    // Process Init Request
    if (g.NavInitRequest && g.NavLayer == window->DC.NavLayerCurrent)
    {
        // Even if 'ImGuiItemFlags_NoNavDefaultFocus' is on (typically collapse/close button) we record the first
        // ResultId so they can be used as a fallback
        if (!(item_flags & ImGuiItemFlags_NoNavDefaultFocus) || g.NavInitResultId == 0)
        {
            g.NavInitResultId = id;
            g.NavInitResultRectRel = nav_bb_rel;
        }
        if (!(item_flags & ImGuiItemFlags_NoNavDefaultFocus))
        {
            g.NavInitRequest = false; // Found a match, clear request
            NavUpdateAnyRequestFlag();
        }
    }

    // Process Move Request (scoring for navigation)
    // FIXME-NAV: Consider policy for double scoring (scoring from NavScoringRectScreen + scoring from a rect wrapped
    // according to current wrapping policy)
    if ((g.NavId != id || (g.NavMoveRequestFlags & ImGuiNavMoveFlags_AllowCurrentNavId)) &&
        !(item_flags & (ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNav)))
    {
        ImGuiNavItemData *result = (window == g.NavWindow) ? &g.NavMoveResultLocal : &g.NavMoveResultOther;
#if IMGUI_DEBUG_NAV_SCORING
        // [DEBUG] Score all items in NavWindow at all times
        if (!g.NavMoveRequest)
            g.NavMoveDir = g.NavMoveDirLast;
        bool new_best = NavScoreItem(result, nav_bb) && g.NavMoveRequest;
#else
        bool new_best = g.NavMoveRequest && NavScoreItem(result, nav_bb);
#endif
        if (new_best)
            NavApplyItemToResult(result, window, id, nav_bb_rel);

        // Features like PageUp/PageDown need to maintain a separate score for the visible set of items.
        const float VISIBLE_RATIO = 0.70f;
        if ((g.NavMoveRequestFlags & ImGuiNavMoveFlags_AlsoScoreVisibleSet) && window->ClipRect.Overlaps(nav_bb))
            if (ImClamp(nav_bb.Max.y, window->ClipRect.Min.y, window->ClipRect.Max.y) -
                    ImClamp(nav_bb.Min.y, window->ClipRect.Min.y, window->ClipRect.Max.y) >=
                (nav_bb.Max.y - nav_bb.Min.y) * VISIBLE_RATIO)
                if (NavScoreItem(&g.NavMoveResultLocalVisibleSet, nav_bb))
                    NavApplyItemToResult(&g.NavMoveResultLocalVisibleSet, window, id, nav_bb_rel);
    }

    // Update window-relative bounding box of navigated item
    if (g.NavId == id)
    {
        g.NavWindow =
            window; // Always refresh g.NavWindow, because some operations such as FocusItem() don't have a window.
        g.NavLayer = window->DC.NavLayerCurrent;
        g.NavFocusScopeId = window->DC.NavFocusScopeIdCurrent;
        g.NavIdIsAlive = true;
        window->NavRectRel[window->DC.NavLayerCurrent] =
            nav_bb_rel; // Store item bounding box (relative to window position)
    }
}

 ImVec2 ImGui::NavCalcPreferredRefPos()
{
    ImGuiContext &g = *GImGui;
    if (g.NavDisableHighlight || !g.NavDisableMouseHover || !g.NavWindow)
    {
        // Mouse (we need a fallback in case the mouse becomes invalid after being used)
        if (IsMousePosValid(&g.IO.MousePos))
            return g.IO.MousePos;
        return g.LastValidMousePos;
    }
    else
    {
        // When navigation is active and mouse is disabled, decide on an arbitrary position around the bottom left of
        // the currently navigated item.
        const ImRect &rect_rel = g.NavWindow->NavRectRel[g.NavLayer];
        ImVec2 pos = g.NavWindow->Pos + ImVec2(rect_rel.Min.x + ImMin(g.Style.FramePadding.x * 4, rect_rel.GetWidth()),
                                               rect_rel.Max.y - ImMin(g.Style.FramePadding.y, rect_rel.GetHeight()));
        ImGuiViewport *viewport = GetMainViewport();
        return ImFloor(ImClamp(
            pos, viewport->Pos,
            viewport->Pos + viewport->Size)); // ImFloor() is important because non-integer mouse position application
                                              // in backend might be lossy and result in undesirable non-zero delta.
    }
}

// FIXME: This could be replaced by updating a frame number in each window when (window == NavWindow) and (NavLayer ==
// 0). This way we could find the last focused window among our children. It would be much less confusing this way?
 void ImGui::NavSaveLastChildNavWindowIntoParent(ImGuiWindow *nav_window)
{
    ImGuiWindow *parent = nav_window;
    while (parent && parent->RootWindow != parent &&
           (parent->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu)) == 0)
        parent = parent->ParentWindow;
    if (parent && parent != nav_window)
        parent->NavLastChildNavWindow = nav_window;
}

// Restore the last focused child.
// Call when we are expected to land on the Main Layer (0) after FocusWindow()
 ImGuiWindow *ImGui::NavRestoreLastChildNavWindow(ImGuiWindow *window)
{
    if (window->NavLastChildNavWindow && window->NavLastChildNavWindow->WasActive)
        return window->NavLastChildNavWindow;
    return window;
}



 int ImGui::FindWindowFocusIndex(ImGuiWindow *window)
{
    ImGuiContext &g = *GImGui;
    IM_UNUSED(g);
    int order = window->FocusOrder;
    IM_ASSERT(g.WindowsFocusOrder[order] == window);
    return order;
}

 void ImGui::ErrorCheckNewFrameSanityChecks()
{
    ImGuiContext &g = *GImGui;

    // Check user IM_ASSERT macro
    // (IF YOU GET A WARNING OR COMPILE ERROR HERE: it means your assert macro is incorrectly defined!
    //  If your macro uses multiple statements, it NEEDS to be surrounded by a 'do { ... } while (0)' block.
    //  This is a common C/C++ idiom to allow multiple statements macros to be used in control flow blocks.)
    // #define IM_ASSERT(EXPR)   if (SomeCode(EXPR)) SomeMoreCode();                    // Wrong!
    // #define IM_ASSERT(EXPR)   do { if (SomeCode(EXPR)) SomeMoreCode(); } while (0)   // Correct!
    if (true)
        IM_ASSERT(1);
    else
        IM_ASSERT(0);

    // Check user data
    // (We pass an error message in the assert expression to make it visible to programmers who are not using a
    // debugger, as most assert handlers display their argument)
    IM_ASSERT(g.Initialized);
    IM_ASSERT((g.IO.DeltaTime > 0.0f || g.FrameCount == 0) && "Need a positive DeltaTime!");
    IM_ASSERT((g.FrameCount == 0 || g.FrameCountEnded == g.FrameCount) &&
              "Forgot to call Render() or EndFrame() at the end of the previous frame?");
    IM_ASSERT(g.IO.DisplaySize.x >= 0.0f && g.IO.DisplaySize.y >= 0.0f && "Invalid DisplaySize value!");
    IM_ASSERT(g.IO.Fonts->Fonts.Size > 0 &&
              "Font Atlas not built. Did you call io.Fonts->GetTexDataAsRGBA32() / GetTexDataAsAlpha8()?");
    IM_ASSERT(g.IO.Fonts->Fonts[0]->IsLoaded() &&
              "Font Atlas not built. Did you call io.Fonts->GetTexDataAsRGBA32() / GetTexDataAsAlpha8()?");
    IM_ASSERT(g.Style.CurveTessellationTol > 0.0f && "Invalid style setting!");
    IM_ASSERT(g.Style.CircleTessellationMaxError > 0.0f && "Invalid style setting!");
    IM_ASSERT(g.Style.Alpha >= 0.0f && g.Style.Alpha <= 1.0f &&
              "Invalid style setting!"); // Allows us to avoid a few clamps in color computations
    IM_ASSERT(g.Style.WindowMinSize.x >= 1.0f && g.Style.WindowMinSize.y >= 1.0f && "Invalid style setting.");
    IM_ASSERT(g.Style.WindowMenuButtonPosition == ImGuiDir_None || g.Style.WindowMenuButtonPosition == ImGuiDir_Left ||
              g.Style.WindowMenuButtonPosition == ImGuiDir_Right);
    for (int n = 0; n < ImGuiKey_COUNT; n++)
        IM_ASSERT(g.IO.KeyMap[n] >= -1 && g.IO.KeyMap[n] < IM_ARRAYSIZE(g.IO.KeysDown) &&
                  "io.KeyMap[] contains an out of bound value (need to be 0..512, or -1 for unmapped key)");

    // Check: required key mapping (we intentionally do NOT check all keys to not pressure user into setting up
    // everything, but Space is required and was only added in 1.60 WIP)
    if (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)
        IM_ASSERT(g.IO.KeyMap[ImGuiKey_Space] != -1 &&
                  "ImGuiKey_Space is not mapped, required for keyboard navigation.");

    // Check: the io.ConfigWindowsResizeFromEdges option requires backend to honor mouse cursor changes and set the
    // ImGuiBackendFlags_HasMouseCursors flag accordingly.
    if (g.IO.ConfigWindowsResizeFromEdges && !(g.IO.BackendFlags & ImGuiBackendFlags_HasMouseCursors))
        g.IO.ConfigWindowsResizeFromEdges = false;
}

 void ImGui::ErrorCheckEndFrameSanityChecks()
{
    ImGuiContext &g = *GImGui;

    // Verify that io.KeyXXX fields haven't been tampered with. Key mods should not be modified between NewFrame() and
    // EndFrame() One possible reason leading to this assert is that your backends update inputs _AFTER_ NewFrame(). It
    // is known that when some modal native windows called mid-frame takes focus away, some backends such as GLFW will
    // send key release events mid-frame. This would normally trigger this assertion and lead to sheared inputs.
    // We silently accommodate for this case by ignoring/ the case where all io.KeyXXX modifiers were released (aka
    // key_mod_flags == 0), while still correctly asserting on mid-frame key press events.
    const ImGuiKeyModFlags key_mod_flags = GetMergedKeyModFlags();
    IM_ASSERT((key_mod_flags == 0 || g.IO.KeyMods == key_mod_flags) &&
              "Mismatching io.KeyCtrl/io.KeyShift/io.KeyAlt/io.KeySuper vs io.KeyMods");
    IM_UNUSED(key_mod_flags);

    // Recover from errors
    // ErrorCheckEndFrameRecover();

    // Report when there is a mismatch of Begin/BeginChild vs End/EndChild calls. Important: Remember that the
    // Begin/BeginChild API requires you to always call End/EndChild even if Begin/BeginChild returns false! (this is
    // unfortunately inconsistent with most other Begin* API).
    if (g.CurrentWindowStack.Size != 1)
    {
        if (g.CurrentWindowStack.Size > 1)
        {
            IM_ASSERT_USER_ERROR(
                g.CurrentWindowStack.Size == 1,
                "Mismatched Begin/BeginChild vs End/EndChild calls: did you forget to call End/EndChild?");
            while (g.CurrentWindowStack.Size > 1)
                End();
        }
        else
        {
            IM_ASSERT_USER_ERROR(
                g.CurrentWindowStack.Size == 1,
                "Mismatched Begin/BeginChild vs End/EndChild calls: did you call End/EndChild too much?");
        }
    }

    IM_ASSERT_USER_ERROR(g.GroupStack.Size == 0, "Missing EndGroup call!");
}

// Called by NewFrame()
void ImGui::UpdateSettings()
{
    // Load settings on first frame (if not explicitly loaded manually before)
    ImGuiContext &g = *GImGui;
    if (!g.SettingsLoaded)
    {
        IM_ASSERT(g.SettingsWindows.empty());
        if (g.IO.IniFilename)
            LoadIniSettingsFromDisk(g.IO.IniFilename);
        g.SettingsLoaded = true;
    }

    // Save settings (with a delay after the last modification, so we don't spam disk too much)
    if (g.SettingsDirtyTimer > 0.0f)
    {
        g.SettingsDirtyTimer -= g.IO.DeltaTime;
        if (g.SettingsDirtyTimer <= 0.0f)
        {
            if (g.IO.IniFilename != NULL)
                SaveIniSettingsToDisk(g.IO.IniFilename);
            else
                g.IO.WantSaveIniSettings = true; // Let user know they can call SaveIniSettingsToMemory(). user will
                                                 // need to clear io.WantSaveIniSettings themselves.
            g.SettingsDirtyTimer = 0.0f;
        }
    }
}

 void ImGui::UpdateMouseInputs()
{
    ImGuiContext &g = *GImGui;

    // Round mouse position to avoid spreading non-rounded position (e.g. UpdateManualResize doesn't support them well)
    if (IsMousePosValid(&g.IO.MousePos))
        g.IO.MousePos = g.LastValidMousePos = ImFloor(g.IO.MousePos);

    // If mouse just appeared or disappeared (usually denoted by -FLT_MAX components) we cancel out movement in
    // MouseDelta
    if (IsMousePosValid(&g.IO.MousePos) && IsMousePosValid(&g.IO.MousePosPrev))
        g.IO.MouseDelta = g.IO.MousePos - g.IO.MousePosPrev;
    else
        g.IO.MouseDelta = ImVec2(0.0f, 0.0f);
    if (g.IO.MouseDelta.x != 0.0f || g.IO.MouseDelta.y != 0.0f)
        g.NavDisableMouseHover = false;

    g.IO.MousePosPrev = g.IO.MousePos;
    for (int i = 0; i < IM_ARRAYSIZE(g.IO.MouseDown); i++)
    {
        g.IO.MouseClicked[i] = g.IO.MouseDown[i] && g.IO.MouseDownDuration[i] < 0.0f;
        g.IO.MouseReleased[i] = !g.IO.MouseDown[i] && g.IO.MouseDownDuration[i] >= 0.0f;
        g.IO.MouseDownDurationPrev[i] = g.IO.MouseDownDuration[i];
        g.IO.MouseDownDuration[i] =
            g.IO.MouseDown[i] ? (g.IO.MouseDownDuration[i] < 0.0f ? 0.0f : g.IO.MouseDownDuration[i] + g.IO.DeltaTime)
                              : -1.0f;
        g.IO.MouseDoubleClicked[i] = false;
        if (g.IO.MouseClicked[i])
        {
            if ((float)(g.Time - g.IO.MouseClickedTime[i]) < g.IO.MouseDoubleClickTime)
            {
                ImVec2 delta_from_click_pos =
                    IsMousePosValid(&g.IO.MousePos) ? (g.IO.MousePos - g.IO.MouseClickedPos[i]) : ImVec2(0.0f, 0.0f);
                if (ImLengthSqr(delta_from_click_pos) < g.IO.MouseDoubleClickMaxDist * g.IO.MouseDoubleClickMaxDist)
                    g.IO.MouseDoubleClicked[i] = true;
                g.IO.MouseClickedTime[i] =
                    -g.IO.MouseDoubleClickTime *
                    2.0f; // Mark as "old enough" so the third click isn't turned into a double-click
            }
            else
            {
                g.IO.MouseClickedTime[i] = g.Time;
            }
            g.IO.MouseClickedPos[i] = g.IO.MousePos;
            g.IO.MouseDownWasDoubleClick[i] = g.IO.MouseDoubleClicked[i];
            g.IO.MouseDragMaxDistanceAbs[i] = ImVec2(0.0f, 0.0f);
            g.IO.MouseDragMaxDistanceSqr[i] = 0.0f;
        }
        else if (g.IO.MouseDown[i])
        {
            // Maintain the maximum distance we reaching from the initial click position, which is used with dragging
            // threshold
            ImVec2 delta_from_click_pos =
                IsMousePosValid(&g.IO.MousePos) ? (g.IO.MousePos - g.IO.MouseClickedPos[i]) : ImVec2(0.0f, 0.0f);
            g.IO.MouseDragMaxDistanceSqr[i] = ImMax(g.IO.MouseDragMaxDistanceSqr[i], ImLengthSqr(delta_from_click_pos));
            g.IO.MouseDragMaxDistanceAbs[i].x =
                ImMax(g.IO.MouseDragMaxDistanceAbs[i].x,
                      delta_from_click_pos.x < 0.0f ? -delta_from_click_pos.x : delta_from_click_pos.x);
            g.IO.MouseDragMaxDistanceAbs[i].y =
                ImMax(g.IO.MouseDragMaxDistanceAbs[i].y,
                      delta_from_click_pos.y < 0.0f ? -delta_from_click_pos.y : delta_from_click_pos.y);
        }
        if (!g.IO.MouseDown[i] && !g.IO.MouseReleased[i])
            g.IO.MouseDownWasDoubleClick[i] = false;
        if (g.IO.MouseClicked[i]) // Clicking any mouse button reactivate mouse hovering which may have been deactivated
                                  // by gamepad/keyboard navigation
            g.NavDisableMouseHover = false;
    }
}

 void StartLockWheelingWindow(ImGuiWindow *window)
{
    ImGuiContext &g = *GImGui;
    if (g.WheelingWindow == window)
        return;
    g.WheelingWindow = window;
    g.WheelingWindowRefMousePos = g.IO.MousePos;
    g.WheelingWindowTimer = WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER;
}

void ImGui::UpdateMouseWheel()
{
    ImGuiContext &g = *GImGui;

    // Reset the locked window if we move the mouse or after the timer elapses
    if (g.WheelingWindow != NULL)
    {
        g.WheelingWindowTimer -= g.IO.DeltaTime;
        if (IsMousePosValid() && ImLengthSqr(g.IO.MousePos - g.WheelingWindowRefMousePos) >
                                     g.IO.MouseDragThreshold * g.IO.MouseDragThreshold)
            g.WheelingWindowTimer = 0.0f;
        if (g.WheelingWindowTimer <= 0.0f)
        {
            g.WheelingWindow = NULL;
            g.WheelingWindowTimer = 0.0f;
        }
    }

    if (g.IO.MouseWheel == 0.0f && g.IO.MouseWheelH == 0.0f)
        return;

    if ((g.ActiveId != 0 && g.ActiveIdUsingMouseWheel) ||
        (g.HoveredIdPreviousFrame != 0 && g.HoveredIdPreviousFrameUsingMouseWheel))
        return;

    ImGuiWindow *window = g.WheelingWindow ? g.WheelingWindow : g.HoveredWindow;
    if (!window || window->Collapsed)
        return;

    // Zoom / Scale window
    // FIXME-OBSOLETE: This is an old feature, it still works but pretty much nobody is using it and may be best
    // redesigned.
    if (g.IO.MouseWheel != 0.0f && g.IO.KeyCtrl && g.IO.FontAllowUserScaling)
    {
        StartLockWheelingWindow(window);
        const float new_font_scale = ImClamp(window->FontWindowScale + g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
        const float scale = new_font_scale / window->FontWindowScale;
        window->FontWindowScale = new_font_scale;
        if (window == window->RootWindow)
        {
            const ImVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
            SetWindowPos(window, window->Pos + offset, 0);
            window->Size = ImFloor(window->Size * scale);
            window->SizeFull = ImFloor(window->SizeFull * scale);
        }
        return;
    }

    // Mouse wheel scrolling
    // If a child window has the ImGuiWindowFlags_NoScrollWithMouse flag, we give a chance to scroll its parent
    if (g.IO.KeyCtrl)
        return;

    // As a standard behavior holding SHIFT while using Vertical Mouse Wheel triggers Horizontal scroll instead
    // (we avoid doing it on OSX as it the OS input layer handles this already)
    const bool swap_axis = g.IO.KeyShift && !g.IO.ConfigMacOSXBehaviors;
    const float wheel_y = swap_axis ? 0.0f : g.IO.MouseWheel;
    const float wheel_x = swap_axis ? g.IO.MouseWheel : g.IO.MouseWheelH;

    // Vertical Mouse Wheel scrolling
    if (wheel_y != 0.0f)
    {
        StartLockWheelingWindow(window);
        while ((window->Flags & ImGuiWindowFlags_ChildWindow) &&
               ((window->ScrollMax.y == 0.0f) || ((window->Flags & ImGuiWindowFlags_NoScrollWithMouse) &&
                                                  !(window->Flags & ImGuiWindowFlags_NoMouseInputs))))
            window = window->ParentWindow;
        if (!(window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & ImGuiWindowFlags_NoMouseInputs))
        {
            float max_step = window->InnerRect.GetHeight() * 0.67f;
            float scroll_step = ImFloor(ImMin(5 * window->CalcFontSize(), max_step));
            SetScrollY(window, window->Scroll.y - wheel_y * scroll_step);
        }
    }

    // Horizontal Mouse Wheel scrolling, or Vertical Mouse Wheel w/ Shift held
    if (wheel_x != 0.0f)
    {
        StartLockWheelingWindow(window);
        while ((window->Flags & ImGuiWindowFlags_ChildWindow) &&
               ((window->ScrollMax.x == 0.0f) || ((window->Flags & ImGuiWindowFlags_NoScrollWithMouse) &&
                                                  !(window->Flags & ImGuiWindowFlags_NoMouseInputs))))
            window = window->ParentWindow;
        if (!(window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & ImGuiWindowFlags_NoMouseInputs))
        {
            float max_step = window->InnerRect.GetWidth() * 0.67f;
            float scroll_step = ImFloor(ImMin(2 * window->CalcFontSize(), max_step));
            SetScrollX(window, window->Scroll.x - wheel_x * scroll_step);
        }
    }
}

void ImGui::UpdateTabFocus()
{
    ImGuiContext &g = *GImGui;

    // Pressing TAB activate widget focus
    g.TabFocusPressed = (g.NavWindow && g.NavWindow->Active && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs) &&
                         !g.IO.KeyCtrl && IsKeyPressedMap(ImGuiKey_Tab));
    if (g.ActiveId == 0 && g.TabFocusPressed)
    {
        // - This path is only taken when no widget are active/tabbed-into yet.
        //   Subsequent tabbing will be processed by FocusableItemRegister()
        // - Note that SetKeyboardFocusHere() sets the Next fields mid-frame. To be consistent we also
        //   manipulate the Next fields here even though they will be turned into Curr fields below.
        g.TabFocusRequestNextWindow = g.NavWindow;
        g.TabFocusRequestNextCounterRegular = INT_MAX;
        if (g.NavId != 0 && g.NavIdTabCounter != INT_MAX)
            g.TabFocusRequestNextCounterTabStop = g.NavIdTabCounter + (g.IO.KeyShift ? -1 : 0);
        else
            g.TabFocusRequestNextCounterTabStop = g.IO.KeyShift ? -1 : 0;
    }

    // Turn queued focus request into current one
    g.TabFocusRequestCurrWindow = NULL;
    g.TabFocusRequestCurrCounterRegular = g.TabFocusRequestCurrCounterTabStop = INT_MAX;
    if (g.TabFocusRequestNextWindow != NULL)
    {
        ImGuiWindow *window = g.TabFocusRequestNextWindow;
        g.TabFocusRequestCurrWindow = window;
        if (g.TabFocusRequestNextCounterRegular != INT_MAX && window->DC.FocusCounterRegular != -1)
            g.TabFocusRequestCurrCounterRegular =
                ImModPositive(g.TabFocusRequestNextCounterRegular, window->DC.FocusCounterRegular + 1);
        if (g.TabFocusRequestNextCounterTabStop != INT_MAX && window->DC.FocusCounterTabStop != -1)
            g.TabFocusRequestCurrCounterTabStop =
                ImModPositive(g.TabFocusRequestNextCounterTabStop, window->DC.FocusCounterTabStop + 1);
        g.TabFocusRequestNextWindow = NULL;
        g.TabFocusRequestNextCounterRegular = g.TabFocusRequestNextCounterTabStop = INT_MAX;
    }

    g.NavIdTabCounter = INT_MAX;
}

// [DEBUG] Item picker tool - start with DebugStartItemPicker() - useful to visually select an item and break into its
// call-stack.
void ImGui::UpdateDebugToolItemPicker()
{
    ImGuiContext &g = *GImGui;
    g.DebugItemPickerBreakId = 0;
    if (g.DebugItemPickerActive)
    {
        const ImGuiID hovered_id = g.HoveredIdPreviousFrame;
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsKeyPressedMap(ImGuiKey_Escape))
            g.DebugItemPickerActive = false;
        if (ImGui::IsMouseClicked(0) && hovered_id)
        {
            g.DebugItemPickerBreakId = hovered_id;
            g.DebugItemPickerActive = false;
        }
        ImGui::SetNextWindowBgAlpha(0.60f);
        ImGui::BeginTooltip();
        ImGui::Text("HoveredId: 0x%08X", hovered_id);
        ImGui::Text("Press ESC to abort picking.");
        ImGui::TextColored(GetStyleColorVec4(hovered_id ? ImGuiCol_Text : ImGuiCol_TextDisabled),
                           "Click to break in debugger!");
        ImGui::EndTooltip();
    }
}

 ImRect GetResizeBorderRect(ImGuiWindow *window, int border_n, float perp_padding, float thickness)
{
    ImRect rect = window->Rect();
    if (thickness == 0.0f)
        rect.Max -= ImVec2(1, 1);
    if (border_n == ImGuiDir_Left)
    {
        return ImRect(rect.Min.x - thickness, rect.Min.y + perp_padding, rect.Min.x + thickness,
                      rect.Max.y - perp_padding);
    }
    if (border_n == ImGuiDir_Right)
    {
        return ImRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x + thickness,
                      rect.Max.y - perp_padding);
    }
    if (border_n == ImGuiDir_Up)
    {
        return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness, rect.Max.x - perp_padding,
                      rect.Min.y + thickness);
    }
    if (border_n == ImGuiDir_Down)
    {
        return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding,
                      rect.Max.y + thickness);
    }
    IM_ASSERT(0);
    return ImRect();
}

// Handle resize for: Resize Grips, Borders, Gamepad
// Return true when using auto-fit (double click on resize grip)
 bool ImGui::UpdateWindowManualResize(ImGuiWindow *window, const ImVec2 &size_auto_fit, int *border_held,
                                            int resize_grip_count, ImU32 resize_grip_col[4],
                                            const ImRect &visibility_rect)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindowFlags flags = window->Flags;

    if ((flags & ImGuiWindowFlags_NoResize) || (flags & ImGuiWindowFlags_AlwaysAutoResize) ||
        window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
        return false;
    if (window->WasActive ==
        false) // Early out to avoid running this code for e.g. an hidden implicit/fallback Debug window.
        return false;

    bool ret_auto_fit = false;
    const int resize_border_count = g.IO.ConfigWindowsResizeFromEdges ? 4 : 0;
    const float grip_draw_size = IM_FLOOR(ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
    const float grip_hover_inner_size = IM_FLOOR(grip_draw_size * 0.75f);
    const float grip_hover_outer_size = g.IO.ConfigWindowsResizeFromEdges ? WINDOWS_HOVER_PADDING : 0.0f;

    ImVec2 pos_target(FLT_MAX, FLT_MAX);
    ImVec2 size_target(FLT_MAX, FLT_MAX);

    // Resize grips and borders are on layer 1
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    // Manual resize grips
    PushID("#RESIZE");
    for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
    {
        const ImGuiResizeGripDef &def = resize_grip_def[resize_grip_n];
        const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, def.CornerPosN);

        // Using the FlattenChilds button flag we make the resize button accessible even if we are hovering over a child
        // window
        bool hovered, held;
        ImRect resize_rect(corner - def.InnerDir * grip_hover_outer_size,
                           corner + def.InnerDir * grip_hover_inner_size);
        if (resize_rect.Min.x > resize_rect.Max.x)
            ImSwap(resize_rect.Min.x, resize_rect.Max.x);
        if (resize_rect.Min.y > resize_rect.Max.y)
            ImSwap(resize_rect.Min.y, resize_rect.Max.y);
        ImGuiID resize_grip_id = window->GetID(resize_grip_n); // == GetWindowResizeCornerID()
        ButtonBehavior(resize_rect, resize_grip_id, &hovered, &held,
                       ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);
        // GetForegroundDrawList(window)->AddRect(resize_rect.Min, resize_rect.Max, IM_COL32(255, 255, 0, 255));
        if (hovered || held)
            g.MouseCursor = (resize_grip_n & 1) ? ImGuiMouseCursor_ResizeNESW : ImGuiMouseCursor_ResizeNWSE;

        if (held && g.IO.MouseDoubleClicked[0] && resize_grip_n == 0)
        {
            // Manual auto-fit when double-clicking
            size_target = CalcWindowSizeAfterConstraint(window, size_auto_fit);
            ret_auto_fit = true;
            ClearActiveID();
        }
        else if (held)
        {
            // Resize from any of the four corners
            // We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
            ImVec2 clamp_min = ImVec2(def.CornerPosN.x == 1.0f ? visibility_rect.Min.x : -FLT_MAX,
                                      def.CornerPosN.y == 1.0f ? visibility_rect.Min.y : -FLT_MAX);
            ImVec2 clamp_max = ImVec2(def.CornerPosN.x == 0.0f ? visibility_rect.Max.x : +FLT_MAX,
                                      def.CornerPosN.y == 0.0f ? visibility_rect.Max.y : +FLT_MAX);
            ImVec2 corner_target = g.IO.MousePos - g.ActiveIdClickOffset +
                                   ImLerp(def.InnerDir * grip_hover_outer_size, def.InnerDir * -grip_hover_inner_size,
                                          def.CornerPosN); // Corner of the window corresponding to our corner grip
            corner_target = ImClamp(corner_target, clamp_min, clamp_max);
            CalcResizePosSizeFromAnyCorner(window, corner_target, def.CornerPosN, &pos_target, &size_target);
        }

        // Only lower-left grip is visible before hovering/activating
        if (resize_grip_n == 0 || held || hovered)
            resize_grip_col[resize_grip_n] = GetColorU32(held      ? ImGuiCol_ResizeGripActive
                                                         : hovered ? ImGuiCol_ResizeGripHovered
                                                                   : ImGuiCol_ResizeGrip);
    }
    for (int border_n = 0; border_n < resize_border_count; border_n++)
    {
        const ImGuiResizeBorderDef &def = resize_border_def[border_n];
        const ImGuiAxis axis = (border_n == ImGuiDir_Left || border_n == ImGuiDir_Right) ? ImGuiAxis_X : ImGuiAxis_Y;

        bool hovered, held;
        ImRect border_rect = GetResizeBorderRect(window, border_n, grip_hover_inner_size, WINDOWS_HOVER_PADDING);
        ImGuiID border_id = window->GetID(border_n + 4); // == GetWindowResizeBorderID()
        ButtonBehavior(border_rect, border_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren);
        // GetForegroundDrawLists(window)->AddRect(border_rect.Min, border_rect.Max, IM_COL32(255, 255, 0, 255));
        if ((hovered && g.HoveredIdTimer > WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER) || held)
        {
            g.MouseCursor = (axis == ImGuiAxis_X) ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS;
            if (held)
                *border_held = border_n;
        }
        if (held)
        {
            ImVec2 clamp_min(border_n == ImGuiDir_Right ? visibility_rect.Min.x : -FLT_MAX,
                             border_n == ImGuiDir_Down ? visibility_rect.Min.y : -FLT_MAX);
            ImVec2 clamp_max(border_n == ImGuiDir_Left ? visibility_rect.Max.x : +FLT_MAX,
                             border_n == ImGuiDir_Up ? visibility_rect.Max.y : +FLT_MAX);
            ImVec2 border_target = window->Pos;
            border_target[axis] = g.IO.MousePos[axis] - g.ActiveIdClickOffset[axis] + WINDOWS_HOVER_PADDING;
            border_target = ImClamp(border_target, clamp_min, clamp_max);
            CalcResizePosSizeFromAnyCorner(window, border_target, ImMin(def.SegmentN1, def.SegmentN2), &pos_target,
                                           &size_target);
        }
    }
    PopID();

    // Restore nav layer
    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;

    // Navigation resize (keyboard/gamepad)
    if (g.NavWindowingTarget && g.NavWindowingTarget->RootWindow == window)
    {
        ImVec2 nav_resize_delta;
        if (g.NavInputSource == ImGuiInputSource_Keyboard && g.IO.KeyShift)
            nav_resize_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard, ImGuiInputReadMode_Down);
        if (g.NavInputSource == ImGuiInputSource_Gamepad)
            nav_resize_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_PadDPad, ImGuiInputReadMode_Down);
        if (nav_resize_delta.x != 0.0f || nav_resize_delta.y != 0.0f)
        {
            const float NAV_RESIZE_SPEED = 600.0f;
            nav_resize_delta *= ImFloor(NAV_RESIZE_SPEED * g.IO.DeltaTime *
                                        ImMin(g.IO.DisplayFramebufferScale.x, g.IO.DisplayFramebufferScale.y));
            nav_resize_delta = ImMax(nav_resize_delta, visibility_rect.Min - window->Pos - window->Size);
            g.NavWindowingToggleLayer = false;
            g.NavDisableMouseHover = true;
            resize_grip_col[0] = GetColorU32(ImGuiCol_ResizeGripActive);
            // FIXME-NAV: Should store and accumulate into a separate size buffer to handle sizing constraints properly,
            // right now a constraint will make us stuck.
            size_target = CalcWindowSizeAfterConstraint(window, window->SizeFull + nav_resize_delta);
        }
    }

    // Apply back modified position/size to window
    if (size_target.x != FLT_MAX)
    {
        window->SizeFull = size_target;
        MarkIniSettingsDirty(window);
    }
    if (pos_target.x != FLT_MAX)
    {
        window->Pos = ImFloor(pos_target);
        MarkIniSettingsDirty(window);
    }

    window->Size = window->SizeFull;
    return ret_auto_fit;
}

 void ImGui::RenderWindowOuterBorders(ImGuiWindow *window)
{
    ImGuiContext &g = *GImGui;
    float rounding = window->WindowRounding;
    float border_size = window->WindowBorderSize;
    if (border_size > 0.0f && !(window->Flags & ImGuiWindowFlags_NoBackground))
        window->DrawList->AddRect(window->Pos, window->Pos + window->Size, GetColorU32(ImGuiCol_Border), rounding, 0,
                                  border_size);

    int border_held = window->ResizeBorderHeld;
    if (border_held != -1)
    {
        const ImGuiResizeBorderDef &def = resize_border_def[border_held];
        ImRect border_r = GetResizeBorderRect(window, border_held, rounding, 0.0f);
        window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN1) + ImVec2(0.5f, 0.5f) +
                                        def.InnerDir * rounding,
                                    rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
        window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN2) + ImVec2(0.5f, 0.5f) +
                                        def.InnerDir * rounding,
                                    rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
        window->DrawList->PathStroke(GetColorU32(ImGuiCol_SeparatorActive), 0,
                                     ImMax(2.0f, border_size)); // Thicker than usual
    }
    if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar))
    {
        float y = window->Pos.y + window->TitleBarHeight() - 1;
        window->DrawList->AddLine(ImVec2(window->Pos.x + border_size, y),
                                  ImVec2(window->Pos.x + window->Size.x - border_size, y), GetColorU32(ImGuiCol_Border),
                                  g.Style.FrameBorderSize);
    }
}

 ImGuiCol GetWindowBgColorIdxFromFlags(ImGuiWindowFlags flags)
{
    if (flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup))
        return ImGuiCol_PopupBg;
    if (flags & ImGuiWindowFlags_ChildWindow)
        return ImGuiCol_ChildBg;
    return ImGuiCol_WindowBg;
}

// Draw background and borders
// Draw and handle scrollbars
void ImGui::RenderWindowDecorations(ImGuiWindow *window, const ImRect &title_bar_rect, bool title_bar_is_highlight,
                                    int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size)
{
    ImGuiContext &g = *GImGui;
    ImGuiStyle &style = g.Style;
    ImGuiWindowFlags flags = window->Flags;

    // Ensure that ScrollBar doesn't read last frame's SkipItems
    IM_ASSERT(window->BeginCount == 0);
    window->SkipItems = false;

    // Draw window + handle manual resize
    // As we highlight the title bar when want_focus is set, multiple reappearing windows will have have their title bar
    // highlighted on their reappearing frame.
    const float window_rounding = window->WindowRounding;
    const float window_border_size = window->WindowBorderSize;
    if (window->Collapsed)
    {
        // Title bar only
        float backup_border_size = style.FrameBorderSize;
        g.Style.FrameBorderSize = window->WindowBorderSize;
        ImU32 title_bar_col = GetColorU32(
            (title_bar_is_highlight && !g.NavDisableHighlight) ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBgCollapsed);
        RenderFrame(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, true, window_rounding);
        g.Style.FrameBorderSize = backup_border_size;
    }
    else
    {
        // Window background
        if (!(flags & ImGuiWindowFlags_NoBackground))
        {
            ImU32 bg_col = GetColorU32(GetWindowBgColorIdxFromFlags(flags));
            bool override_alpha = false;
            float alpha = 1.0f;
            if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasBgAlpha)
            {
                alpha = g.NextWindowData.BgAlphaVal;
                override_alpha = true;
            }
            if (override_alpha)
                bg_col = (bg_col & ~IM_COL32_A_MASK) | (IM_F32_TO_INT8_SAT(alpha) << IM_COL32_A_SHIFT);
            window->DrawList->AddRectFilled(window->Pos + ImVec2(0, window->TitleBarHeight()),
                                            window->Pos + window->Size, bg_col, window_rounding,
                                            (flags & ImGuiWindowFlags_NoTitleBar) ? 0 : ImDrawFlags_RoundCornersBottom);
        }

        // Title bar
        if (!(flags & ImGuiWindowFlags_NoTitleBar))
        {
            ImU32 title_bar_col = GetColorU32(title_bar_is_highlight ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg);
            window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, window_rounding,
                                            ImDrawFlags_RoundCornersTop);
        }

        // Menu bar
        if (flags & ImGuiWindowFlags_MenuBar)
        {
            ImRect menu_bar_rect = window->MenuBarRect();
            menu_bar_rect.ClipWith(window->Rect()); // Soft clipping, in particular child window don't have minimum size
                                                    // covering the menu bar so this is useful for them.
            window->DrawList->AddRectFilled(
                menu_bar_rect.Min + ImVec2(window_border_size, 0), menu_bar_rect.Max - ImVec2(window_border_size, 0),
                GetColorU32(ImGuiCol_MenuBarBg), (flags & ImGuiWindowFlags_NoTitleBar) ? window_rounding : 0.0f,
                ImDrawFlags_RoundCornersTop);
            if (style.FrameBorderSize > 0.0f && menu_bar_rect.Max.y < window->Pos.y + window->Size.y)
                window->DrawList->AddLine(menu_bar_rect.GetBL(), menu_bar_rect.GetBR(), GetColorU32(ImGuiCol_Border),
                                          style.FrameBorderSize);
        }

        // Scrollbars
        if (window->ScrollbarX)
            Scrollbar(ImGuiAxis_X);
        if (window->ScrollbarY)
            Scrollbar(ImGuiAxis_Y);

        // Render resize grips (after their input handling so we don't have a frame of latency)
        if (!(flags & ImGuiWindowFlags_NoResize))
        {
            for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
            {
                const ImGuiResizeGripDef &grip = resize_grip_def[resize_grip_n];
                const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, grip.CornerPosN);
                window->DrawList->PathLineTo(
                    corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(window_border_size, resize_grip_draw_size)
                                                                  : ImVec2(resize_grip_draw_size, window_border_size)));
                window->DrawList->PathLineTo(
                    corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(resize_grip_draw_size, window_border_size)
                                                                  : ImVec2(window_border_size, resize_grip_draw_size)));
                window->DrawList->PathArcToFast(
                    ImVec2(corner.x + grip.InnerDir.x * (window_rounding + window_border_size),
                           corner.y + grip.InnerDir.y * (window_rounding + window_border_size)),
                    window_rounding, grip.AngleMin12, grip.AngleMax12);
                window->DrawList->PathFillConvex(resize_grip_col[resize_grip_n]);
            }
        }

        // Borders
        RenderWindowOuterBorders(window);
    }
}


// Update viewports and monitor infos
 void ImGui::UpdateViewportsNewFrame()
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(g.Viewports.Size == 1);

    // Update main viewport with current platform position.
    // FIXME-VIEWPORT: Size is driven by backend/user code for backward-compatibility but we should aim to make this
    // more consistent.
    ImGuiViewportP *main_viewport = g.Viewports[0];
    main_viewport->Flags = ImGuiViewportFlags_IsPlatformWindow | ImGuiViewportFlags_OwnedByApp;
    main_viewport->Pos = ImVec2(0.0f, 0.0f);
    main_viewport->Size = g.IO.DisplaySize;

    for (int n = 0; n < g.Viewports.Size; n++)
    {
        ImGuiViewportP *viewport = g.Viewports[n];

        // Lock down space taken by menu bars and status bars, reset the offset for fucntions like BeginMainMenuBar() to
        // alter them again.
        viewport->WorkOffsetMin = viewport->BuildWorkOffsetMin;
        viewport->WorkOffsetMax = viewport->BuildWorkOffsetMax;
        viewport->BuildWorkOffsetMin = viewport->BuildWorkOffsetMax = ImVec2(0.0f, 0.0f);
        viewport->UpdateWorkRect();
    }
}
