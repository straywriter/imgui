#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <ImGui/ImGuiInternal.h>

struct ImGuiContext;
extern ImGuiContext *GImGui;

ImGuiContext *ImGui::CreateContext(ImFontAtlas *shared_font_atlas)
{
    ImGuiContext *ctx = IM_NEW(ImGuiContext)(shared_font_atlas);
    if (GImGui == NULL)
        SetCurrentContext(ctx);
    Initialize(ctx);
    return ctx;
}

void ImGui::DestroyContext(ImGuiContext *ctx)
{
    if (ctx == NULL)
        ctx = GImGui;
    Shutdown(ctx);
    if (GImGui == ctx)
        SetCurrentContext(NULL);
    IM_DELETE(ctx);
}

// Internal state access - if you want to share Dear ImGui state between modules (e.g. DLL) or allocate it yourself
// Note that we still point to some static data and members (such as GFontAtlas), so the state instance you end up using
// will point to the static data within its module
ImGuiContext *ImGui::GetCurrentContext()
{
    return GImGui;
}

void ImGui::SetCurrentContext(ImGuiContext *ctx)
{
#ifdef IMGUI_SET_CURRENT_CONTEXT_FUNC
    IMGUI_SET_CURRENT_CONTEXT_FUNC(ctx); // For custom thread-based hackery you may want to have control over this.
#else
    GImGui = ctx;
#endif
}

ImGuiIO &ImGui::GetIO()
{
    IM_ASSERT(GImGui != NULL &&
              "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->IO;
}

ImGuiStyle &ImGui::GetStyle()
{
    IM_ASSERT(GImGui != NULL &&
              "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->Style;
}

//
// namespace ImGui
// {
// // Navigation
// static void NavUpdate();
// static void NavUpdateWindowing();
// static void NavUpdateWindowingOverlay();
// static void NavUpdateMoveResult();
// static void NavUpdateInitResult();
// static float NavUpdatePageUpPageDown();
// static inline void NavUpdateAnyRequestFlag();
// static void NavEndFrame();
// static bool NavScoreItem(ImGuiNavItemData *result, ImRect cand);
// static void NavApplyItemToResult(ImGuiNavItemData *result, ImGuiWindow *window, ImGuiID id, const ImRect &nav_bb_rel);
// static void NavProcessItem(ImGuiWindow *window, const ImRect &nav_bb, ImGuiID id);
// static ImVec2 NavCalcPreferredRefPos();
// static void NavSaveLastChildNavWindowIntoParent(ImGuiWindow *nav_window);
// static ImGuiWindow *NavRestoreLastChildNavWindow(ImGuiWindow *window);
// static void NavRestoreLayer(ImGuiNavLayer layer);
// static int FindWindowFocusIndex(ImGuiWindow *window);

// // Error Checking
// static void ErrorCheckNewFrameSanityChecks();
// static void ErrorCheckEndFrameSanityChecks();

// // Misc
// static void UpdateSettings();
// static void UpdateMouseInputs();
// static void UpdateMouseWheel();
// static void UpdateTabFocus();
// static void UpdateDebugToolItemPicker();
// static bool UpdateWindowManualResize(ImGuiWindow *window, const ImVec2 &size_auto_fit, int *border_held,
//                                      int resize_grip_count, ImU32 resize_grip_col[4], const ImRect &visibility_rect);
// static void RenderWindowOuterBorders(ImGuiWindow *window);
// static void RenderWindowDecorations(ImGuiWindow *window, const ImRect &title_bar_rect, bool title_bar_is_highlight,
//                                     int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size);
// static void RenderWindowTitleBarContents(ImGuiWindow *window, const ImRect &title_bar_rect, const char *name,
//                                          bool *p_open);

// // Viewports
// static void UpdateViewportsNewFrame();

// } // namespace ImGui

void ImGui::NewFrame()
{
    IM_ASSERT(GImGui != NULL &&
              "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    ImGuiContext &g = *GImGui;

    // Remove pending delete hooks before frame start.
    // This deferred removal avoid issues of removal while iterating the hook vector
    for (int n = g.Hooks.Size - 1; n >= 0; n--)
        if (g.Hooks[n].Type == ImGuiContextHookType_PendingRemoval_)
            g.Hooks.erase(&g.Hooks[n]);

    CallContextHooks(&g, ImGuiContextHookType_NewFramePre);

    // Check and assert for various common IO and Configuration mistakes
    ErrorCheckNewFrameSanityChecks();

    // Load settings on first frame, save settings when modified (after a delay)
    UpdateSettings();

    g.Time += g.IO.DeltaTime;
    g.WithinFrameScope = true;
    g.FrameCount += 1;
    g.TooltipOverrideCount = 0;
    g.WindowsActiveCount = 0;
    g.MenusIdSubmittedThisFrame.resize(0);

    // Calculate frame-rate for the user, as a purely luxurious feature
    g.FramerateSecPerFrameAccum += g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
    g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
    g.FramerateSecPerFrameIdx = (g.FramerateSecPerFrameIdx + 1) % IM_ARRAYSIZE(g.FramerateSecPerFrame);
    g.FramerateSecPerFrameCount = ImMin(g.FramerateSecPerFrameCount + 1, IM_ARRAYSIZE(g.FramerateSecPerFrame));
    g.IO.Framerate = (g.FramerateSecPerFrameAccum > 0.0f)
                         ? (1.0f / (g.FramerateSecPerFrameAccum / (float)g.FramerateSecPerFrameCount))
                         : FLT_MAX;

    UpdateViewportsNewFrame();

    // Setup current font and draw list shared data
    g.IO.Fonts->Locked = true;
    SetCurrentFont(GetDefaultFont());
    IM_ASSERT(g.Font->IsLoaded());
    ImRect virtual_space(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int n = 0; n < g.Viewports.Size; n++)
        virtual_space.Add(g.Viewports[n]->GetMainRect());
    g.DrawListSharedData.ClipRectFullscreen = virtual_space.ToVec4();
    g.DrawListSharedData.CurveTessellationTol = g.Style.CurveTessellationTol;
    g.DrawListSharedData.SetCircleTessellationMaxError(g.Style.CircleTessellationMaxError);
    g.DrawListSharedData.InitialFlags = ImDrawListFlags_None;
    if (g.Style.AntiAliasedLines)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLines;
    if (g.Style.AntiAliasedLinesUseTex && !(g.Font->ContainerAtlas->Flags & ImFontAtlasFlags_NoBakedLines))
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLinesUseTex;
    if (g.Style.AntiAliasedFill)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedFill;
    if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AllowVtxOffset;

    // Mark rendering data as invalid to prevent user who may have a handle on it to use it.
    for (int n = 0; n < g.Viewports.Size; n++)
    {
        ImGuiViewportP *viewport = g.Viewports[n];
        viewport->DrawDataP.Clear();
    }

    // Drag and drop keep the source ID alive so even if the source disappear our state is consistent
    if (g.DragDropActive && g.DragDropPayload.SourceId == g.ActiveId)
        KeepAliveID(g.DragDropPayload.SourceId);

    // Update HoveredId data
    if (!g.HoveredIdPreviousFrame)
        g.HoveredIdTimer = 0.0f;
    if (!g.HoveredIdPreviousFrame || (g.HoveredId && g.ActiveId == g.HoveredId))
        g.HoveredIdNotActiveTimer = 0.0f;
    if (g.HoveredId)
        g.HoveredIdTimer += g.IO.DeltaTime;
    if (g.HoveredId && g.ActiveId != g.HoveredId)
        g.HoveredIdNotActiveTimer += g.IO.DeltaTime;
    g.HoveredIdPreviousFrame = g.HoveredId;
    g.HoveredIdPreviousFrameUsingMouseWheel = g.HoveredIdUsingMouseWheel;
    g.HoveredId = 0;
    g.HoveredIdAllowOverlap = false;
    g.HoveredIdUsingMouseWheel = false;
    g.HoveredIdDisabled = false;

    // Update ActiveId data (clear reference to active widget if the widget isn't alive anymore)
    if (g.ActiveIdIsAlive != g.ActiveId && g.ActiveIdPreviousFrame == g.ActiveId && g.ActiveId != 0)
        ClearActiveID();
    if (g.ActiveId)
        g.ActiveIdTimer += g.IO.DeltaTime;
    g.LastActiveIdTimer += g.IO.DeltaTime;
    g.ActiveIdPreviousFrame = g.ActiveId;
    g.ActiveIdPreviousFrameWindow = g.ActiveIdWindow;
    g.ActiveIdPreviousFrameHasBeenEditedBefore = g.ActiveIdHasBeenEditedBefore;
    g.ActiveIdIsAlive = 0;
    g.ActiveIdHasBeenEditedThisFrame = false;
    g.ActiveIdPreviousFrameIsAlive = false;
    g.ActiveIdIsJustActivated = false;
    if (g.TempInputId != 0 && g.ActiveId != g.TempInputId)
        g.TempInputId = 0;
    if (g.ActiveId == 0)
    {
        g.ActiveIdUsingNavDirMask = 0x00;
        g.ActiveIdUsingNavInputMask = 0x00;
        g.ActiveIdUsingKeyInputMask = 0x00;
    }

    // Drag and drop
    g.DragDropAcceptIdPrev = g.DragDropAcceptIdCurr;
    g.DragDropAcceptIdCurr = 0;
    g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
    g.DragDropWithinSource = false;
    g.DragDropWithinTarget = false;
    g.DragDropHoldJustPressedId = 0;

    // Update keyboard input state
    // Synchronize io.KeyMods with individual modifiers io.KeyXXX bools
    g.IO.KeyMods = GetMergedKeyModFlags();
    memcpy(g.IO.KeysDownDurationPrev, g.IO.KeysDownDuration, sizeof(g.IO.KeysDownDuration));
    for (int i = 0; i < IM_ARRAYSIZE(g.IO.KeysDown); i++)
        g.IO.KeysDownDuration[i] =
            g.IO.KeysDown[i] ? (g.IO.KeysDownDuration[i] < 0.0f ? 0.0f : g.IO.KeysDownDuration[i] + g.IO.DeltaTime)
                             : -1.0f;

    // Update gamepad/keyboard navigation
    NavUpdate();

    // Update mouse input state
    UpdateMouseInputs();

    // Find hovered window
    // (needs to be before UpdateMouseMovingWindowNewFrame so we fill g.HoveredWindowUnderMovingWindow on the mouse
    // release frame)
    UpdateHoveredWindowAndCaptureFlags();

    // Handle user moving window with mouse (at the beginning of the frame to avoid input lag or sheering)
    UpdateMouseMovingWindowNewFrame();

    // Background darkening/whitening
    if (GetTopMostPopupModal() != NULL || (g.NavWindowingTarget != NULL && g.NavWindowingHighlightAlpha > 0.0f))
        g.DimBgRatio = ImMin(g.DimBgRatio + g.IO.DeltaTime * 6.0f, 1.0f);
    else
        g.DimBgRatio = ImMax(g.DimBgRatio - g.IO.DeltaTime * 10.0f, 0.0f);

    g.MouseCursor = ImGuiMouseCursor_Arrow;
    g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame = g.WantTextInputNextFrame = -1;
    g.PlatformImePos = ImVec2(1.0f, 1.0f); // OS Input Method Editor showing on top-left of our window by default

    // Mouse wheel scrolling, scale
    UpdateMouseWheel();

    // Update legacy TAB focus
    UpdateTabFocus();

    // Mark all windows as not visible and compact unused memory.
    IM_ASSERT(g.WindowsFocusOrder.Size <= g.Windows.Size);
    const float memory_compact_start_time = (g.GcCompactAll || g.IO.ConfigMemoryCompactTimer < 0.0f)
                                                ? FLT_MAX
                                                : (float)g.Time - g.IO.ConfigMemoryCompactTimer;
    for (int i = 0; i != g.Windows.Size; i++)
    {
        ImGuiWindow *window = g.Windows[i];
        window->WasActive = window->Active;
        window->BeginCount = 0;
        window->Active = false;
        window->WriteAccessed = false;

        // Garbage collect transient buffers of recently unused windows
        if (!window->WasActive && !window->MemoryCompacted && window->LastTimeActive < memory_compact_start_time)
            GcCompactTransientWindowBuffers(window);
    }

    // Garbage collect transient buffers of recently unused tables
    for (int i = 0; i < g.TablesLastTimeActive.Size; i++)
        if (g.TablesLastTimeActive[i] >= 0.0f && g.TablesLastTimeActive[i] < memory_compact_start_time)
            TableGcCompactTransientBuffers(g.Tables.GetByIndex(i));
    for (int i = 0; i < g.TablesTempDataStack.Size; i++)
        if (g.TablesTempDataStack[i].LastTimeActive >= 0.0f &&
            g.TablesTempDataStack[i].LastTimeActive < memory_compact_start_time)
            TableGcCompactTransientBuffers(&g.TablesTempDataStack[i]);
    if (g.GcCompactAll)
        GcCompactTransientMiscBuffers();
    g.GcCompactAll = false;

    // Closing the focused window restore focus to the first active root window in descending z-order
    if (g.NavWindow && !g.NavWindow->WasActive)
        FocusTopMostWindowUnderOne(NULL, NULL);

    // No window should be open at the beginning of the frame.
    // But in order to allow the user to call NewFrame() multiple times without calling Render(), we are doing an
    // explicit clear.
    g.CurrentWindowStack.resize(0);
    g.BeginPopupStack.resize(0);
    g.ItemFlagsStack.resize(0);
    g.ItemFlagsStack.push_back(ImGuiItemFlags_None);
    g.GroupStack.resize(0);
    ClosePopupsOverWindow(g.NavWindow, false);

    // [DEBUG] Item picker tool - start with DebugStartItemPicker() - useful to visually select an item and break into
    // its call-stack.
    UpdateDebugToolItemPicker();

    // Create implicit/fallback window - which we will only render it if the user has added something to it.
    // We don't use "Debug" to avoid colliding with user trying to create a "Debug" window with custom flags.
    // This fallback is particularly important as it avoid ImGui:: calls from crashing.
    g.WithinFrameScopeWithImplicitWindow = true;
    SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    Begin("Debug##Default");
    IM_ASSERT(g.CurrentWindow->IsFallbackWindow == true);

    CallContextHooks(&g, ImGuiContextHookType_NewFramePost);
}

/**
 *
 *
 * @param out_list
 * @param draw_list
 */
static void AddDrawListToDrawData(ImVector<ImDrawList *> *out_list, ImDrawList *draw_list);

/**
 *
 *
 * @param out_sorted_windows
 * @param window
 */
static void AddWindowToSortBuffer(ImVector<ImGuiWindow *> *out_sorted_windows, ImGuiWindow *window);

// // This is normally called by Render(). You may want to call it directly if you want to avoid calling Render() but
// the gain will be very minimal.
void ImGui::EndFrame()
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(g.Initialized);

    // Don't process EndFrame() multiple times.
    if (g.FrameCountEnded == g.FrameCount)
        return;
    IM_ASSERT(g.WithinFrameScope && "Forgot to call ImGui::NewFrame()?");

    CallContextHooks(&g, ImGuiContextHookType_EndFramePre);

    ErrorCheckEndFrameSanityChecks();

    // Notify OS when our Input Method Editor cursor has moved (e.g. CJK inputs using Microsoft IME)
    if (g.IO.ImeSetInputScreenPosFn &&
        (g.PlatformImeLastPos.x == FLT_MAX || ImLengthSqr(g.PlatformImeLastPos - g.PlatformImePos) > 0.0001f))
    {
        g.IO.ImeSetInputScreenPosFn((int)g.PlatformImePos.x, (int)g.PlatformImePos.y);
        g.PlatformImeLastPos = g.PlatformImePos;
    }

    // Hide implicit/fallback "Debug" window if it hasn't been used
    g.WithinFrameScopeWithImplicitWindow = false;
    if (g.CurrentWindow && !g.CurrentWindow->WriteAccessed)
        g.CurrentWindow->Active = false;
    End();

    // Update navigation: CTRL+Tab, wrap-around requests
    NavEndFrame();

    // Drag and Drop: Elapse payload (if delivered, or if source stops being submitted)
    if (g.DragDropActive)
    {
        bool is_delivered = g.DragDropPayload.Delivery;
        bool is_elapsed = (g.DragDropPayload.DataFrameCount + 1 < g.FrameCount) &&
                          ((g.DragDropSourceFlags & ImGuiDragDropFlags_SourceAutoExpirePayload) ||
                           !IsMouseDown(g.DragDropMouseButton));
        if (is_delivered || is_elapsed)
            ClearDragDrop();
    }

    // Drag and Drop: Fallback for source tooltip. This is not ideal but better than nothing.
    if (g.DragDropActive && g.DragDropSourceFrameCount < g.FrameCount &&
        !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
    {
        g.DragDropWithinSource = true;
        SetTooltip("...");
        g.DragDropWithinSource = false;
    }

    // End frame
    g.WithinFrameScope = false;
    g.FrameCountEnded = g.FrameCount;

    // Initiate moving window + handle left-click and right-click focus
    UpdateMouseMovingWindowEndFrame();

    // Sort the window list so that all child windows are after their parent
    // We cannot do that on FocusWindow() because children may not exist yet
    g.WindowsTempSortBuffer.resize(0);
    g.WindowsTempSortBuffer.reserve(g.Windows.Size);
    for (int i = 0; i != g.Windows.Size; i++)
    {
        ImGuiWindow *window = g.Windows[i];
        if (window->Active &&
            (window->Flags & ImGuiWindowFlags_ChildWindow)) // if a child is active its parent will add it
            continue;
        AddWindowToSortBuffer(&g.WindowsTempSortBuffer, window);
    }

    // This usually assert if there is a mismatch between the ImGuiWindowFlags_ChildWindow / ParentWindow values and
    // DC.ChildWindows[] in parents, aka we've done something wrong.
    IM_ASSERT(g.Windows.Size == g.WindowsTempSortBuffer.Size);
    g.Windows.swap(g.WindowsTempSortBuffer);
    g.IO.MetricsActiveWindows = g.WindowsActiveCount;

    // Unlock font atlas
    g.IO.Fonts->Locked = false;

    // Clear Input data for next frame
    g.IO.MouseWheel = g.IO.MouseWheelH = 0.0f;
    g.IO.InputQueueCharacters.resize(0);
    memset(g.IO.NavInputs, 0, sizeof(g.IO.NavInputs));

    CallContextHooks(&g, ImGuiContextHookType_EndFramePost);
}

static bool IsWindowActiveAndVisible(ImGuiWindow *window)
{
    return (window->Active) && (!window->Hidden);
}

static void SetupViewportDrawData(ImGuiViewportP *viewport, ImVector<ImDrawList *> *draw_lists)
{
    ImGuiIO &io = ImGui::GetIO();
    ImDrawData *draw_data = &viewport->DrawDataP;
    draw_data->Valid = true;
    draw_data->CmdLists = (draw_lists->Size > 0) ? draw_lists->Data : NULL;
    draw_data->CmdListsCount = draw_lists->Size;
    draw_data->TotalVtxCount = draw_data->TotalIdxCount = 0;
    draw_data->DisplayPos = viewport->Pos;
    draw_data->DisplaySize = viewport->Size;
    draw_data->FramebufferScale = io.DisplayFramebufferScale;
    for (int n = 0; n < draw_lists->Size; n++)
    {
        draw_data->TotalVtxCount += draw_lists->Data[n]->VtxBuffer.Size;
        draw_data->TotalIdxCount += draw_lists->Data[n]->IdxBuffer.Size;
    }
}

static void AddWindowToDrawData(ImGuiWindow *window, int layer)
{
    ImGuiContext &g = *GImGui;
    ImGuiViewportP *viewport = g.Viewports[0];
    g.IO.MetricsRenderWindows++;
    AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[layer], window->DrawList);
    for (int i = 0; i < window->DC.ChildWindows.Size; i++)
    {
        ImGuiWindow *child = window->DC.ChildWindows[i];
        if (IsWindowActiveAndVisible(child)) // Clipped children may have been marked not active
            AddWindowToDrawData(child, layer);
    }
}

static void AddRootWindowToDrawData(ImGuiWindow *window)
{
    int layer = (window->Flags & ImGuiWindowFlags_Tooltip) ? 0 : 0;
    AddWindowToDrawData(window, layer);
}

void ImGui::Render()
{
    ImGuiContext &g = *GImGui;
    IM_ASSERT(g.Initialized);

    if (g.FrameCountEnded != g.FrameCount)
        EndFrame();
    g.FrameCountRendered = g.FrameCount;
    g.IO.MetricsRenderWindows = 0;

    CallContextHooks(&g, ImGuiContextHookType_RenderPre);

    // Add background ImDrawList (for each active viewport)
    for (int n = 0; n != g.Viewports.Size; n++)
    {
        ImGuiViewportP *viewport = g.Viewports[n];
        viewport->DrawDataBuilder.Clear();
        if (viewport->DrawLists[0] != NULL)
            AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], GetBackgroundDrawList(viewport));
    }

    // Add ImDrawList to render
    ImGuiWindow *windows_to_render_top_most[2];
    windows_to_render_top_most[0] =
        (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus))
            ? g.NavWindowingTarget->RootWindow
            : NULL;
    windows_to_render_top_most[1] = (g.NavWindowingTarget ? g.NavWindowingListWindow : NULL);
    for (int n = 0; n != g.Windows.Size; n++)
    {
        ImGuiWindow *window = g.Windows[n];
        IM_MSVC_WARNING_SUPPRESS(
            6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
        if (IsWindowActiveAndVisible(window) && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0 &&
            window != windows_to_render_top_most[0] && window != windows_to_render_top_most[1])
            AddRootWindowToDrawData(window);
    }
    for (int n = 0; n < IM_ARRAYSIZE(windows_to_render_top_most); n++)
        if (windows_to_render_top_most[n] &&
            IsWindowActiveAndVisible(windows_to_render_top_most[n])) // NavWindowingTarget is always temporarily
                                                                     // displayed as the top-most window
            AddRootWindowToDrawData(windows_to_render_top_most[n]);

    // Setup ImDrawData structures for end-user
    g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = 0;
    for (int n = 0; n < g.Viewports.Size; n++)
    {
        ImGuiViewportP *viewport = g.Viewports[n];
        viewport->DrawDataBuilder.FlattenIntoSingleLayer();

        // Draw software mouse cursor if requested by io.MouseDrawCursor flag
        if (g.IO.MouseDrawCursor)
            RenderMouseCursor(GetForegroundDrawList(viewport), g.IO.MousePos, g.Style.MouseCursorScale, g.MouseCursor,
                              IM_COL32_WHITE, IM_COL32_BLACK, IM_COL32(0, 0, 0, 48));

        // Add foreground ImDrawList (for each active viewport)
        if (viewport->DrawLists[1] != NULL)
            AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], GetForegroundDrawList(viewport));

        SetupViewportDrawData(viewport, &viewport->DrawDataBuilder.Layers[0]);
        ImDrawData *draw_data = &viewport->DrawDataP;
        g.IO.MetricsRenderVertices += draw_data->TotalVtxCount;
        g.IO.MetricsRenderIndices += draw_data->TotalIdxCount;
    }

    CallContextHooks(&g, ImGuiContextHookType_RenderPost);
}

ImDrawData *ImGui::GetDrawData()
{
    ImGuiContext &g = *GImGui;
    ImGuiViewportP *viewport = g.Viewports[0];
    return viewport->DrawDataP.Valid ? &viewport->DrawDataP : NULL;
}

static void SetWindowConditionAllowFlags(ImGuiWindow *window, ImGuiCond flags, bool enabled)
{
    window->SetWindowPosAllowFlags =
        enabled ? (window->SetWindowPosAllowFlags | flags) : (window->SetWindowPosAllowFlags & ~flags);
    window->SetWindowSizeAllowFlags =
        enabled ? (window->SetWindowSizeAllowFlags | flags) : (window->SetWindowSizeAllowFlags & ~flags);
    window->SetWindowCollapsedAllowFlags =
        enabled ? (window->SetWindowCollapsedAllowFlags | flags) : (window->SetWindowCollapsedAllowFlags & ~flags);
}

static void ApplyWindowSettings(ImGuiWindow *window, ImGuiWindowSettings *settings)
{
    window->Pos = ImFloor(ImVec2(settings->Pos.x, settings->Pos.y));
    if (settings->Size.x > 0 && settings->Size.y > 0)
        window->Size = window->SizeFull = ImFloor(ImVec2(settings->Size.x, settings->Size.y));
    window->Collapsed = settings->Collapsed;
}

static ImGuiWindow *CreateNewWindow(const char *name, ImGuiWindowFlags flags)
{
    ImGuiContext &g = *GImGui;
    // IMGUI_DEBUG_LOG("CreateNewWindow '%s', flags = 0x%08X\n", name, flags);

    // Create window the first time
    ImGuiWindow *window = IM_NEW(ImGuiWindow)(&g, name);
    window->Flags = flags;
    g.WindowsById.SetVoidPtr(window->ID, window);

    // Default/arbitrary window position. Use SetNextWindowPos() with the appropriate condition flag to change the
    // initial position of a window.
    const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    window->Pos = main_viewport->Pos + ImVec2(60, 60);

    // User can disable loading and saving of settings. Tooltip and child windows also don't store settings.
    if (!(flags & ImGuiWindowFlags_NoSavedSettings))
        if (ImGuiWindowSettings *settings = ImGui::FindWindowSettings(window->ID))
        {
            // Retrieve settings from .ini file
            window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);
            SetWindowConditionAllowFlags(window, ImGuiCond_FirstUseEver, false);
            ApplyWindowSettings(window, settings);
        }
    window->DC.CursorStartPos = window->DC.CursorMaxPos =
        window->Pos; // So first call to CalcContentSize() doesn't return crazy values

    if ((flags & ImGuiWindowFlags_AlwaysAutoResize) != 0)
    {
        window->AutoFitFramesX = window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = false;
    }
    else
    {
        if (window->Size.x <= 0.0f)
            window->AutoFitFramesX = 2;
        if (window->Size.y <= 0.0f)
            window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = (window->AutoFitFramesX > 0) || (window->AutoFitFramesY > 0);
    }

    if (!(flags & ImGuiWindowFlags_ChildWindow))
    {
        g.WindowsFocusOrder.push_back(window);
        window->FocusOrder = (short)(g.WindowsFocusOrder.Size - 1);
    }

    if (flags & ImGuiWindowFlags_NoBringToFrontOnFocus)
        g.Windows.push_front(window); // Quite slow but rare and only once
    else
        g.Windows.push_back(window);
    return window;
}

 ImVec2 CalcWindowSizeAfterConstraint(ImGuiWindow *window, const ImVec2 &size_desired)
{
    ImGuiContext &g = *GImGui;
    ImVec2 new_size = size_desired;
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint)
    {
        // Using -1,-1 on either X/Y axis to preserve the current size.
        ImRect cr = g.NextWindowData.SizeConstraintRect;
        new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? ImClamp(new_size.x, cr.Min.x, cr.Max.x) : window->SizeFull.x;
        new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? ImClamp(new_size.y, cr.Min.y, cr.Max.y) : window->SizeFull.y;
        if (g.NextWindowData.SizeCallback)
        {
            ImGuiSizeCallbackData data;
            data.UserData = g.NextWindowData.SizeCallbackUserData;
            data.Pos = window->Pos;
            data.CurrentSize = window->SizeFull;
            data.DesiredSize = new_size;
            g.NextWindowData.SizeCallback(&data);
            new_size = data.DesiredSize;
        }
        new_size.x = IM_FLOOR(new_size.x);
        new_size.y = IM_FLOOR(new_size.y);
    }

    // Minimum size
    if (!(window->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_AlwaysAutoResize)))
    {
        ImGuiWindow *window_for_height = window;
        const float decoration_up_height = window_for_height->TitleBarHeight() + window_for_height->MenuBarHeight();
        new_size = ImMax(new_size, g.Style.WindowMinSize);
        new_size.y = ImMax(new_size.y,
                           decoration_up_height +
                               ImMax(0.0f, g.Style.WindowRounding - 1.0f)); // Reduce artifacts with very small windows
    }
    return new_size;
}

static void CalcWindowContentSizes(ImGuiWindow *window, ImVec2 *content_size_current, ImVec2 *content_size_ideal)
{
    bool preserve_old_content_sizes = false;
    if (window->Collapsed && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
        preserve_old_content_sizes = true;
    else if (window->Hidden && window->HiddenFramesCannotSkipItems == 0 && window->HiddenFramesCanSkipItems > 0)
        preserve_old_content_sizes = true;
    if (preserve_old_content_sizes)
    {
        *content_size_current = window->ContentSize;
        *content_size_ideal = window->ContentSizeIdeal;
        return;
    }

    content_size_current->x = (window->ContentSizeExplicit.x != 0.0f)
                                  ? window->ContentSizeExplicit.x
                                  : IM_FLOOR(window->DC.CursorMaxPos.x - window->DC.CursorStartPos.x);
    content_size_current->y = (window->ContentSizeExplicit.y != 0.0f)
                                  ? window->ContentSizeExplicit.y
                                  : IM_FLOOR(window->DC.CursorMaxPos.y - window->DC.CursorStartPos.y);
    content_size_ideal->x =
        (window->ContentSizeExplicit.x != 0.0f)
            ? window->ContentSizeExplicit.x
            : IM_FLOOR(ImMax(window->DC.CursorMaxPos.x, window->DC.IdealMaxPos.x) - window->DC.CursorStartPos.x);
    content_size_ideal->y =
        (window->ContentSizeExplicit.y != 0.0f)
            ? window->ContentSizeExplicit.y
            : IM_FLOOR(ImMax(window->DC.CursorMaxPos.y, window->DC.IdealMaxPos.y) - window->DC.CursorStartPos.y);
}

static ImVec2 CalcWindowAutoFitSize(ImGuiWindow *window, const ImVec2 &size_contents)
{
    ImGuiContext &g = *GImGui;
    ImGuiStyle &style = g.Style;
    const float decoration_up_height = window->TitleBarHeight() + window->MenuBarHeight();
    ImVec2 size_pad = window->WindowPadding * 2.0f;
    ImVec2 size_desired = size_contents + size_pad + ImVec2(0.0f, decoration_up_height);
    if (window->Flags & ImGuiWindowFlags_Tooltip)
    {
        // Tooltip always resize
        return size_desired;
    }
    else
    {
        // Maximum window size is determined by the viewport size or monitor size
        const bool is_popup = (window->Flags & ImGuiWindowFlags_Popup) != 0;
        const bool is_menu = (window->Flags & ImGuiWindowFlags_ChildMenu) != 0;
        ImVec2 size_min = style.WindowMinSize;
        if (is_popup || is_menu) // Popups and menus bypass style.WindowMinSize by default, but we give then a non-zero
                                 // minimum size to facilitate understanding problematic cases (e.g. empty popups)
            size_min = ImMin(size_min, ImVec2(4.0f, 4.0f));

        // FIXME-VIEWPORT-WORKAREA: May want to use GetWorkSize() instead of Size depending on the type of windows?
        ImVec2 avail_size = ImGui::GetMainViewport()->Size;
        ImVec2 size_auto_fit =
            ImClamp(size_desired, size_min, ImMax(size_min, avail_size - style.DisplaySafeAreaPadding * 2.0f));

        // When the window cannot fit all contents (either because of constraints, either because screen is too small),
        // we are growing the size on the other axis to compensate for expected scrollbar. FIXME: Might turn bigger than
        // ViewportSize-WindowPadding.
        ImVec2 size_auto_fit_after_constraint = CalcWindowSizeAfterConstraint(window, size_auto_fit);
        bool will_have_scrollbar_x = (size_auto_fit_after_constraint.x - size_pad.x - 0.0f < size_contents.x &&
                                      !(window->Flags & ImGuiWindowFlags_NoScrollbar) &&
                                      (window->Flags & ImGuiWindowFlags_HorizontalScrollbar)) ||
                                     (window->Flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        bool will_have_scrollbar_y =
            (size_auto_fit_after_constraint.y - size_pad.y - decoration_up_height < size_contents.y &&
             !(window->Flags & ImGuiWindowFlags_NoScrollbar)) ||
            (window->Flags & ImGuiWindowFlags_AlwaysVerticalScrollbar);
        if (will_have_scrollbar_x)
            size_auto_fit.y += style.ScrollbarSize;
        if (will_have_scrollbar_y)
            size_auto_fit.x += style.ScrollbarSize;
        return size_auto_fit;
    }
}

ImVec2 ImGui::CalcWindowNextAutoFitSize(ImGuiWindow *window)
{
    ImVec2 size_contents_current;
    ImVec2 size_contents_ideal;
    CalcWindowContentSizes(window, &size_contents_current, &size_contents_ideal);
    ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, size_contents_ideal);
    ImVec2 size_final = CalcWindowSizeAfterConstraint(window, size_auto_fit);
    return size_final;
}

void CalcResizePosSizeFromAnyCorner(ImGuiWindow *window, const ImVec2 &corner_target, const ImVec2 &corner_norm,
                                           ImVec2 *out_pos, ImVec2 *out_size)
{
    ImVec2 pos_min = ImLerp(corner_target, window->Pos, corner_norm);                // Expected window upper-left
    ImVec2 pos_max = ImLerp(window->Pos + window->Size, corner_target, corner_norm); // Expected window lower-right
    ImVec2 size_expected = pos_max - pos_min;
    ImVec2 size_constrained = CalcWindowSizeAfterConstraint(window, size_expected);
    *out_pos = pos_min;
    if (corner_norm.x == 0.0f)
        out_pos->x -= (size_constrained.x - size_expected.x);
    if (corner_norm.y == 0.0f)
        out_pos->y -= (size_constrained.y - size_expected.y);
    *out_size = size_constrained;
}

static void SetCurrentWindow(ImGuiWindow *window)
{
    ImGuiContext &g = *GImGui;
    g.CurrentWindow = window;
    g.CurrentTable =
        window && window->DC.CurrentTableIdx != -1 ? g.Tables.GetByIndex(window->DC.CurrentTableIdx) : NULL;
    if (window)
        g.FontSize = g.DrawListSharedData.FontSize = window->CalcFontSize();
}

static inline void ClampWindowRect(ImGuiWindow *window, const ImRect &visibility_rect)
{
    ImGuiContext &g = *GImGui;
    ImVec2 size_for_clamping = window->Size;
    if (g.IO.ConfigWindowsMoveFromTitleBarOnly && !(window->Flags & ImGuiWindowFlags_NoTitleBar))
        size_for_clamping.y = window->TitleBarHeight();
    window->Pos = ImClamp(window->Pos, visibility_rect.Min - size_for_clamping, visibility_rect.Max);
}

// Helper to snap on edges when aiming at an item very close to the edge,
// So the difference between WindowPadding and ItemSpacing will be in the visible area after scrolling.
// When we refactor the scrolling API this may be configurable with a flag?
// Note that the effect for this won't be visible on X axis with default Style settings as WindowPadding.x ==
// ItemSpacing.x by default.
static float CalcScrollEdgeSnap(float target, float snap_min, float snap_max, float snap_threshold, float center_ratio)
{
    if (target <= snap_min + snap_threshold)
        return ImLerp(snap_min, target, center_ratio);
    if (target >= snap_max - snap_threshold)
        return ImLerp(target, snap_max, center_ratio);
    return target;
}

static ImVec2 CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow *window)
{
    ImVec2 scroll = window->Scroll;
    if (window->ScrollTarget.x < FLT_MAX)
    {
        float decoration_total_width = window->ScrollbarSizes.x;
        float center_x_ratio = window->ScrollTargetCenterRatio.x;
        float scroll_target_x = window->ScrollTarget.x;
        if (window->ScrollTargetEdgeSnapDist.x > 0.0f)
        {
            float snap_x_min = 0.0f;
            float snap_x_max = window->ScrollMax.x + window->SizeFull.x - decoration_total_width;
            scroll_target_x = CalcScrollEdgeSnap(scroll_target_x, snap_x_min, snap_x_max,
                                                 window->ScrollTargetEdgeSnapDist.x, center_x_ratio);
        }
        scroll.x = scroll_target_x - center_x_ratio * (window->SizeFull.x - decoration_total_width);
    }
    if (window->ScrollTarget.y < FLT_MAX)
    {
        float decoration_total_height = window->TitleBarHeight() + window->MenuBarHeight() + window->ScrollbarSizes.y;
        float center_y_ratio = window->ScrollTargetCenterRatio.y;
        float scroll_target_y = window->ScrollTarget.y;
        if (window->ScrollTargetEdgeSnapDist.y > 0.0f)
        {
            float snap_y_min = 0.0f;
            float snap_y_max = window->ScrollMax.y + window->SizeFull.y - decoration_total_height;
            scroll_target_y = CalcScrollEdgeSnap(scroll_target_y, snap_y_min, snap_y_max,
                                                 window->ScrollTargetEdgeSnapDist.y, center_y_ratio);
        }
        scroll.y = scroll_target_y - center_y_ratio * (window->SizeFull.y - decoration_total_height);
    }
    scroll.x = IM_FLOOR(ImMax(scroll.x, 0.0f));
    scroll.y = IM_FLOOR(ImMax(scroll.y, 0.0f));
    if (!window->Collapsed && !window->SkipItems)
    {
        scroll.x = ImMin(scroll.x, window->ScrollMax.x);
        scroll.y = ImMin(scroll.y, window->ScrollMax.y);
    }
    return scroll;
}

// Push a new Dear ImGui window to add widgets to.
// - A default window called "Debug" is automatically stacked at the beginning of every frame so you can use widgets
// without explicitly calling a Begin/End pair.
// - Begin/End can be called multiple times during the frame with the same window name to append content.
// - The window name is used as a unique identifier to preserve window information across frames (and save rudimentary
// information to the .ini file).
//   You can use the "##" or "###" markers to use the same label with different id, or same id with different label. See
//   documentation at the top of this file.
// - Return false when window is collapsed, so you can early out in your code. You always need to call ImGui::End() even
// if false is returned.
// - Passing 'bool* p_open' displays a Close button on the upper-right corner of the window, the pointed value will be
// set to false when the button is pressed.
bool ImGui::Begin(const char *name, bool *p_open, ImGuiWindowFlags flags)
{
    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    IM_ASSERT(name != NULL && name[0] != '\0'); // Window name required
    IM_ASSERT(g.WithinFrameScope);              // Forgot to call ImGui::NewFrame()
    IM_ASSERT(
        g.FrameCountEnded !=
        g.FrameCount); // Called ImGui::Render() or ImGui::EndFrame() and haven't called ImGui::NewFrame() again yet

    // Find or create
    ImGuiWindow *window = FindWindowByName(name);
    const bool window_just_created = (window == NULL);
    if (window_just_created)
        window = CreateNewWindow(name, flags);

    // Automatically disable manual moving/resizing when NoInputs is set
    if ((flags & ImGuiWindowFlags_NoInputs) == ImGuiWindowFlags_NoInputs)
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (flags & ImGuiWindowFlags_NavFlattened)
        IM_ASSERT(flags & ImGuiWindowFlags_ChildWindow);

    const int current_frame = g.FrameCount;
    const bool first_begin_of_the_frame = (window->LastFrameActive != current_frame);
    window->IsFallbackWindow = (g.CurrentWindowStack.Size == 0 && g.WithinFrameScopeWithImplicitWindow);

    // Update the Appearing flag
    bool window_just_activated_by_user =
        (window->LastFrameActive <
         current_frame - 1); // Not using !WasActive because the implicit "Debug" window would always toggle off->on
    if (flags & ImGuiWindowFlags_Popup)
    {
        ImGuiPopupData &popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
        window_just_activated_by_user |=
            (window->PopupId !=
             popup_ref.PopupId); // We recycle popups so treat window as activated if popup id changed
        window_just_activated_by_user |= (window != popup_ref.Window);
    }
    window->Appearing = window_just_activated_by_user;
    if (window->Appearing)
        SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, true);

    // Update Flags, LastFrameActive, BeginOrderXXX fields
    if (first_begin_of_the_frame)
    {
        window->Flags = (ImGuiWindowFlags)flags;
        window->LastFrameActive = current_frame;
        window->LastTimeActive = (float)g.Time;
        window->BeginOrderWithinParent = 0;
        window->BeginOrderWithinContext = (short)(g.WindowsActiveCount++);
    }
    else
    {
        flags = window->Flags;
    }

    // Parent window is latched only on the first call to Begin() of the frame, so further append-calls can be done from
    // a different window stack
    ImGuiWindow *parent_window_in_stack = g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back();
    ImGuiWindow *parent_window =
        first_begin_of_the_frame
            ? ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup)) ? parent_window_in_stack : NULL)
            : window->ParentWindow;
    IM_ASSERT(parent_window != NULL || !(flags & ImGuiWindowFlags_ChildWindow));

    // We allow window memory to be compacted so recreate the base stack when needed.
    if (window->IDStack.Size == 0)
        window->IDStack.push_back(window->ID);

    // Add to stack
    // We intentionally set g.CurrentWindow to NULL to prevent usage until when the viewport is set, then will call
    // SetCurrentWindow()
    g.CurrentWindowStack.push_back(window);
    g.CurrentWindow = window;
    window->DC.StackSizesOnBegin.SetToCurrentState();
    g.CurrentWindow = NULL;

    if (flags & ImGuiWindowFlags_Popup)
    {
        ImGuiPopupData &popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
        popup_ref.Window = window;
        g.BeginPopupStack.push_back(popup_ref);
        window->PopupId = popup_ref.PopupId;
    }

    // Update ->RootWindow and others pointers (before any possible call to FocusWindow)
    if (first_begin_of_the_frame)
        UpdateWindowParentAndRootLinks(window, flags, parent_window);

    // Process SetNextWindow***() calls
    // (FIXME: Consider splitting the HasXXX flags into X/Y components
    bool window_pos_set_by_api = false;
    bool window_size_x_set_by_api = false, window_size_y_set_by_api = false;
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasPos)
    {
        window_pos_set_by_api = (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) != 0;
        if (window_pos_set_by_api && ImLengthSqr(g.NextWindowData.PosPivotVal) > 0.00001f)
        {
            // May be processed on the next frame if this is our first frame and we are measuring size
            // FIXME: Look into removing the branch so everything can go through this same code path for consistency.
            window->SetWindowPosVal = g.NextWindowData.PosVal;
            window->SetWindowPosPivot = g.NextWindowData.PosPivotVal;
            window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
        }
        else
        {
            SetWindowPos(window, g.NextWindowData.PosVal, g.NextWindowData.PosCond);
        }
    }
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSize)
    {
        window_size_x_set_by_api =
            (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.x > 0.0f);
        window_size_y_set_by_api =
            (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.y > 0.0f);
        SetWindowSize(window, g.NextWindowData.SizeVal, g.NextWindowData.SizeCond);
    }
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasScroll)
    {
        if (g.NextWindowData.ScrollVal.x >= 0.0f)
        {
            window->ScrollTarget.x = g.NextWindowData.ScrollVal.x;
            window->ScrollTargetCenterRatio.x = 0.0f;
        }
        if (g.NextWindowData.ScrollVal.y >= 0.0f)
        {
            window->ScrollTarget.y = g.NextWindowData.ScrollVal.y;
            window->ScrollTargetCenterRatio.y = 0.0f;
        }
    }
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasContentSize)
        window->ContentSizeExplicit = g.NextWindowData.ContentSizeVal;
    else if (first_begin_of_the_frame)
        window->ContentSizeExplicit = ImVec2(0.0f, 0.0f);
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasCollapsed)
        SetWindowCollapsed(window, g.NextWindowData.CollapsedVal, g.NextWindowData.CollapsedCond);
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasFocus)
        FocusWindow(window);
    if (window->Appearing)
        SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, false);

    // When reusing window again multiple times a frame, just append content (don't need to setup again)
    if (first_begin_of_the_frame)
    {
        // Initialize
        const bool window_is_child_tooltip =
            (flags & ImGuiWindowFlags_ChildWindow) &&
            (flags &
             ImGuiWindowFlags_Tooltip); // FIXME-WIP: Undocumented behavior of Child+Tooltip for pinned tooltip (#1345)
        window->Active = true;
        window->HasCloseButton = (p_open != NULL);
        window->ClipRect = ImVec4(-FLT_MAX, -FLT_MAX, +FLT_MAX, +FLT_MAX);
        window->IDStack.resize(1);
        window->DrawList->_ResetForNewFrame();
        window->DC.CurrentTableIdx = -1;

        // Restore buffer capacity when woken from a compacted state, to avoid
        if (window->MemoryCompacted)
            GcAwakeTransientWindowBuffers(window);

        // Update stored window name when it changes (which can _only_ happen with the "###" operator, so the ID would
        // stay unchanged). The title bar always display the 'name' parameter, so we only update the string storage if
        // it needs to be visible to the end-user elsewhere.
        bool window_title_visible_elsewhere = false;
        if (g.NavWindowingListWindow != NULL &&
            (window->Flags & ImGuiWindowFlags_NoNavFocus) == 0) // Window titles visible when using CTRL+TAB
            window_title_visible_elsewhere = true;
        if (window_title_visible_elsewhere && !window_just_created && strcmp(name, window->Name) != 0)
        {
            size_t buf_len = (size_t)window->NameBufLen;
            window->Name = ImStrdupcpy(window->Name, &buf_len, name);
            window->NameBufLen = (int)buf_len;
        }

        // UPDATE CONTENTS SIZE, UPDATE HIDDEN STATUS

        // Update contents size from last frame for auto-fitting (or use explicit size)
        const bool window_just_appearing_after_hidden_for_resize = (window->HiddenFramesCannotSkipItems > 0);
        CalcWindowContentSizes(window, &window->ContentSize, &window->ContentSizeIdeal);
        if (window->HiddenFramesCanSkipItems > 0)
            window->HiddenFramesCanSkipItems--;
        if (window->HiddenFramesCannotSkipItems > 0)
            window->HiddenFramesCannotSkipItems--;
        if (window->HiddenFramesForRenderOnly > 0)
            window->HiddenFramesForRenderOnly--;

        // Hide new windows for one frame until they calculate their size
        if (window_just_created && (!window_size_x_set_by_api || !window_size_y_set_by_api))
            window->HiddenFramesCannotSkipItems = 1;

        // Hide popup/tooltip window when re-opening while we measure size (because we recycle the windows)
        // We reset Size/ContentSize for reappearing popups/tooltips early in this function, so further code won't be
        // tempted to use the old size.
        if (window_just_activated_by_user && (flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) != 0)
        {
            window->HiddenFramesCannotSkipItems = 1;
            if (flags & ImGuiWindowFlags_AlwaysAutoResize)
            {
                if (!window_size_x_set_by_api)
                    window->Size.x = window->SizeFull.x = 0.f;
                if (!window_size_y_set_by_api)
                    window->Size.y = window->SizeFull.y = 0.f;
                window->ContentSize = window->ContentSizeIdeal = ImVec2(0.f, 0.f);
            }
        }

        // SELECT VIEWPORT
        // FIXME-VIEWPORT: In the docking/viewport branch, this is the point where we select the current viewport (which
        // may affect the style)
        SetCurrentWindow(window);

        // LOCK BORDER SIZE AND PADDING FOR THE FRAME (so that altering them doesn't cause inconsistencies)

        if (flags & ImGuiWindowFlags_ChildWindow)
            window->WindowBorderSize = style.ChildBorderSize;
        else
            window->WindowBorderSize =
                ((flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) && !(flags & ImGuiWindowFlags_Modal))
                    ? style.PopupBorderSize
                    : style.WindowBorderSize;
        window->WindowPadding = style.WindowPadding;
        if ((flags & ImGuiWindowFlags_ChildWindow) &&
            !(flags & (ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_Popup)) &&
            window->WindowBorderSize == 0.0f)
            window->WindowPadding = ImVec2(0.0f, (flags & ImGuiWindowFlags_MenuBar) ? style.WindowPadding.y : 0.0f);

        // Lock menu offset so size calculation can use it as menu-bar windows need a minimum size.
        window->DC.MenuBarOffset.x =
            ImMax(ImMax(window->WindowPadding.x, style.ItemSpacing.x), g.NextWindowData.MenuBarOffsetMinVal.x);
        window->DC.MenuBarOffset.y = g.NextWindowData.MenuBarOffsetMinVal.y;

        // Collapse window by double-clicking on title bar
        // At this point we don't have a clipping rectangle setup yet, so we can use the title bar area for hit
        // detection and drawing
        if (!(flags & ImGuiWindowFlags_NoTitleBar) && !(flags & ImGuiWindowFlags_NoCollapse))
        {
            // We don't use a regular button+id to test for double-click on title bar (mostly due to legacy reason,
            // could be fixed), so verify that we don't have items over the title bar.
            ImRect title_bar_rect = window->TitleBarRect();
            if (g.HoveredWindow == window && g.HoveredId == 0 && g.HoveredIdPreviousFrame == 0 &&
                IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max) && g.IO.MouseDoubleClicked[0])
                window->WantCollapseToggle = true;
            if (window->WantCollapseToggle)
            {
                window->Collapsed = !window->Collapsed;
                MarkIniSettingsDirty(window);
            }
        }
        else
        {
            window->Collapsed = false;
        }
        window->WantCollapseToggle = false;

        // SIZE

        // Calculate auto-fit size, handle automatic resize
        const ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal);
        bool use_current_size_for_scrollbar_x = window_just_created;
        bool use_current_size_for_scrollbar_y = window_just_created;
        if ((flags & ImGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed)
        {
            // Using SetNextWindowSize() overrides ImGuiWindowFlags_AlwaysAutoResize, so it can be used on
            // tooltips/popups, etc.
            if (!window_size_x_set_by_api)
            {
                window->SizeFull.x = size_auto_fit.x;
                use_current_size_for_scrollbar_x = true;
            }
            if (!window_size_y_set_by_api)
            {
                window->SizeFull.y = size_auto_fit.y;
                use_current_size_for_scrollbar_y = true;
            }
        }
        else if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
        {
            // Auto-fit may only grow window during the first few frames
            // We still process initial auto-fit on collapsed windows to get a window width, but otherwise don't honor
            // ImGuiWindowFlags_AlwaysAutoResize when collapsed.
            if (!window_size_x_set_by_api && window->AutoFitFramesX > 0)
            {
                window->SizeFull.x =
                    window->AutoFitOnlyGrows ? ImMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
                use_current_size_for_scrollbar_x = true;
            }
            if (!window_size_y_set_by_api && window->AutoFitFramesY > 0)
            {
                window->SizeFull.y =
                    window->AutoFitOnlyGrows ? ImMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
                use_current_size_for_scrollbar_y = true;
            }
            if (!window->Collapsed)
                MarkIniSettingsDirty(window);
        }

        // Apply minimum/maximum window size constraints and final size
        window->SizeFull = CalcWindowSizeAfterConstraint(window, window->SizeFull);
        window->Size = window->Collapsed && !(flags & ImGuiWindowFlags_ChildWindow) ? window->TitleBarRect().GetSize()
                                                                                    : window->SizeFull;

        // Decoration size
        const float decoration_up_height = window->TitleBarHeight() + window->MenuBarHeight();

        // POSITION

        // Popup latch its initial position, will position itself when it appears next frame
        if (window_just_activated_by_user)
        {
            window->AutoPosLastDirection = ImGuiDir_None;
            if ((flags & ImGuiWindowFlags_Popup) != 0 && !(flags & ImGuiWindowFlags_Modal) &&
                !window_pos_set_by_api) // FIXME: BeginPopup() could use SetNextWindowPos()
                window->Pos = g.BeginPopupStack.back().OpenPopupPos;
        }

        // Position child window
        if (flags & ImGuiWindowFlags_ChildWindow)
        {
            IM_ASSERT(parent_window && parent_window->Active);
            window->BeginOrderWithinParent = (short)parent_window->DC.ChildWindows.Size;
            parent_window->DC.ChildWindows.push_back(window);
            if (!(flags & ImGuiWindowFlags_Popup) && !window_pos_set_by_api && !window_is_child_tooltip)
                window->Pos = parent_window->DC.CursorPos;
        }

        const bool window_pos_with_pivot =
            (window->SetWindowPosVal.x != FLT_MAX && window->HiddenFramesCannotSkipItems == 0);
        if (window_pos_with_pivot)
            SetWindowPos(window, window->SetWindowPosVal - window->Size * window->SetWindowPosPivot,
                         0); // Position given a pivot (e.g. for centering)
        else if ((flags & ImGuiWindowFlags_ChildMenu) != 0)
            window->Pos = FindBestWindowPosForPopup(window);
        else if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api &&
                 window_just_appearing_after_hidden_for_resize)
            window->Pos = FindBestWindowPosForPopup(window);
        else if ((flags & ImGuiWindowFlags_Tooltip) != 0 && !window_pos_set_by_api && !window_is_child_tooltip)
            window->Pos = FindBestWindowPosForPopup(window);

        // Calculate the range of allowed position for that window (to be movable and visible past safe area padding)
        // When clamping to stay visible, we will enforce that window->Pos stays inside of visibility_rect.
        ImGuiViewportP *viewport = (ImGuiViewportP *)(void *)GetMainViewport();
        ImRect viewport_rect(viewport->GetMainRect());
        ImRect viewport_work_rect(viewport->GetWorkRect());
        ImVec2 visibility_padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
        ImRect visibility_rect(viewport_work_rect.Min + visibility_padding,
                               viewport_work_rect.Max - visibility_padding);

        // Clamp position/size so window stays visible within its viewport or monitor
        // Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window
        // when initializing or minimizing.
        if (!window_pos_set_by_api && !(flags & ImGuiWindowFlags_ChildWindow) && window->AutoFitFramesX <= 0 &&
            window->AutoFitFramesY <= 0)
            if (viewport_rect.GetWidth() > 0.0f && viewport_rect.GetHeight() > 0.0f)
                ClampWindowRect(window, visibility_rect);
        window->Pos = ImFloor(window->Pos);

        // Lock window rounding for the frame (so that altering them doesn't cause inconsistencies)
        // Large values tend to lead to variety of artifacts and are not recommended.
        window->WindowRounding = (flags & ImGuiWindowFlags_ChildWindow) ? style.ChildRounding
                                 : ((flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiWindowFlags_Modal))
                                     ? style.PopupRounding
                                     : style.WindowRounding;

        // For windows with title bar or menu bar, we clamp to FrameHeight(FontSize + FramePadding.y * 2.0f) to
        // completely hide artifacts.
        // if ((window->Flags & ImGuiWindowFlags_MenuBar) || !(window->Flags & ImGuiWindowFlags_NoTitleBar))
        //    window->WindowRounding = ImMin(window->WindowRounding, g.FontSize + style.FramePadding.y * 2.0f);

        // Apply window focus (new and reactivated windows are moved to front)
        bool want_focus = false;
        if (window_just_activated_by_user && !(flags & ImGuiWindowFlags_NoFocusOnAppearing))
        {
            if (flags & ImGuiWindowFlags_Popup)
                want_focus = true;
            else if ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Tooltip)) == 0)
                want_focus = true;
        }

        // Handle manual resize: Resize Grips, Borders, Gamepad
        int border_held = -1;
        ImU32 resize_grip_col[4] = {};
        const int resize_grip_count =
            g.IO.ConfigWindowsResizeFromEdges
                ? 2
                : 1; // Allow resize from lower-left if we have the mouse cursor feedback for it.
        const float resize_grip_draw_size =
            IM_FLOOR(ImMax(g.FontSize * 1.10f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
        if (!window->Collapsed)
            if (UpdateWindowManualResize(window, size_auto_fit, &border_held, resize_grip_count, &resize_grip_col[0],
                                         visibility_rect))
                use_current_size_for_scrollbar_x = use_current_size_for_scrollbar_y = true;
        window->ResizeBorderHeld = (signed char)border_held;

        // SCROLLBAR VISIBILITY

        // Update scrollbar visibility (based on the Size that was effective during last frame or the auto-resized
        // Size).
        if (!window->Collapsed)
        {
            // When reading the current size we need to read it after size constraints have been applied.
            // When we use InnerRect here we are intentionally reading last frame size, same for ScrollbarSizes values
            // before we set them again.
            ImVec2 avail_size_from_current_frame =
                ImVec2(window->SizeFull.x, window->SizeFull.y - decoration_up_height);
            ImVec2 avail_size_from_last_frame = window->InnerRect.GetSize() + window->ScrollbarSizes;
            ImVec2 needed_size_from_last_frame =
                window_just_created ? ImVec2(0, 0) : window->ContentSize + window->WindowPadding * 2.0f;
            float size_x_for_scrollbars =
                use_current_size_for_scrollbar_x ? avail_size_from_current_frame.x : avail_size_from_last_frame.x;
            float size_y_for_scrollbars =
                use_current_size_for_scrollbar_y ? avail_size_from_current_frame.y : avail_size_from_last_frame.y;
            // bool scrollbar_y_from_last_frame = window->ScrollbarY; // FIXME: May want to use that in the ScrollbarX
            // expression? How many pros vs cons?
            window->ScrollbarY =
                (flags & ImGuiWindowFlags_AlwaysVerticalScrollbar) ||
                ((needed_size_from_last_frame.y > size_y_for_scrollbars) && !(flags & ImGuiWindowFlags_NoScrollbar));
            window->ScrollbarX =
                (flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar) ||
                ((needed_size_from_last_frame.x >
                  size_x_for_scrollbars - (window->ScrollbarY ? style.ScrollbarSize : 0.0f)) &&
                 !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar));
            if (window->ScrollbarX && !window->ScrollbarY)
                window->ScrollbarY =
                    (needed_size_from_last_frame.y > size_y_for_scrollbars) && !(flags & ImGuiWindowFlags_NoScrollbar);
            window->ScrollbarSizes = ImVec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f,
                                            window->ScrollbarX ? style.ScrollbarSize : 0.0f);
        }

        // UPDATE RECTANGLES (1- THOSE NOT AFFECTED BY SCROLLING)
        // Update various regions. Variables they depends on should be set above in this function.
        // We set this up after processing the resize grip so that our rectangles doesn't lag by a frame.

        // Outer rectangle
        // Not affected by window border size. Used by:
        // - FindHoveredWindow() (w/ extra padding when border resize is enabled)
        // - Begin() initial clipping rect for drawing window background and borders.
        // - Begin() clipping whole child
        const ImRect host_rect =
            ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip)
                ? parent_window->ClipRect
                : viewport_rect;
        const ImRect outer_rect = window->Rect();
        const ImRect title_bar_rect = window->TitleBarRect();
        window->OuterRectClipped = outer_rect;
        window->OuterRectClipped.ClipWith(host_rect);

        // Inner rectangle
        // Not affected by window border size. Used by:
        // - InnerClipRect
        // - ScrollToBringRectIntoView()
        // - NavUpdatePageUpPageDown()
        // - Scrollbar()
        window->InnerRect.Min.x = window->Pos.x;
        window->InnerRect.Min.y = window->Pos.y + decoration_up_height;
        window->InnerRect.Max.x = window->Pos.x + window->Size.x - window->ScrollbarSizes.x;
        window->InnerRect.Max.y = window->Pos.y + window->Size.y - window->ScrollbarSizes.y;

        // Inner clipping rectangle.
        // Will extend a little bit outside the normal work region.
        // This is to allow e.g. Selectable or CollapsingHeader or some separators to cover that space.
        // Force round operator last to ensure that e.g. (int)(max.x-min.x) in user's render code produce correct
        // result. Note that if our window is collapsed we will end up with an inverted (~null) clipping rectangle which
        // is the correct behavior. Affected by window/frame border size. Used by:
        // - Begin() initial clip rect
        float top_border_size =
            (((flags & ImGuiWindowFlags_MenuBar) || !(flags & ImGuiWindowFlags_NoTitleBar)) ? style.FrameBorderSize
                                                                                            : window->WindowBorderSize);
        window->InnerClipRect.Min.x = ImFloor(0.5f + window->InnerRect.Min.x +
                                              ImMax(ImFloor(window->WindowPadding.x * 0.5f), window->WindowBorderSize));
        window->InnerClipRect.Min.y = ImFloor(0.5f + window->InnerRect.Min.y + top_border_size);
        window->InnerClipRect.Max.x = ImFloor(0.5f + window->InnerRect.Max.x -
                                              ImMax(ImFloor(window->WindowPadding.x * 0.5f), window->WindowBorderSize));
        window->InnerClipRect.Max.y = ImFloor(0.5f + window->InnerRect.Max.y - window->WindowBorderSize);
        window->InnerClipRect.ClipWithFull(host_rect);

        // Default item width. Make it proportional to window size if window manually resizes
        if (window->Size.x > 0.0f && !(flags & ImGuiWindowFlags_Tooltip) &&
            !(flags & ImGuiWindowFlags_AlwaysAutoResize))
            window->ItemWidthDefault = ImFloor(window->Size.x * 0.65f);
        else
            window->ItemWidthDefault = ImFloor(g.FontSize * 16.0f);

        // SCROLLING

        // Lock down maximum scrolling
        // The value of ScrollMax are ahead from ScrollbarX/ScrollbarY which is intentionally using InnerRect from
        // previous rect in order to accommodate for right/bottom aligned items without creating a scrollbar.
        window->ScrollMax.x =
            ImMax(0.0f, window->ContentSize.x + window->WindowPadding.x * 2.0f - window->InnerRect.GetWidth());
        window->ScrollMax.y =
            ImMax(0.0f, window->ContentSize.y + window->WindowPadding.y * 2.0f - window->InnerRect.GetHeight());

        // Apply scrolling
        window->Scroll = CalcNextScrollFromScrollTargetAndClamp(window);
        window->ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);

        // DRAWING

        // Setup draw list and outer clipping rectangle
        IM_ASSERT(window->DrawList->CmdBuffer.Size == 1 && window->DrawList->CmdBuffer[0].ElemCount == 0);
        window->DrawList->PushTextureID(g.Font->ContainerAtlas->TexID);
        PushClipRect(host_rect.Min, host_rect.Max, false);

        // Draw modal window background (darkens what is behind them, all viewports)
        const bool dim_bg_for_modal = (flags & ImGuiWindowFlags_Modal) && window == GetTopMostPopupModal() &&
                                      window->HiddenFramesCannotSkipItems <= 0;
        const bool dim_bg_for_window_list =
            g.NavWindowingTargetAnim && (window == g.NavWindowingTargetAnim->RootWindow);
        if (dim_bg_for_modal || dim_bg_for_window_list)
        {
            const ImU32 dim_bg_col =
                GetColorU32(dim_bg_for_modal ? ImGuiCol_ModalWindowDimBg : ImGuiCol_NavWindowingDimBg, g.DimBgRatio);
            window->DrawList->AddRectFilled(viewport_rect.Min, viewport_rect.Max, dim_bg_col);
        }

        // Draw navigation selection/windowing rectangle background
        if (dim_bg_for_window_list && window == g.NavWindowingTargetAnim)
        {
            ImRect bb = window->Rect();
            bb.Expand(g.FontSize);
            if (!bb.Contains(viewport_rect)) // Avoid drawing if the window covers all the viewport anyway
                window->DrawList->AddRectFilled(
                    bb.Min, bb.Max, GetColorU32(ImGuiCol_NavWindowingHighlight, g.NavWindowingHighlightAlpha * 0.25f),
                    g.Style.WindowRounding);
        }

        // Since 1.71, child window can render their decoration (bg color, border, scrollbars, etc.) within their parent
        // to save a draw call. When using overlapping child windows, this will break the assumption that child z-order
        // is mapped to submission order. We disable this when the parent window has zero vertices, which is a common
        // pattern leading to laying out multiple overlapping child. We also disabled this when we have dimming overlay
        // behind this specific one child.
        // FIXME: More code may rely on explicit sorting of overlapping child window and would need to disable this
        // somehow. Please get in contact if you are affected.
        {
            bool render_decorations_in_parent = false;
            if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip)
                if (window->DrawList->CmdBuffer.back().ElemCount == 0 && parent_window->DrawList->VtxBuffer.Size > 0)
                    render_decorations_in_parent = true;
            if (render_decorations_in_parent)
                window->DrawList = parent_window->DrawList;

            // Handle title bar, scrollbar, resize grips and resize borders
            const ImGuiWindow *window_to_highlight = g.NavWindowingTarget ? g.NavWindowingTarget : g.NavWindow;
            const bool title_bar_is_highlight =
                want_focus || (window_to_highlight && window->RootWindowForTitleBarHighlight ==
                                                          window_to_highlight->RootWindowForTitleBarHighlight);
            RenderWindowDecorations(window, title_bar_rect, title_bar_is_highlight, resize_grip_count, resize_grip_col,
                                    resize_grip_draw_size);

            if (render_decorations_in_parent)
                window->DrawList = &window->DrawListInst;
        }

        // Draw navigation selection/windowing rectangle border
        if (g.NavWindowingTargetAnim == window)
        {
            float rounding = ImMax(window->WindowRounding, g.Style.WindowRounding);
            ImRect bb = window->Rect();
            bb.Expand(g.FontSize);
            if (bb.Contains(viewport_rect)) // If a window fits the entire viewport, adjust its highlight inward
            {
                bb.Expand(-g.FontSize - 1.0f);
                rounding = window->WindowRounding;
            }
            window->DrawList->AddRect(bb.Min, bb.Max,
                                      GetColorU32(ImGuiCol_NavWindowingHighlight, g.NavWindowingHighlightAlpha),
                                      rounding, 0, 3.0f);
        }

        // UPDATE RECTANGLES (2- THOSE AFFECTED BY SCROLLING)

        // Work rectangle.
        // Affected by window padding and border size. Used by:
        // - Columns() for right-most edge
        // - TreeNode(), CollapsingHeader() for right-most edge
        // - BeginTabBar() for right-most edge
        const bool allow_scrollbar_x =
            !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar);
        const bool allow_scrollbar_y = !(flags & ImGuiWindowFlags_NoScrollbar);
        const float work_rect_size_x =
            (window->ContentSizeExplicit.x != 0.0f
                 ? window->ContentSizeExplicit.x
                 : ImMax(allow_scrollbar_x ? window->ContentSize.x : 0.0f,
                         window->Size.x - window->WindowPadding.x * 2.0f - window->ScrollbarSizes.x));
        const float work_rect_size_y =
            (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y
                                                   : ImMax(allow_scrollbar_y ? window->ContentSize.y : 0.0f,
                                                           window->Size.y - window->WindowPadding.y * 2.0f -
                                                               decoration_up_height - window->ScrollbarSizes.y));
        window->WorkRect.Min.x = ImFloor(window->InnerRect.Min.x - window->Scroll.x +
                                         ImMax(window->WindowPadding.x, window->WindowBorderSize));
        window->WorkRect.Min.y = ImFloor(window->InnerRect.Min.y - window->Scroll.y +
                                         ImMax(window->WindowPadding.y, window->WindowBorderSize));
        window->WorkRect.Max.x = window->WorkRect.Min.x + work_rect_size_x;
        window->WorkRect.Max.y = window->WorkRect.Min.y + work_rect_size_y;
        window->ParentWorkRect = window->WorkRect;

        // [LEGACY] Content Region
        // FIXME-OBSOLETE: window->ContentRegionRect.Max is currently very misleading / partly faulty, but some
        // BeginChild() patterns relies on it. Used by:
        // - Mouse wheel scrolling + many other things
        window->ContentRegionRect.Min.x = window->Pos.x - window->Scroll.x + window->WindowPadding.x;
        window->ContentRegionRect.Min.y =
            window->Pos.y - window->Scroll.y + window->WindowPadding.y + decoration_up_height;
        window->ContentRegionRect.Max.x =
            window->ContentRegionRect.Min.x +
            (window->ContentSizeExplicit.x != 0.0f
                 ? window->ContentSizeExplicit.x
                 : (window->Size.x - window->WindowPadding.x * 2.0f - window->ScrollbarSizes.x));
        window->ContentRegionRect.Max.y =
            window->ContentRegionRect.Min.y +
            (window->ContentSizeExplicit.y != 0.0f
                 ? window->ContentSizeExplicit.y
                 : (window->Size.y - window->WindowPadding.y * 2.0f - decoration_up_height - window->ScrollbarSizes.y));

        // Setup drawing context
        // (NB: That term "drawing context / DC" lost its meaning a long time ago. Initially was meant to hold transient
        // data only. Nowadays difference between window-> and window->DC-> is dubious.)
        window->DC.Indent.x = 0.0f + window->WindowPadding.x - window->Scroll.x;
        window->DC.GroupOffset.x = 0.0f;
        window->DC.ColumnsOffset.x = 0.0f;
        window->DC.CursorStartPos =
            window->Pos + ImVec2(window->DC.Indent.x + window->DC.ColumnsOffset.x,
                                 decoration_up_height + window->WindowPadding.y - window->Scroll.y);
        window->DC.CursorPos = window->DC.CursorStartPos;
        window->DC.CursorPosPrevLine = window->DC.CursorPos;
        window->DC.CursorMaxPos = window->DC.CursorStartPos;
        window->DC.IdealMaxPos = window->DC.CursorStartPos;
        window->DC.CurrLineSize = window->DC.PrevLineSize = ImVec2(0.0f, 0.0f);
        window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;

        window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        window->DC.NavLayersActiveMask = window->DC.NavLayersActiveMaskNext;
        window->DC.NavLayersActiveMaskNext = 0x00;
        window->DC.NavHideHighlightOneFrame = false;
        window->DC.NavHasScroll = (window->ScrollMax.y > 0.0f);

        window->DC.MenuBarAppending = false;
        window->DC.MenuColumns.Update(3, style.ItemSpacing.x, window_just_activated_by_user);
        window->DC.TreeDepth = 0;
        window->DC.TreeJumpToParentOnPopMask = 0x00;
        window->DC.ChildWindows.resize(0);
        window->DC.StateStorage = &window->StateStorage;
        window->DC.CurrentColumns = NULL;
        window->DC.LayoutType = ImGuiLayoutType_Vertical;
        window->DC.ParentLayoutType = parent_window ? parent_window->DC.LayoutType : ImGuiLayoutType_Vertical;
        window->DC.FocusCounterRegular = window->DC.FocusCounterTabStop = -1;

        window->DC.ItemWidth = window->ItemWidthDefault;
        window->DC.TextWrapPos = -1.0f; // disabled
        window->DC.ItemWidthStack.resize(0);
        window->DC.TextWrapPosStack.resize(0);

        if (window->AutoFitFramesX > 0)
            window->AutoFitFramesX--;
        if (window->AutoFitFramesY > 0)
            window->AutoFitFramesY--;

        // Apply focus (we need to call FocusWindow() AFTER setting DC.CursorStartPos so our initial navigation
        // reference rectangle can start around there)
        if (want_focus)
        {
            FocusWindow(window);
            NavInitWindow(
                window,
                false); // <-- this is in the way for us to be able to defer and sort reappearing FocusWindow() calls
        }

        // Title bar
        if (!(flags & ImGuiWindowFlags_NoTitleBar))
            RenderWindowTitleBarContents(window,
                                         ImRect(title_bar_rect.Min.x + window->WindowBorderSize, title_bar_rect.Min.y,
                                                title_bar_rect.Max.x - window->WindowBorderSize, title_bar_rect.Max.y),
                                         name, p_open);

        // Clear hit test shape every frame
        window->HitTestHoleSize.x = window->HitTestHoleSize.y = 0;

        // Pressing CTRL+C while holding on a window copy its content to the clipboard
        // This works but 1. doesn't handle multiple Begin/End pairs, 2. recursing into another Begin/End pair - so we
        // need to work that out and add better logging scope. Maybe we can support CTRL+C on every element?
        /*
        //if (g.NavWindow == window && g.ActiveId == 0)
        if (g.ActiveId == window->MoveId)
            if (g.IO.KeyCtrl && IsKeyPressedMap(ImGuiKey_C))
                LogToClipboard();
        */

        // We fill last item data based on Title Bar/Tab, in order for IsItemHovered() and IsItemActive() to be usable
        // after Begin(). This is useful to allow creating context menus on title bar only, etc.
        SetLastItemData(
            window, window->MoveId,
            IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max, false) ? ImGuiItemStatusFlags_HoveredRect : 0,
            title_bar_rect);

#ifdef IMGUI_ENABLE_TEST_ENGINE
        if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
            IMGUI_TEST_ENGINE_ITEM_ADD(window->DC.LastItemRect, window->DC.LastItemId);
#endif
    }
    else
    {
        // Append
        SetCurrentWindow(window);
    }

    // Pull/inherit current state
    g.CurrentItemFlags = g.ItemFlagsStack.back(); // Inherit from shared stack
    window->DC.NavFocusScopeIdCurrent = (flags & ImGuiWindowFlags_ChildWindow)
                                            ? parent_window->DC.NavFocusScopeIdCurrent
                                            : 0; // Inherit from parent only // -V595

    PushClipRect(window->InnerClipRect.Min, window->InnerClipRect.Max, true);

    // Clear 'accessed' flag last thing (After PushClipRect which will set the flag. We want the flag to stay false when
    // the default "Debug" window is unused)
    window->WriteAccessed = false;
    window->BeginCount++;
    g.NextWindowData.ClearFlags();

    // Update visibility
    if (first_begin_of_the_frame)
    {
        if (flags & ImGuiWindowFlags_ChildWindow)
        {
            // Child window can be out of sight and have "negative" clip windows.
            // Mark them as collapsed so commands are skipped earlier (we can't manually collapse them because they have
            // no title bar).
            IM_ASSERT((flags & ImGuiWindowFlags_NoTitleBar) != 0);
            if (!(flags & ImGuiWindowFlags_AlwaysAutoResize) && window->AutoFitFramesX <= 0 &&
                window->AutoFitFramesY <= 0) // FIXME: Doesn't make sense for ChildWindow??
                if (!g.LogEnabled)
                    if (window->OuterRectClipped.Min.x >= window->OuterRectClipped.Max.x ||
                        window->OuterRectClipped.Min.y >= window->OuterRectClipped.Max.y)
                        window->HiddenFramesCanSkipItems = 1;

            // Hide along with parent or if parent is collapsed
            if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCanSkipItems > 0))
                window->HiddenFramesCanSkipItems = 1;
            if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCannotSkipItems > 0))
                window->HiddenFramesCannotSkipItems = 1;
        }

        // Don't render if style alpha is 0.0 at the time of Begin(). This is arbitrary and inconsistent but has been
        // there for a long while (may remove at some point)
        if (style.Alpha <= 0.0f)
            window->HiddenFramesCanSkipItems = 1;

        // Update the Hidden flag
        window->Hidden = (window->HiddenFramesCanSkipItems > 0) || (window->HiddenFramesCannotSkipItems > 0) ||
                         (window->HiddenFramesForRenderOnly > 0);

        // Disable inputs for requested number of frames
        if (window->DisableInputsFrames > 0)
        {
            window->DisableInputsFrames--;
            window->Flags |= ImGuiWindowFlags_NoInputs;
        }

        // Update the SkipItems flag, used to early out of all items functions (no layout required)
        bool skip_items = false;
        if (window->Collapsed || !window->Active || window->Hidden)
            if (window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && window->HiddenFramesCannotSkipItems <= 0)
                skip_items = true;
        window->SkipItems = skip_items;
    }

    return !window->SkipItems;
}

void ImGui::End()
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;

    // Error checking: verify that user hasn't called End() too many times!
    if (g.CurrentWindowStack.Size <= 1 && g.WithinFrameScopeWithImplicitWindow)
    {
        IM_ASSERT_USER_ERROR(g.CurrentWindowStack.Size > 1, "Calling End() too many times!");
        return;
    }
    IM_ASSERT(g.CurrentWindowStack.Size > 0);

    // Error checking: verify that user doesn't directly call End() on a child window.
    if (window->Flags & ImGuiWindowFlags_ChildWindow)
        IM_ASSERT_USER_ERROR(g.WithinEndChild, "Must call EndChild() and not End()!");

    // Close anything that is open
    if (window->DC.CurrentColumns)
        EndColumns();
    PopClipRect(); // Inner window clip rectangle

    // Stop logging
    if (!(window->Flags & ImGuiWindowFlags_ChildWindow)) // FIXME: add more options for scope of logging
        LogFinish();

    // Pop from window stack
    g.CurrentWindowStack.pop_back();
    if (window->Flags & ImGuiWindowFlags_Popup)
        g.BeginPopupStack.pop_back();
    window->DC.StackSizesOnBegin.CompareWithCurrentState();
    SetCurrentWindow(g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back());
}

bool ImGui::BeginChild(const char *str_id, const ImVec2 &size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    ImGuiWindow *window = GetCurrentWindow();
    return BeginChildEx(str_id, window->GetID(str_id), size_arg, border, extra_flags);
}

bool ImGui::BeginChild(ImGuiID id, const ImVec2 &size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    IM_ASSERT(id != 0);
    return BeginChildEx(NULL, id, size_arg, border, extra_flags);
}

void ImGui::EndChild()
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;

    IM_ASSERT(g.WithinEndChild == false);
    IM_ASSERT(window->Flags & ImGuiWindowFlags_ChildWindow); // Mismatched BeginChild()/EndChild() calls

    g.WithinEndChild = true;
    if (window->BeginCount > 1)
    {
        End();
    }
    else
    {
        ImVec2 sz = window->Size;
        if (window->AutoFitChildAxises &
            (1 << ImGuiAxis_X)) // Arbitrary minimum zero-ish child size of 4.0f causes less trouble than a 0.0f
            sz.x = ImMax(4.0f, sz.x);
        if (window->AutoFitChildAxises & (1 << ImGuiAxis_Y))
            sz.y = ImMax(4.0f, sz.y);
        End();

        ImGuiWindow *parent_window = g.CurrentWindow;
        ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
        ItemSize(sz);
        if ((window->DC.NavLayersActiveMask != 0 || window->DC.NavHasScroll) &&
            !(window->Flags & ImGuiWindowFlags_NavFlattened))
        {
            ItemAdd(bb, window->ChildId);
            RenderNavHighlight(bb, window->ChildId);

            // When browsing a window that has no activable items (scroll only) we keep a highlight on the child
            if (window->DC.NavLayersActiveMask == 0 && window == g.NavWindow)
                RenderNavHighlight(ImRect(bb.Min - ImVec2(2, 2), bb.Max + ImVec2(2, 2)), g.NavId,
                                   ImGuiNavHighlightFlags_TypeThin);
        }
        else
        {
            // Not navigable into
            ItemAdd(bb, 0);
        }
        if (g.HoveredWindow == window)
            parent_window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
    }
    g.WithinEndChild = false;
    g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
}

// //----------------------------------------------------

// //----------------------------------------------------

// // Debug options
// #define IMGUI_DEBUG_NAV_SCORING                                                                                        \
//     0 // Display navigation scoring preview when hovering items. Display last moving direction matches when holding CTRL
// #define IMGUI_DEBUG_NAV_RECTS 0 // Display the reference navigation rectangle for each window
// #define IMGUI_DEBUG_INI_SETTINGS                                                                                       \
//     0 // Save additional comments in .ini file (particularly helps for Docking, but makes saving slower)

// // When using CTRL+TAB (or Gamepad Square+L/R) we delay the visual a little in order to reduce visual noise doing a fast
// // switch.
// static const float NAV_WINDOWING_HIGHLIGHT_DELAY =
//     0.20f; // Time before the highlight and screen dimming starts fading in
// static const float NAV_WINDOWING_LIST_APPEAR_DELAY = 0.15f; // Time before the window list starts to appear

// // Window resizing from edges (when io.ConfigWindowsResizeFromEdges = true and ImGuiBackendFlags_HasMouseCursors is set
// // in io.BackendFlags by backend)
// static const float WINDOWS_HOVER_PADDING =
//     4.0f; // Extend outside window for hovering/resizing (maxxed with TouchPadding) and inside windows for borders.
//           // Affect FindHoveredWindow().
// static const float WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER =
//     0.04f; // Reduce visual noise by only highlighting the border after a certain time.
// static const float WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER =
//     2.00f; // Lock scrolled window (so it doesn't pick child windows that are scrolling through) for a certain time,
//            // unless mouse moved.

// // extern const ImGuiResizeGripDef resize_grip_def[4];

// static const ImGuiResizeGripDef resize_grip_def[4] = {
//     {ImVec2(1, 1), ImVec2(-1, -1), 0, 3}, // Lower-right
//     {ImVec2(0, 1), ImVec2(+1, -1), 3, 6}, // Lower-left
//     {ImVec2(0, 0), ImVec2(+1, +1), 6, 9}, // Upper-left (Unused)
//     {ImVec2(1, 0), ImVec2(-1, +1), 9, 12} // Upper-right (Unused)
// };

// // extern const ImGuiResizeBorderDef resize_border_def[4];
// static const ImGuiResizeBorderDef resize_border_def[4] = {
//     {ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f}, // Left
//     {ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f}, // Right
//     {ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f}, // Up
//     {ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f}  // Down
// };

// static void ImGui::NavUpdate()
// {
//     ImGuiContext &g = *GImGui;
//     ImGuiIO &io = g.IO;

//     io.WantSetMousePos = false;
//     g.NavWrapRequestWindow = NULL;
//     g.NavWrapRequestFlags = ImGuiNavMoveFlags_None;
// #if 0
//     if (g.NavScoringCount > 0) IMGUI_DEBUG_LOG("NavScoringCount %d for '%s' layer %d (Init:%d, Move:%d)\n", g.FrameCount, g.NavScoringCount, g.NavWindow ? g.NavWindow->Name : "NULL", g.NavLayer, g.NavInitRequest || g.NavInitResultId != 0, g.NavMoveRequest);
// #endif

//     // Set input source as Gamepad when buttons are pressed (as some features differs when used with Gamepad vs
//     // Keyboard) (do it before we map Keyboard input!)
//     bool nav_keyboard_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0;
//     bool nav_gamepad_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 &&
//                               (io.BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
//     if (nav_gamepad_active && g.NavInputSource != ImGuiInputSource_Gamepad)
//     {
//         if (io.NavInputs[ImGuiNavInput_Activate] > 0.0f || io.NavInputs[ImGuiNavInput_Input] > 0.0f ||
//             io.NavInputs[ImGuiNavInput_Cancel] > 0.0f || io.NavInputs[ImGuiNavInput_Menu] > 0.0f ||
//             io.NavInputs[ImGuiNavInput_DpadLeft] > 0.0f || io.NavInputs[ImGuiNavInput_DpadRight] > 0.0f ||
//             io.NavInputs[ImGuiNavInput_DpadUp] > 0.0f || io.NavInputs[ImGuiNavInput_DpadDown] > 0.0f)
//             g.NavInputSource = ImGuiInputSource_Gamepad;
//     }

//     // Update Keyboard->Nav inputs mapping
//     if (nav_keyboard_active)
//     {
// #define NAV_MAP_KEY(_KEY, _NAV_INPUT)                                                                                  \
//     do                                                                                                                 \
//     {                                                                                                                  \
//         if (IsKeyDown(io.KeyMap[_KEY]))                                                                                \
//         {                                                                                                              \
//             io.NavInputs[_NAV_INPUT] = 1.0f;                                                                           \
//             g.NavInputSource = ImGuiInputSource_Keyboard;                                                              \
//         }                                                                                                              \
//     } while (0)
//         NAV_MAP_KEY(ImGuiKey_Space, ImGuiNavInput_Activate);
//         NAV_MAP_KEY(ImGuiKey_Enter, ImGuiNavInput_Input);
//         NAV_MAP_KEY(ImGuiKey_Escape, ImGuiNavInput_Cancel);
//         NAV_MAP_KEY(ImGuiKey_LeftArrow, ImGuiNavInput_KeyLeft_);
//         NAV_MAP_KEY(ImGuiKey_RightArrow, ImGuiNavInput_KeyRight_);
//         NAV_MAP_KEY(ImGuiKey_UpArrow, ImGuiNavInput_KeyUp_);
//         NAV_MAP_KEY(ImGuiKey_DownArrow, ImGuiNavInput_KeyDown_);
//         if (io.KeyCtrl)
//             io.NavInputs[ImGuiNavInput_TweakSlow] = 1.0f;
//         if (io.KeyShift)
//             io.NavInputs[ImGuiNavInput_TweakFast] = 1.0f;

//         // AltGR is normally Alt+Ctrl but we can't reliably detect it (not all backends/systems/layout emit it as
//         // Alt+Ctrl) But also even on keyboards without AltGR we don't want Alt+Ctrl to open menu anyway.
//         if (io.KeyAlt && !io.KeyCtrl)
//             io.NavInputs[ImGuiNavInput_KeyMenu_] = 1.0f;

//         // We automatically cancel toggling nav layer when any text has been typed while holding Alt. (See #370)
//         if (io.KeyAlt && !io.KeyCtrl && g.NavWindowingToggleLayer && io.InputQueueCharacters.Size > 0)
//             g.NavWindowingToggleLayer = false;

// #undef NAV_MAP_KEY
//     }
//     memcpy(io.NavInputsDownDurationPrev, io.NavInputsDownDuration, sizeof(io.NavInputsDownDuration));
//     for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++)
//         io.NavInputsDownDuration[i] =
//             (io.NavInputs[i] > 0.0f)
//                 ? (io.NavInputsDownDuration[i] < 0.0f ? 0.0f : io.NavInputsDownDuration[i] + io.DeltaTime)
//                 : -1.0f;

//     // Process navigation init request (select first/default focus)
//     if (g.NavInitResultId != 0)
//         NavUpdateInitResult();
//     g.NavInitRequest = false;
//     g.NavInitRequestFromMove = false;
//     g.NavInitResultId = 0;
//     g.NavJustMovedToId = 0;

//     // Process navigation move request
//     if (g.NavMoveRequest)
//         NavUpdateMoveResult();

//     // When a forwarded move request failed, we restore the highlight that we disabled during the forward frame
//     if (g.NavMoveRequestForward == ImGuiNavForward_ForwardActive)
//     {
//         IM_ASSERT(g.NavMoveRequest);
//         if (g.NavMoveResultLocal.ID == 0 && g.NavMoveResultOther.ID == 0)
//             g.NavDisableHighlight = false;
//         g.NavMoveRequestForward = ImGuiNavForward_None;
//     }

//     // Apply application mouse position movement, after we had a chance to process move request result.
//     if (g.NavMousePosDirty && g.NavIdIsAlive)
//     {
//         // Set mouse position given our knowledge of the navigated item position from last frame
//         if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos) &&
//             (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos))
//             if (!g.NavDisableHighlight && g.NavDisableMouseHover && g.NavWindow)
//             {
//                 io.MousePos = io.MousePosPrev = NavCalcPreferredRefPos();
//                 io.WantSetMousePos = true;
//             }
//         g.NavMousePosDirty = false;
//     }
//     g.NavIdIsAlive = false;
//     g.NavJustTabbedId = 0;
//     IM_ASSERT(g.NavLayer == 0 || g.NavLayer == 1);

//     // Store our return window (for returning from Layer 1 to Layer 0) and clear it as soon as we step back in our own
//     // Layer 0
//     if (g.NavWindow)
//         NavSaveLastChildNavWindowIntoParent(g.NavWindow);
//     if (g.NavWindow && g.NavWindow->NavLastChildNavWindow != NULL && g.NavLayer == ImGuiNavLayer_Main)
//         g.NavWindow->NavLastChildNavWindow = NULL;

//     // Update CTRL+TAB and Windowing features (hold Square to move/resize/etc.)
//     NavUpdateWindowing();

//     // Set output flags for user application
//     io.NavActive = (nav_keyboard_active || nav_gamepad_active) && g.NavWindow &&
//                    !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs);
//     io.NavVisible = (io.NavActive && g.NavId != 0 && !g.NavDisableHighlight) || (g.NavWindowingTarget != NULL);

//     // Process NavCancel input (to close a popup, get back to parent, clear focus)
//     if (IsNavInputTest(ImGuiNavInput_Cancel, ImGuiInputReadMode_Pressed))
//     {
//         IMGUI_DEBUG_LOG_NAV("[nav] ImGuiNavInput_Cancel\n");
//         if (g.ActiveId != 0)
//         {
//             if (!IsActiveIdUsingNavInput(ImGuiNavInput_Cancel))
//                 ClearActiveID();
//         }
//         else if (g.NavLayer != ImGuiNavLayer_Main)
//         {
//             // Leave the "menu" layer
//             NavRestoreLayer(ImGuiNavLayer_Main);
//         }
//         else if (g.NavWindow && g.NavWindow != g.NavWindow->RootWindow &&
//                  !(g.NavWindow->Flags & ImGuiWindowFlags_Popup) && g.NavWindow->ParentWindow)
//         {
//             // Exit child window
//             ImGuiWindow *child_window = g.NavWindow;
//             ImGuiWindow *parent_window = g.NavWindow->ParentWindow;
//             IM_ASSERT(child_window->ChildId != 0);
//             ImRect child_rect = child_window->Rect();
//             FocusWindow(parent_window);
//             SetNavID(child_window->ChildId, ImGuiNavLayer_Main, 0,
//                      ImRect(child_rect.Min - parent_window->Pos, child_rect.Max - parent_window->Pos));
//         }
//         else if (g.OpenPopupStack.Size > 0)
//         {
//             // Close open popup/menu
//             if (!(g.OpenPopupStack.back().Window->Flags & ImGuiWindowFlags_Modal))
//                 ClosePopupToLevel(g.OpenPopupStack.Size - 1, true);
//         }
//         else
//         {
//             // Clear NavLastId for popups but keep it for regular child window so we can leave one and come back where
//             // we were
//             if (g.NavWindow &&
//                 ((g.NavWindow->Flags & ImGuiWindowFlags_Popup) || !(g.NavWindow->Flags & ImGuiWindowFlags_ChildWindow)))
//                 g.NavWindow->NavLastIds[0] = 0;
//             g.NavId = g.NavFocusScopeId = 0;
//         }
//     }

//     // Process manual activation request
//     g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId = g.NavInputId = 0;
//     if (g.NavId != 0 && !g.NavDisableHighlight && !g.NavWindowingTarget && g.NavWindow &&
//         !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
//     {
//         bool activate_down = IsNavInputDown(ImGuiNavInput_Activate);
//         bool activate_pressed = activate_down && IsNavInputTest(ImGuiNavInput_Activate, ImGuiInputReadMode_Pressed);
//         if (g.ActiveId == 0 && activate_pressed)
//             g.NavActivateId = g.NavId;
//         if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && activate_down)
//             g.NavActivateDownId = g.NavId;
//         if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && activate_pressed)
//             g.NavActivatePressedId = g.NavId;
//         if ((g.ActiveId == 0 || g.ActiveId == g.NavId) &&
//             IsNavInputTest(ImGuiNavInput_Input, ImGuiInputReadMode_Pressed))
//             g.NavInputId = g.NavId;
//     }
//     if (g.NavWindow && (g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
//         g.NavDisableHighlight = true;
//     if (g.NavActivateId != 0)
//         IM_ASSERT(g.NavActivateDownId == g.NavActivateId);
//     g.NavMoveRequest = false;

//     // Process programmatic activation request
//     if (g.NavNextActivateId != 0)
//         g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId = g.NavInputId = g.NavNextActivateId;
//     g.NavNextActivateId = 0;

//     // Initiate directional inputs request
//     if (g.NavMoveRequestForward == ImGuiNavForward_None)
//     {
//         g.NavMoveDir = ImGuiDir_None;
//         g.NavMoveRequestFlags = ImGuiNavMoveFlags_None;
//         if (g.NavWindow && !g.NavWindowingTarget && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
//         {
//             const ImGuiInputReadMode read_mode = ImGuiInputReadMode_Repeat;
//             if (!IsActiveIdUsingNavDir(ImGuiDir_Left) && (IsNavInputTest(ImGuiNavInput_DpadLeft, read_mode) ||
//                                                           IsNavInputTest(ImGuiNavInput_KeyLeft_, read_mode)))
//             {
//                 g.NavMoveDir = ImGuiDir_Left;
//             }
//             if (!IsActiveIdUsingNavDir(ImGuiDir_Right) && (IsNavInputTest(ImGuiNavInput_DpadRight, read_mode) ||
//                                                            IsNavInputTest(ImGuiNavInput_KeyRight_, read_mode)))
//             {
//                 g.NavMoveDir = ImGuiDir_Right;
//             }
//             if (!IsActiveIdUsingNavDir(ImGuiDir_Up) &&
//                 (IsNavInputTest(ImGuiNavInput_DpadUp, read_mode) || IsNavInputTest(ImGuiNavInput_KeyUp_, read_mode)))
//             {
//                 g.NavMoveDir = ImGuiDir_Up;
//             }
//             if (!IsActiveIdUsingNavDir(ImGuiDir_Down) && (IsNavInputTest(ImGuiNavInput_DpadDown, read_mode) ||
//                                                           IsNavInputTest(ImGuiNavInput_KeyDown_, read_mode)))
//             {
//                 g.NavMoveDir = ImGuiDir_Down;
//             }
//         }
//         g.NavMoveClipDir = g.NavMoveDir;
//     }
//     else
//     {
//         // Forwarding previous request (which has been modified, e.g. wrap around menus rewrite the requests with a
//         // starting rectangle at the other side of the window) (Preserve g.NavMoveRequestFlags, g.NavMoveClipDir which
//         // were set by the NavMoveRequestForward() function)
//         IM_ASSERT(g.NavMoveDir != ImGuiDir_None && g.NavMoveClipDir != ImGuiDir_None);
//         IM_ASSERT(g.NavMoveRequestForward == ImGuiNavForward_ForwardQueued);
//         IMGUI_DEBUG_LOG_NAV("[nav] NavMoveRequestForward %d\n", g.NavMoveDir);
//         g.NavMoveRequestForward = ImGuiNavForward_ForwardActive;
//     }

//     // Update PageUp/PageDown/Home/End scroll
//     // FIXME-NAV: Consider enabling those keys even without the master ImGuiConfigFlags_NavEnableKeyboard flag?
//     float nav_scoring_rect_offset_y = 0.0f;
//     if (nav_keyboard_active)
//         nav_scoring_rect_offset_y = NavUpdatePageUpPageDown();

//     // If we initiate a movement request and have no current NavId, we initiate a InitDefautRequest that will be used as
//     // a fallback if the direction fails to find a match
//     if (g.NavMoveDir != ImGuiDir_None)
//     {
//         g.NavMoveRequest = true;
//         g.NavMoveRequestKeyMods = io.KeyMods;
//         g.NavMoveDirLast = g.NavMoveDir;
//     }
//     if (g.NavMoveRequest && g.NavId == 0)
//     {
//         IMGUI_DEBUG_LOG_NAV("[nav] NavInitRequest: from move, window \"%s\", layer=%d\n", g.NavWindow->Name,
//                             g.NavLayer);
//         g.NavInitRequest = g.NavInitRequestFromMove = true;
//         // Reassigning with same value, we're being explicit here.
//         g.NavInitResultId = 0; // -V1048
//         g.NavDisableHighlight = false;
//     }
//     NavUpdateAnyRequestFlag();

//     // Scrolling
//     if (g.NavWindow && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs) && !g.NavWindowingTarget)
//     {
//         // *Fallback* manual-scroll with Nav directional keys when window has no navigable item
//         ImGuiWindow *window = g.NavWindow;
//         const float scroll_speed = IM_ROUND(
//             window->CalcFontSize() * 100 *
//             io.DeltaTime); // We need round the scrolling speed because sub-pixel scroll isn't reliably supported.
//         if (window->DC.NavLayersActiveMask == 0x00 && window->DC.NavHasScroll && g.NavMoveRequest)
//         {
//             if (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right)
//                 SetScrollX(window, ImFloor(window->Scroll.x +
//                                            ((g.NavMoveDir == ImGuiDir_Left) ? -1.0f : +1.0f) * scroll_speed));
//             if (g.NavMoveDir == ImGuiDir_Up || g.NavMoveDir == ImGuiDir_Down)
//                 SetScrollY(window,
//                            ImFloor(window->Scroll.y + ((g.NavMoveDir == ImGuiDir_Up) ? -1.0f : +1.0f) * scroll_speed));
//         }

//         // *Normal* Manual scroll with NavScrollXXX keys
//         // Next movement request will clamp the NavId reference rectangle to the visible area, so navigation will resume
//         // within those bounds.
//         ImVec2 scroll_dir =
//             GetNavInputAmount2d(ImGuiNavDirSourceFlags_PadLStick, ImGuiInputReadMode_Down, 1.0f / 10.0f, 10.0f);
//         if (scroll_dir.x != 0.0f && window->ScrollbarX)
//             SetScrollX(window, ImFloor(window->Scroll.x + scroll_dir.x * scroll_speed));
//         if (scroll_dir.y != 0.0f)
//             SetScrollY(window, ImFloor(window->Scroll.y + scroll_dir.y * scroll_speed));
//     }

//     // Reset search results
//     g.NavMoveResultLocal.Clear();
//     g.NavMoveResultLocalVisibleSet.Clear();
//     g.NavMoveResultOther.Clear();

//     // When using gamepad, we project the reference nav bounding box into window visible area.
//     // This is to allow resuming navigation inside the visible area after doing a large amount of scrolling, since with
//     // gamepad every movements are relative (can't focus a visible object like we can with the mouse).
//     if (g.NavMoveRequest && g.NavInputSource == ImGuiInputSource_Gamepad && g.NavLayer == ImGuiNavLayer_Main)
//     {
//         ImGuiWindow *window = g.NavWindow;
//         ImRect window_rect_rel(window->InnerRect.Min - window->Pos - ImVec2(1, 1),
//                                window->InnerRect.Max - window->Pos + ImVec2(1, 1));
//         if (!window_rect_rel.Contains(window->NavRectRel[g.NavLayer]))
//         {
//             IMGUI_DEBUG_LOG_NAV("[nav] NavMoveRequest: clamp NavRectRel\n");
//             float pad = window->CalcFontSize() * 0.5f;
//             window_rect_rel.Expand(
//                 ImVec2(-ImMin(window_rect_rel.GetWidth(), pad),
//                        -ImMin(window_rect_rel.GetHeight(), pad))); // Terrible approximation for the intent of starting
//                                                                    // navigation from first fully visible item
//             window->NavRectRel[g.NavLayer].ClipWithFull(window_rect_rel);
//             g.NavId = g.NavFocusScopeId = 0;
//         }
//     }

//     // For scoring we use a single segment on the left side our current item bounding box (not touching the edge to
//     // avoid box overlap with zero-spaced items)
//     ImRect nav_rect_rel = g.NavWindow && !g.NavWindow->NavRectRel[g.NavLayer].IsInverted()
//                               ? g.NavWindow->NavRectRel[g.NavLayer]
//                               : ImRect(0, 0, 0, 0);
//     g.NavScoringRect = g.NavWindow ? ImRect(g.NavWindow->Pos + nav_rect_rel.Min, g.NavWindow->Pos + nav_rect_rel.Max)
//                                    : ImRect(0, 0, 0, 0);
//     g.NavScoringRect.TranslateY(nav_scoring_rect_offset_y);
//     g.NavScoringRect.Min.x = ImMin(g.NavScoringRect.Min.x + 1.0f, g.NavScoringRect.Max.x);
//     g.NavScoringRect.Max.x = g.NavScoringRect.Min.x;
//     IM_ASSERT(!g.NavScoringRect.IsInverted()); // Ensure if we have a finite, non-inverted bounding box here will allows
//                                                // us to remove extraneous ImFabs() calls in NavScoreItem().
//     // GetForegroundDrawList()->AddRect(g.NavScoringRectScreen.Min, g.NavScoringRectScreen.Max,
//     // IM_COL32(255,200,0,255)); // [DEBUG]
//     g.NavScoringCount = 0;
// #if IMGUI_DEBUG_NAV_RECTS
//     if (g.NavWindow)
//     {
//         ImDrawList *draw_list = GetForegroundDrawList(g.NavWindow);
//         if (1)
//         {
//             for (int layer = 0; layer < 2; layer++)
//                 draw_list->AddRect(g.NavWindow->Pos + g.NavWindow->NavRectRel[layer].Min,
//                                    g.NavWindow->Pos + g.NavWindow->NavRectRel[layer].Max, IM_COL32(255, 200, 0, 255));
//         } // [DEBUG]
//         if (1)
//         {
//             ImU32 col = (!g.NavWindow->Hidden) ? IM_COL32(255, 0, 255, 255) : IM_COL32(255, 0, 0, 255);
//             ImVec2 p = NavCalcPreferredRefPos();
//             char buf[32];
//             ImFormatString(buf, 32, "%d", g.NavLayer);
//             draw_list->AddCircleFilled(p, 3.0f, col);
//             draw_list->AddText(NULL, 13.0f, p + ImVec2(8, -4), col, buf);
//         }
//     }
// #endif
// }

// static ImGuiWindow *FindWindowNavFocusable(int i_start, int i_stop, int dir) // FIXME-OPT O(N)
// {
//     ImGuiContext &g = *GImGui;
//     for (int i = i_start; i >= 0 && i < g.WindowsFocusOrder.Size && i != i_stop; i += dir)
//         if (ImGui::IsWindowNavFocusable(g.WindowsFocusOrder[i]))
//             return g.WindowsFocusOrder[i];
//     return NULL;
// }

// static void NavUpdateWindowingHighlightWindow(int focus_change_dir)
// {
//     ImGuiContext &g = *GImGui;
//     IM_ASSERT(g.NavWindowingTarget);
//     if (g.NavWindowingTarget->Flags & ImGuiWindowFlags_Modal)
//         return;

//     const int i_current = ImGui::FindWindowFocusIndex(g.NavWindowingTarget);
//     ImGuiWindow *window_target = FindWindowNavFocusable(i_current + focus_change_dir, -INT_MAX, focus_change_dir);
//     if (!window_target)
//         window_target = FindWindowNavFocusable((focus_change_dir < 0) ? (g.WindowsFocusOrder.Size - 1) : 0, i_current,
//                                                focus_change_dir);
//     if (window_target) // Don't reset windowing target if there's a single window in the list
//         g.NavWindowingTarget = g.NavWindowingTargetAnim = window_target;
//     g.NavWindowingToggleLayer = false;
// }

// // Windowing management mode
// // Keyboard: CTRL+Tab (change focus/move/resize), Alt (toggle menu layer)
// // Gamepad:  Hold Menu/Square (change focus/move/resize), Tap Menu/Square (toggle menu layer)
// static void ImGui::NavUpdateWindowing()
// {
//     ImGuiContext &g = *GImGui;
//     ImGuiWindow *apply_focus_window = NULL;
//     bool apply_toggle_layer = false;

//     ImGuiWindow *modal_window = GetTopMostPopupModal();
//     bool allow_windowing = (modal_window == NULL);
//     if (!allow_windowing)
//         g.NavWindowingTarget = NULL;

//     // Fade out
//     if (g.NavWindowingTargetAnim && g.NavWindowingTarget == NULL)
//     {
//         g.NavWindowingHighlightAlpha = ImMax(g.NavWindowingHighlightAlpha - g.IO.DeltaTime * 10.0f, 0.0f);
//         if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
//             g.NavWindowingTargetAnim = NULL;
//     }

//     // Start CTRL-TAB or Square+L/R window selection
//     bool start_windowing_with_gamepad =
//         allow_windowing && !g.NavWindowingTarget && IsNavInputTest(ImGuiNavInput_Menu, ImGuiInputReadMode_Pressed);
//     bool start_windowing_with_keyboard = allow_windowing && !g.NavWindowingTarget && g.IO.KeyCtrl &&
//                                          IsKeyPressedMap(ImGuiKey_Tab) &&
//                                          (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard);
//     if (start_windowing_with_gamepad || start_windowing_with_keyboard)
//         if (ImGuiWindow *window =
//                 g.NavWindow ? g.NavWindow : FindWindowNavFocusable(g.WindowsFocusOrder.Size - 1, -INT_MAX, -1))
//         {
//             g.NavWindowingTarget = g.NavWindowingTargetAnim =
//                 window->RootWindow; // FIXME-DOCK: Will need to use RootWindowDockStop
//             g.NavWindowingTimer = g.NavWindowingHighlightAlpha = 0.0f;
//             g.NavWindowingToggleLayer = start_windowing_with_keyboard ? false : true;
//             g.NavInputSource = start_windowing_with_keyboard ? ImGuiInputSource_Keyboard : ImGuiInputSource_Gamepad;
//         }

//     // Gamepad update
//     g.NavWindowingTimer += g.IO.DeltaTime;
//     if (g.NavWindowingTarget && g.NavInputSource == ImGuiInputSource_Gamepad)
//     {
//         // Highlight only appears after a brief time holding the button, so that a fast tap on PadMenu (to toggle
//         // NavLayer) doesn't add visual noise
//         g.NavWindowingHighlightAlpha = ImMax(g.NavWindowingHighlightAlpha,
//                                              ImSaturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) / 0.05f));

//         // Select window to focus
//         const int focus_change_dir = (int)IsNavInputTest(ImGuiNavInput_FocusPrev, ImGuiInputReadMode_RepeatSlow) -
//                                      (int)IsNavInputTest(ImGuiNavInput_FocusNext, ImGuiInputReadMode_RepeatSlow);
//         if (focus_change_dir != 0)
//         {
//             NavUpdateWindowingHighlightWindow(focus_change_dir);
//             g.NavWindowingHighlightAlpha = 1.0f;
//         }

//         // Single press toggles NavLayer, long press with L/R apply actual focus on release (until then the window was
//         // merely rendered top-most)
//         if (!IsNavInputDown(ImGuiNavInput_Menu))
//         {
//             g.NavWindowingToggleLayer &=
//                 (g.NavWindowingHighlightAlpha <
//                  1.0f); // Once button was held long enough we don't consider it a tap-to-toggle-layer press anymore.
//             if (g.NavWindowingToggleLayer && g.NavWindow)
//                 apply_toggle_layer = true;
//             else if (!g.NavWindowingToggleLayer)
//                 apply_focus_window = g.NavWindowingTarget;
//             g.NavWindowingTarget = NULL;
//         }
//     }

//     // Keyboard: Focus
//     if (g.NavWindowingTarget && g.NavInputSource == ImGuiInputSource_Keyboard)
//     {
//         // Visuals only appears after a brief time after pressing TAB the first time, so that a fast CTRL+TAB doesn't
//         // add visual noise
//         g.NavWindowingHighlightAlpha =
//             ImMax(g.NavWindowingHighlightAlpha,
//                   ImSaturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) / 0.05f)); // 1.0f
//         if (IsKeyPressedMap(ImGuiKey_Tab, true))
//             NavUpdateWindowingHighlightWindow(g.IO.KeyShift ? +1 : -1);
//         if (!g.IO.KeyCtrl)
//             apply_focus_window = g.NavWindowingTarget;
//     }

//     // Keyboard: Press and Release ALT to toggle menu layer
//     // FIXME: We lack an explicit IO variable for "is the imgui window focused", so compare mouse validity to detect the
//     // common case of backend clearing releases all keys on ALT-TAB
//     if (IsNavInputTest(ImGuiNavInput_KeyMenu_, ImGuiInputReadMode_Pressed))
//         g.NavWindowingToggleLayer = true;
//     if ((g.ActiveId == 0 || g.ActiveIdAllowOverlap) && g.NavWindowingToggleLayer &&
//         IsNavInputTest(ImGuiNavInput_KeyMenu_, ImGuiInputReadMode_Released))
//         if (IsMousePosValid(&g.IO.MousePos) == IsMousePosValid(&g.IO.MousePosPrev))
//             apply_toggle_layer = true;

//     // Move window
//     if (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoMove))
//     {
//         ImVec2 move_delta;
//         if (g.NavInputSource == ImGuiInputSource_Keyboard && !g.IO.KeyShift)
//             move_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard, ImGuiInputReadMode_Down);
//         if (g.NavInputSource == ImGuiInputSource_Gamepad)
//             move_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_PadLStick, ImGuiInputReadMode_Down);
//         if (move_delta.x != 0.0f || move_delta.y != 0.0f)
//         {
//             const float NAV_MOVE_SPEED = 800.0f;
//             const float move_speed =
//                 ImFloor(NAV_MOVE_SPEED * g.IO.DeltaTime *
//                         ImMin(g.IO.DisplayFramebufferScale.x,
//                               g.IO.DisplayFramebufferScale.y)); // FIXME: Doesn't handle variable framerate very well
//             ImGuiWindow *moving_window = g.NavWindowingTarget->RootWindow;
//             SetWindowPos(moving_window, moving_window->Pos + move_delta * move_speed, ImGuiCond_Always);
//             MarkIniSettingsDirty(moving_window);
//             g.NavDisableMouseHover = true;
//         }
//     }

//     // Apply final focus
//     if (apply_focus_window && (g.NavWindow == NULL || apply_focus_window != g.NavWindow->RootWindow))
//     {
//         ClearActiveID();
//         g.NavDisableHighlight = false;
//         g.NavDisableMouseHover = true;
//         apply_focus_window = NavRestoreLastChildNavWindow(apply_focus_window);
//         ClosePopupsOverWindow(apply_focus_window, false);
//         FocusWindow(apply_focus_window);
//         if (apply_focus_window->NavLastIds[0] == 0)
//             NavInitWindow(apply_focus_window, false);

//         // If the window has ONLY a menu layer (no main layer), select it directly
//         // Use NavLayersActiveMaskNext since windows didn't have a chance to be Begin()-ed on this frame,
//         // so CTRL+Tab where the keys are only held for 1 frame will be able to use correct layers mask since
//         // the target window as already been previewed once.
//         // FIXME-NAV: This should be done in NavInit.. or in FocusWindow... However in both of those cases,
//         // we won't have a guarantee that windows has been visible before and therefore NavLayersActiveMask*
//         // won't be valid.
//         if (apply_focus_window->DC.NavLayersActiveMaskNext == (1 << ImGuiNavLayer_Menu))
//             g.NavLayer = ImGuiNavLayer_Menu;
//     }
//     if (apply_focus_window)
//         g.NavWindowingTarget = NULL;

//     // Apply menu/layer toggle
//     if (apply_toggle_layer && g.NavWindow)
//     {
//         ClearActiveID();

//         // Move to parent menu if necessary
//         ImGuiWindow *new_nav_window = g.NavWindow;
//         while (new_nav_window->ParentWindow &&
//                (new_nav_window->DC.NavLayersActiveMask & (1 << ImGuiNavLayer_Menu)) == 0 &&
//                (new_nav_window->Flags & ImGuiWindowFlags_ChildWindow) != 0 &&
//                (new_nav_window->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu)) == 0)
//             new_nav_window = new_nav_window->ParentWindow;
//         if (new_nav_window != g.NavWindow)
//         {
//             ImGuiWindow *old_nav_window = g.NavWindow;
//             FocusWindow(new_nav_window);
//             new_nav_window->NavLastChildNavWindow = old_nav_window;
//         }
//         g.NavDisableHighlight = false;
//         g.NavDisableMouseHover = true;

//         // Reinitialize navigation when entering menu bar with the Alt key.
//         const ImGuiNavLayer new_nav_layer = (g.NavWindow->DC.NavLayersActiveMask & (1 << ImGuiNavLayer_Menu))
//                                                 ? (ImGuiNavLayer)((int)g.NavLayer ^ 1)
//                                                 : ImGuiNavLayer_Main;
//         if (new_nav_layer == ImGuiNavLayer_Menu)
//             g.NavWindow->NavLastIds[new_nav_layer] = 0;
//         NavRestoreLayer(new_nav_layer);
//     }
// }

// // Window has already passed the IsWindowNavFocusable()
// static const char *GetFallbackWindowNameForWindowingList(ImGuiWindow *window)
// {
//     if (window->Flags & ImGuiWindowFlags_Popup)
//         return "(Popup)";
//     if ((window->Flags & ImGuiWindowFlags_MenuBar) && strcmp(window->Name, "##MainMenuBar") == 0)
//         return "(Main menu bar)";
//     return "(Untitled)";
// }
// // Overlay displayed when using CTRL+TAB. Called by EndFrame().
// void ImGui::NavUpdateWindowingOverlay()
// {
//     ImGuiContext &g = *GImGui;
//     IM_ASSERT(g.NavWindowingTarget != NULL);

//     if (g.NavWindowingTimer < NAV_WINDOWING_LIST_APPEAR_DELAY)
//         return;

//     if (g.NavWindowingListWindow == NULL)
//         g.NavWindowingListWindow = FindWindowByName("###NavWindowingList");
//     const ImGuiViewport *viewport = GetMainViewport();
//     SetNextWindowSizeConstraints(ImVec2(viewport->Size.x * 0.20f, viewport->Size.y * 0.20f), ImVec2(FLT_MAX, FLT_MAX));
//     SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
//     PushStyleVar(ImGuiStyleVar_WindowPadding, g.Style.WindowPadding * 2.0f);
//     Begin("###NavWindowingList", NULL,
//           ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize |
//               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
//               ImGuiWindowFlags_NoSavedSettings);
//     for (int n = g.WindowsFocusOrder.Size - 1; n >= 0; n--)
//     {
//         ImGuiWindow *window = g.WindowsFocusOrder[n];
//         IM_ASSERT(window != NULL); // Fix static analyzers
//         if (!IsWindowNavFocusable(window))
//             continue;
//         const char *label = window->Name;
//         if (label == FindRenderedTextEnd(label))
//             label = GetFallbackWindowNameForWindowingList(window);
//         Selectable(label, g.NavWindowingTarget == window);
//     }
//     End();
//     PopStyleVar();
// }

// // Apply result from previous frame navigation directional move request
// static void ImGui::NavUpdateMoveResult()
// {
//     ImGuiContext &g = *GImGui;
//     if (g.NavMoveResultLocal.ID == 0 && g.NavMoveResultOther.ID == 0)
//     {
//         // In a situation when there is no results but NavId != 0, re-enable the Navigation highlight (because g.NavId
//         // is not considered as a possible result)
//         if (g.NavId != 0)
//         {
//             g.NavDisableHighlight = false;
//             g.NavDisableMouseHover = true;
//         }
//         return;
//     }

//     // Select which result to use
//     ImGuiNavItemData *result = (g.NavMoveResultLocal.ID != 0) ? &g.NavMoveResultLocal : &g.NavMoveResultOther;

//     // PageUp/PageDown behavior first jumps to the bottom/top mostly visible item, _otherwise_ use the result from the
//     // previous/next page.
//     if (g.NavMoveRequestFlags & ImGuiNavMoveFlags_AlsoScoreVisibleSet)
//         if (g.NavMoveResultLocalVisibleSet.ID != 0 && g.NavMoveResultLocalVisibleSet.ID != g.NavId)
//             result = &g.NavMoveResultLocalVisibleSet;

//     // Maybe entering a flattened child from the outside? In this case solve the tie using the regular scoring rules.
//     if (result != &g.NavMoveResultOther && g.NavMoveResultOther.ID != 0 &&
//         g.NavMoveResultOther.Window->ParentWindow == g.NavWindow)
//         if ((g.NavMoveResultOther.DistBox < result->DistBox) ||
//             (g.NavMoveResultOther.DistBox == result->DistBox && g.NavMoveResultOther.DistCenter < result->DistCenter))
//             result = &g.NavMoveResultOther;
//     IM_ASSERT(g.NavWindow && result->Window);

//     // Scroll to keep newly navigated item fully into view.
//     if (g.NavLayer == ImGuiNavLayer_Main)
//     {
//         ImVec2 delta_scroll;
//         if (g.NavMoveRequestFlags & ImGuiNavMoveFlags_ScrollToEdge)
//         {
//             float scroll_target = (g.NavMoveDir == ImGuiDir_Up) ? result->Window->ScrollMax.y : 0.0f;
//             delta_scroll.y = result->Window->Scroll.y - scroll_target;
//             SetScrollY(result->Window, scroll_target);
//         }
//         else
//         {
//             ImRect rect_abs =
//                 ImRect(result->RectRel.Min + result->Window->Pos, result->RectRel.Max + result->Window->Pos);
//             delta_scroll = ScrollToBringRectIntoView(result->Window, rect_abs);
//         }

//         // Offset our result position so mouse position can be applied immediately after in NavUpdate()
//         result->RectRel.TranslateX(-delta_scroll.x);
//         result->RectRel.TranslateY(-delta_scroll.y);
//     }

//     ClearActiveID();
//     g.NavWindow = result->Window;
//     if (g.NavId != result->ID)
//     {
//         // Don't set NavJustMovedToId if just landed on the same spot (which may happen with
//         // ImGuiNavMoveFlags_AllowCurrentNavId)
//         g.NavJustMovedToId = result->ID;
//         g.NavJustMovedToFocusScopeId = result->FocusScopeId;
//         g.NavJustMovedToKeyMods = g.NavMoveRequestKeyMods;
//     }
//     IMGUI_DEBUG_LOG_NAV("[nav] NavMoveRequest: result NavID 0x%08X in Layer %d Window \"%s\"\n", result->ID, g.NavLayer,
//                         g.NavWindow->Name);
//     SetNavID(result->ID, g.NavLayer, result->FocusScopeId, result->RectRel);
//     g.NavDisableHighlight = false;
//     g.NavDisableMouseHover = g.NavMousePosDirty = true;
// }

// static void ImGui::NavUpdateInitResult()
// {
//     // In very rare cases g.NavWindow may be null (e.g. clearing focus after requesting an init request, which does
//     // happen when releasing Alt while clicking on void)
//     ImGuiContext &g = *GImGui;
//     if (!g.NavWindow)
//         return;

//     // Apply result from previous navigation init request (will typically select the first item, unless
//     // SetItemDefaultFocus() has been called)
//     // FIXME-NAV: On _NavFlattened windows, g.NavWindow will only be updated during subsequent frame. Not a problem
//     // currently.
//     IMGUI_DEBUG_LOG_NAV("[nav] NavInitRequest: result NavID 0x%08X in Layer %d Window \"%s\"\n", g.NavInitResultId,
//                         g.NavLayer, g.NavWindow->Name);
//     SetNavID(g.NavInitResultId, g.NavLayer, 0, g.NavInitResultRectRel);
//     if (g.NavInitRequestFromMove)
//     {
//         g.NavDisableHighlight = false;
//         g.NavDisableMouseHover = g.NavMousePosDirty = true;
//     }
// }

// // Handle PageUp/PageDown/Home/End keys
// static float ImGui::NavUpdatePageUpPageDown()
// {
//     ImGuiContext &g = *GImGui;
//     ImGuiIO &io = g.IO;

//     if (g.NavMoveDir != ImGuiDir_None || g.NavWindow == NULL)
//         return 0.0f;
//     if ((g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs) || g.NavWindowingTarget != NULL ||
//         g.NavLayer != ImGuiNavLayer_Main)
//         return 0.0f;

//     ImGuiWindow *window = g.NavWindow;
//     const bool page_up_held = IsKeyDown(io.KeyMap[ImGuiKey_PageUp]) && !IsActiveIdUsingKey(ImGuiKey_PageUp);
//     const bool page_down_held = IsKeyDown(io.KeyMap[ImGuiKey_PageDown]) && !IsActiveIdUsingKey(ImGuiKey_PageDown);
//     const bool home_pressed = IsKeyPressed(io.KeyMap[ImGuiKey_Home]) && !IsActiveIdUsingKey(ImGuiKey_Home);
//     const bool end_pressed = IsKeyPressed(io.KeyMap[ImGuiKey_End]) && !IsActiveIdUsingKey(ImGuiKey_End);
//     if (page_up_held != page_down_held || home_pressed != end_pressed) // If either (not both) are pressed
//     {
//         if (window->DC.NavLayersActiveMask == 0x00 && window->DC.NavHasScroll)
//         {
//             // Fallback manual-scroll when window has no navigable item
//             if (IsKeyPressed(io.KeyMap[ImGuiKey_PageUp], true))
//                 SetScrollY(window, window->Scroll.y - window->InnerRect.GetHeight());
//             else if (IsKeyPressed(io.KeyMap[ImGuiKey_PageDown], true))
//                 SetScrollY(window, window->Scroll.y + window->InnerRect.GetHeight());
//             else if (home_pressed)
//                 SetScrollY(window, 0.0f);
//             else if (end_pressed)
//                 SetScrollY(window, window->ScrollMax.y);
//         }
//         else
//         {
//             ImRect &nav_rect_rel = window->NavRectRel[g.NavLayer];
//             const float page_offset_y =
//                 ImMax(0.0f, window->InnerRect.GetHeight() - window->CalcFontSize() * 1.0f + nav_rect_rel.GetHeight());
//             float nav_scoring_rect_offset_y = 0.0f;
//             if (IsKeyPressed(io.KeyMap[ImGuiKey_PageUp], true))
//             {
//                 nav_scoring_rect_offset_y = -page_offset_y;
//                 g.NavMoveDir = ImGuiDir_Down; // Because our scoring rect is offset up, we request the down direction
//                                               // (so we can always land on the last item)
//                 g.NavMoveClipDir = ImGuiDir_Up;
//                 g.NavMoveRequestFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_AlsoScoreVisibleSet;
//             }
//             else if (IsKeyPressed(io.KeyMap[ImGuiKey_PageDown], true))
//             {
//                 nav_scoring_rect_offset_y = +page_offset_y;
//                 g.NavMoveDir = ImGuiDir_Up; // Because our scoring rect is offset down, we request the up direction (so
//                                             // we can always land on the last item)
//                 g.NavMoveClipDir = ImGuiDir_Down;
//                 g.NavMoveRequestFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_AlsoScoreVisibleSet;
//             }
//             else if (home_pressed)
//             {
//                 // FIXME-NAV: handling of Home/End is assuming that the top/bottom most item will be visible with
//                 // Scroll.y == 0/ScrollMax.y Scrolling will be handled via the ImGuiNavMoveFlags_ScrollToEdge flag, we
//                 // don't scroll immediately to avoid scrolling happening before nav result. Preserve current horizontal
//                 // position if we have any.
//                 nav_rect_rel.Min.y = nav_rect_rel.Max.y = -window->Scroll.y;
//                 if (nav_rect_rel.IsInverted())
//                     nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
//                 g.NavMoveDir = ImGuiDir_Down;
//                 g.NavMoveRequestFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_ScrollToEdge;
//             }
//             else if (end_pressed)
//             {
//                 nav_rect_rel.Min.y = nav_rect_rel.Max.y = window->ScrollMax.y + window->SizeFull.y - window->Scroll.y;
//                 if (nav_rect_rel.IsInverted())
//                     nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
//                 g.NavMoveDir = ImGuiDir_Up;
//                 g.NavMoveRequestFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_ScrollToEdge;
//             }
//             return nav_scoring_rect_offset_y;
//         }
//     }
//     return 0.0f;
// }

// static inline void ImGui::NavUpdateAnyRequestFlag()
// {
//     ImGuiContext &g = *GImGui;
//     g.NavAnyRequest = g.NavMoveRequest || g.NavInitRequest || (IMGUI_DEBUG_NAV_SCORING && g.NavWindow != NULL);
//     if (g.NavAnyRequest)
//         IM_ASSERT(g.NavWindow != NULL);
// }

// static void ImGui::NavEndFrame()
// {
//     ImGuiContext &g = *GImGui;

//     // Show CTRL+TAB list window
//     if (g.NavWindowingTarget != NULL)
//         NavUpdateWindowingOverlay();

//     // Perform wrap-around in menus
//     ImGuiWindow *window = g.NavWrapRequestWindow;
//     ImGuiNavMoveFlags move_flags = g.NavWrapRequestFlags;
//     if (window != NULL && g.NavWindow == window && NavMoveRequestButNoResultYet() &&
//         g.NavMoveRequestForward == ImGuiNavForward_None && g.NavLayer == ImGuiNavLayer_Main)
//     {
//         IM_ASSERT(move_flags != 0); // No points calling this with no wrapping
//         ImRect bb_rel = window->NavRectRel[0];

//         ImGuiDir clip_dir = g.NavMoveDir;
//         if (g.NavMoveDir == ImGuiDir_Left && (move_flags & (ImGuiNavMoveFlags_WrapX | ImGuiNavMoveFlags_LoopX)))
//         {
//             bb_rel.Min.x = bb_rel.Max.x =
//                 ImMax(window->SizeFull.x, window->ContentSize.x + window->WindowPadding.x * 2.0f) - window->Scroll.x;
//             if (move_flags & ImGuiNavMoveFlags_WrapX)
//             {
//                 bb_rel.TranslateY(-bb_rel.GetHeight());
//                 clip_dir = ImGuiDir_Up;
//             }
//             NavMoveRequestForward(g.NavMoveDir, clip_dir, bb_rel, move_flags);
//         }
//         if (g.NavMoveDir == ImGuiDir_Right && (move_flags & (ImGuiNavMoveFlags_WrapX | ImGuiNavMoveFlags_LoopX)))
//         {
//             bb_rel.Min.x = bb_rel.Max.x = -window->Scroll.x;
//             if (move_flags & ImGuiNavMoveFlags_WrapX)
//             {
//                 bb_rel.TranslateY(+bb_rel.GetHeight());
//                 clip_dir = ImGuiDir_Down;
//             }
//             NavMoveRequestForward(g.NavMoveDir, clip_dir, bb_rel, move_flags);
//         }
//         if (g.NavMoveDir == ImGuiDir_Up && (move_flags & (ImGuiNavMoveFlags_WrapY | ImGuiNavMoveFlags_LoopY)))
//         {
//             bb_rel.Min.y = bb_rel.Max.y =
//                 ImMax(window->SizeFull.y, window->ContentSize.y + window->WindowPadding.y * 2.0f) - window->Scroll.y;
//             if (move_flags & ImGuiNavMoveFlags_WrapY)
//             {
//                 bb_rel.TranslateX(-bb_rel.GetWidth());
//                 clip_dir = ImGuiDir_Left;
//             }
//             NavMoveRequestForward(g.NavMoveDir, clip_dir, bb_rel, move_flags);
//         }
//         if (g.NavMoveDir == ImGuiDir_Down && (move_flags & (ImGuiNavMoveFlags_WrapY | ImGuiNavMoveFlags_LoopY)))
//         {
//             bb_rel.Min.y = bb_rel.Max.y = -window->Scroll.y;
//             if (move_flags & ImGuiNavMoveFlags_WrapY)
//             {
//                 bb_rel.TranslateX(+bb_rel.GetWidth());
//                 clip_dir = ImGuiDir_Right;
//             }
//             NavMoveRequestForward(g.NavMoveDir, clip_dir, bb_rel, move_flags);
//         }
//     }
// }

// static void inline NavClampRectToVisibleAreaForMoveDir(ImGuiDir move_dir, ImRect &r, const ImRect &clip_rect)
// {
//     if (move_dir == ImGuiDir_Left || move_dir == ImGuiDir_Right)
//     {
//         r.Min.y = ImClamp(r.Min.y, clip_rect.Min.y, clip_rect.Max.y);
//         r.Max.y = ImClamp(r.Max.y, clip_rect.Min.y, clip_rect.Max.y);
//     }
//     else
//     {
//         r.Min.x = ImClamp(r.Min.x, clip_rect.Min.x, clip_rect.Max.x);
//         r.Max.x = ImClamp(r.Max.x, clip_rect.Min.x, clip_rect.Max.x);
//     }
// }

// static float inline NavScoreItemDistInterval(float a0, float a1, float b0, float b1)
// {
//     if (a1 < b0)
//         return a1 - b0;
//     if (b1 < a0)
//         return a0 - b1;
//     return 0.0f;
// }

// // Scoring function for gamepad/keyboard directional navigation. Based on https://gist.github.com/rygorous/6981057
// static bool ImGui::NavScoreItem(ImGuiNavItemData *result, ImRect cand)
// {
//     ImGuiContext &g = *GImGui;
//     ImGuiWindow *window = g.CurrentWindow;
//     if (g.NavLayer != window->DC.NavLayerCurrent)
//         return false;

//     const ImRect &curr = g.NavScoringRect; // Current modified source rect (NB: we've applied Max.x = Min.x in
//                                            // NavUpdate() to inhibit the effect of having varied item width)
//     g.NavScoringCount++;

//     // When entering through a NavFlattened border, we consider child window items as fully clipped for scoring
//     if (window->ParentWindow == g.NavWindow)
//     {
//         IM_ASSERT((window->Flags | g.NavWindow->Flags) & ImGuiWindowFlags_NavFlattened);
//         if (!window->ClipRect.Overlaps(cand))
//             return false;
//         cand.ClipWithFull(
//             window->ClipRect); // This allows the scored item to not overlap other candidates in the parent window
//     }

//     // We perform scoring on items bounding box clipped by the current clipping rectangle on the other axis (clipping on
//     // our movement axis would give us equal scores for all clipped items) For example, this ensure that items in one
//     // column are not reached when moving vertically from items in another column.
//     NavClampRectToVisibleAreaForMoveDir(g.NavMoveClipDir, cand, window->ClipRect);

//     // Compute distance between boxes
//     // FIXME-NAV: Introducing biases for vertical navigation, needs to be removed.
//     float dbx = NavScoreItemDistInterval(cand.Min.x, cand.Max.x, curr.Min.x, curr.Max.x);
//     float dby = NavScoreItemDistInterval(
//         ImLerp(cand.Min.y, cand.Max.y, 0.2f), ImLerp(cand.Min.y, cand.Max.y, 0.8f),
//         ImLerp(curr.Min.y, curr.Max.y, 0.2f),
//         ImLerp(curr.Min.y, curr.Max.y,
//                0.8f)); // Scale down on Y to keep using box-distance for vertically touching items
//     if (dby != 0.0f && dbx != 0.0f)
//         dbx = (dbx / 1000.0f) + ((dbx > 0.0f) ? +1.0f : -1.0f);
//     float dist_box = ImFabs(dbx) + ImFabs(dby);

//     // Compute distance between centers (this is off by a factor of 2, but we only compare center distances with each
//     // other so it doesn't matter)
//     float dcx = (cand.Min.x + cand.Max.x) - (curr.Min.x + curr.Max.x);
//     float dcy = (cand.Min.y + cand.Max.y) - (curr.Min.y + curr.Max.y);
//     float dist_center = ImFabs(dcx) + ImFabs(dcy); // L1 metric (need this for our connectedness guarantee)

//     // Determine which quadrant of 'curr' our candidate item 'cand' lies in based on distance
//     ImGuiDir quadrant;
//     float dax = 0.0f, day = 0.0f, dist_axial = 0.0f;
//     if (dbx != 0.0f || dby != 0.0f)
//     {
//         // For non-overlapping boxes, use distance between boxes
//         dax = dbx;
//         day = dby;
//         dist_axial = dist_box;
//         quadrant = ImGetDirQuadrantFromDelta(dbx, dby);
//     }
//     else if (dcx != 0.0f || dcy != 0.0f)
//     {
//         // For overlapping boxes with different centers, use distance between centers
//         dax = dcx;
//         day = dcy;
//         dist_axial = dist_center;
//         quadrant = ImGetDirQuadrantFromDelta(dcx, dcy);
//     }
//     else
//     {
//         // Degenerate case: two overlapping buttons with same center, break ties arbitrarily (note that LastItemId here
//         // is really the _previous_ item order, but it doesn't matter)
//         quadrant = (window->DC.LastItemId < g.NavId) ? ImGuiDir_Left : ImGuiDir_Right;
//     }

// #if IMGUI_DEBUG_NAV_SCORING
//     char buf[128];
//     if (IsMouseHoveringRect(cand.Min, cand.Max))
//     {
//         ImFormatString(buf, IM_ARRAYSIZE(buf),
//                        "dbox (%.2f,%.2f->%.4f)\ndcen (%.2f,%.2f->%.4f)\nd (%.2f,%.2f->%.4f)\nnav %c, quadrant %c", dbx,
//                        dby, dist_box, dcx, dcy, dist_center, dax, day, dist_axial, "WENS"[g.NavMoveDir],
//                        "WENS"[quadrant]);
//         ImDrawList *draw_list = GetForegroundDrawList(window);
//         draw_list->AddRect(curr.Min, curr.Max, IM_COL32(255, 200, 0, 100));
//         draw_list->AddRect(cand.Min, cand.Max, IM_COL32(255, 255, 0, 200));
//         draw_list->AddRectFilled(cand.Max - ImVec2(4, 4), cand.Max + CalcTextSize(buf) + ImVec2(4, 4),
//                                  IM_COL32(40, 0, 0, 150));
//         draw_list->AddText(g.IO.FontDefault, 13.0f, cand.Max, ~0U, buf);
//     }
//     else if (g.IO.KeyCtrl) // Hold to preview score in matching quadrant. Press C to rotate.
//     {
//         if (IsKeyPressedMap(ImGuiKey_C))
//         {
//             g.NavMoveDirLast = (ImGuiDir)((g.NavMoveDirLast + 1) & 3);
//             g.IO.KeysDownDuration[g.IO.KeyMap[ImGuiKey_C]] = 0.01f;
//         }
//         if (quadrant == g.NavMoveDir)
//         {
//             ImFormatString(buf, IM_ARRAYSIZE(buf), "%.0f/%.0f", dist_box, dist_center);
//             ImDrawList *draw_list = GetForegroundDrawList(window);
//             draw_list->AddRectFilled(cand.Min, cand.Max, IM_COL32(255, 0, 0, 200));
//             draw_list->AddText(g.IO.FontDefault, 13.0f, cand.Min, IM_COL32(255, 255, 255, 255), buf);
//         }
//     }
// #endif

//     // Is it in the quadrant we're interesting in moving to?
//     bool new_best = false;
//     if (quadrant == g.NavMoveDir)
//     {
//         // Does it beat the current best candidate?
//         if (dist_box < result->DistBox)
//         {
//             result->DistBox = dist_box;
//             result->DistCenter = dist_center;
//             return true;
//         }
//         if (dist_box == result->DistBox)
//         {
//             // Try using distance between center points to break ties
//             if (dist_center < result->DistCenter)
//             {
//                 result->DistCenter = dist_center;
//                 new_best = true;
//             }
//             else if (dist_center == result->DistCenter)
//             {
//                 // Still tied! we need to be extra-careful to make sure everything gets linked properly. We consistently
//                 // break ties by symbolically moving "later" items (with higher index) to the right/downwards by an
//                 // infinitesimal amount since we the current "best" button already (so it must have a lower index), this
//                 // is fairly easy. This rule ensures that all buttons with dx==dy==0 will end up being linked in order
//                 // of appearance along the x axis.
//                 if (((g.NavMoveDir == ImGuiDir_Up || g.NavMoveDir == ImGuiDir_Down) ? dby : dbx) <
//                     0.0f) // moving bj to the right/down decreases distance
//                     new_best = true;
//             }
//         }
//     }

//     // Axial check: if 'curr' has no link at all in some direction and 'cand' lies roughly in that direction, add a
//     // tentative link. This will only be kept if no "real" matches are found, so it only augments the graph produced by
//     // the above method using extra links. (important, since it doesn't guarantee strong connectedness) This is just to
//     // avoid buttons having no links in a particular direction when there's a suitable neighbor. you get good graphs
//     // without this too. 2017/09/29: FIXME: This now currently only enabled inside menu bars, ideally we'd disable it
//     // everywhere. Menus in particular need to catch failure. For general navigation it feels awkward. Disabling it may
//     // lead to disconnected graphs when nodes are very spaced out on different axis. Perhaps consider offering this as
//     // an option?
//     if (result->DistBox == FLT_MAX && dist_axial < result->DistAxial) // Check axial match
//         if (g.NavLayer == ImGuiNavLayer_Menu && !(g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu))
//             if ((g.NavMoveDir == ImGuiDir_Left && dax < 0.0f) || (g.NavMoveDir == ImGuiDir_Right && dax > 0.0f) ||
//                 (g.NavMoveDir == ImGuiDir_Up && day < 0.0f) || (g.NavMoveDir == ImGuiDir_Down && day > 0.0f))
//             {
//                 result->DistAxial = dist_axial;
//                 new_best = true;
//             }

//     return new_best;
// }

// static void ImGui::NavApplyItemToResult(ImGuiNavItemData *result, ImGuiWindow *window, ImGuiID id,
//                                         const ImRect &nav_bb_rel)
// {
//     result->Window = window;
//     result->ID = id;
//     result->FocusScopeId = window->DC.NavFocusScopeIdCurrent;
//     result->RectRel = nav_bb_rel;
// }

// // We get there when either NavId == id, or when g.NavAnyRequest is set (which is updated by NavUpdateAnyRequestFlag
// // above)
// static void ImGui::NavProcessItem(ImGuiWindow *window, const ImRect &nav_bb, const ImGuiID id)
// {
//     ImGuiContext &g = *GImGui;
//     // if (!g.IO.NavActive)  // [2017/10/06] Removed this possibly redundant test but I am not sure of all the
//     // side-effects yet. Some of the feature here will need to work regardless of using a _NoNavInputs flag.
//     //    return;

//     const ImGuiItemFlags item_flags = g.CurrentItemFlags;
//     const ImRect nav_bb_rel(nav_bb.Min - window->Pos, nav_bb.Max - window->Pos);

//     // Process Init Request
//     if (g.NavInitRequest && g.NavLayer == window->DC.NavLayerCurrent)
//     {
//         // Even if 'ImGuiItemFlags_NoNavDefaultFocus' is on (typically collapse/close button) we record the first
//         // ResultId so they can be used as a fallback
//         if (!(item_flags & ImGuiItemFlags_NoNavDefaultFocus) || g.NavInitResultId == 0)
//         {
//             g.NavInitResultId = id;
//             g.NavInitResultRectRel = nav_bb_rel;
//         }
//         if (!(item_flags & ImGuiItemFlags_NoNavDefaultFocus))
//         {
//             g.NavInitRequest = false; // Found a match, clear request
//             NavUpdateAnyRequestFlag();
//         }
//     }

//     // Process Move Request (scoring for navigation)
//     // FIXME-NAV: Consider policy for double scoring (scoring from NavScoringRectScreen + scoring from a rect wrapped
//     // according to current wrapping policy)
//     if ((g.NavId != id || (g.NavMoveRequestFlags & ImGuiNavMoveFlags_AllowCurrentNavId)) &&
//         !(item_flags & (ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNav)))
//     {
//         ImGuiNavItemData *result = (window == g.NavWindow) ? &g.NavMoveResultLocal : &g.NavMoveResultOther;
// #if IMGUI_DEBUG_NAV_SCORING
//         // [DEBUG] Score all items in NavWindow at all times
//         if (!g.NavMoveRequest)
//             g.NavMoveDir = g.NavMoveDirLast;
//         bool new_best = NavScoreItem(result, nav_bb) && g.NavMoveRequest;
// #else
//         bool new_best = g.NavMoveRequest && NavScoreItem(result, nav_bb);
// #endif
//         if (new_best)
//             NavApplyItemToResult(result, window, id, nav_bb_rel);

//         // Features like PageUp/PageDown need to maintain a separate score for the visible set of items.
//         const float VISIBLE_RATIO = 0.70f;
//         if ((g.NavMoveRequestFlags & ImGuiNavMoveFlags_AlsoScoreVisibleSet) && window->ClipRect.Overlaps(nav_bb))
//             if (ImClamp(nav_bb.Max.y, window->ClipRect.Min.y, window->ClipRect.Max.y) -
//                     ImClamp(nav_bb.Min.y, window->ClipRect.Min.y, window->ClipRect.Max.y) >=
//                 (nav_bb.Max.y - nav_bb.Min.y) * VISIBLE_RATIO)
//                 if (NavScoreItem(&g.NavMoveResultLocalVisibleSet, nav_bb))
//                     NavApplyItemToResult(&g.NavMoveResultLocalVisibleSet, window, id, nav_bb_rel);
//     }

//     // Update window-relative bounding box of navigated item
//     if (g.NavId == id)
//     {
//         g.NavWindow =
//             window; // Always refresh g.NavWindow, because some operations such as FocusItem() don't have a window.
//         g.NavLayer = window->DC.NavLayerCurrent;
//         g.NavFocusScopeId = window->DC.NavFocusScopeIdCurrent;
//         g.NavIdIsAlive = true;
//         window->NavRectRel[window->DC.NavLayerCurrent] =
//             nav_bb_rel; // Store item bounding box (relative to window position)
//     }
// }

// static ImVec2 ImGui::NavCalcPreferredRefPos()
// {
//     ImGuiContext &g = *GImGui;
//     if (g.NavDisableHighlight || !g.NavDisableMouseHover || !g.NavWindow)
//     {
//         // Mouse (we need a fallback in case the mouse becomes invalid after being used)
//         if (IsMousePosValid(&g.IO.MousePos))
//             return g.IO.MousePos;
//         return g.LastValidMousePos;
//     }
//     else
//     {
//         // When navigation is active and mouse is disabled, decide on an arbitrary position around the bottom left of
//         // the currently navigated item.
//         const ImRect &rect_rel = g.NavWindow->NavRectRel[g.NavLayer];
//         ImVec2 pos = g.NavWindow->Pos + ImVec2(rect_rel.Min.x + ImMin(g.Style.FramePadding.x * 4, rect_rel.GetWidth()),
//                                                rect_rel.Max.y - ImMin(g.Style.FramePadding.y, rect_rel.GetHeight()));
//         ImGuiViewport *viewport = GetMainViewport();
//         return ImFloor(ImClamp(
//             pos, viewport->Pos,
//             viewport->Pos + viewport->Size)); // ImFloor() is important because non-integer mouse position application
//                                               // in backend might be lossy and result in undesirable non-zero delta.
//     }
// }

// // FIXME: This could be replaced by updating a frame number in each window when (window == NavWindow) and (NavLayer ==
// // 0). This way we could find the last focused window among our children. It would be much less confusing this way?
// static void ImGui::NavSaveLastChildNavWindowIntoParent(ImGuiWindow *nav_window)
// {
//     ImGuiWindow *parent = nav_window;
//     while (parent && parent->RootWindow != parent &&
//            (parent->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu)) == 0)
//         parent = parent->ParentWindow;
//     if (parent && parent != nav_window)
//         parent->NavLastChildNavWindow = nav_window;
// }

// // Restore the last focused child.
// // Call when we are expected to land on the Main Layer (0) after FocusWindow()
// static ImGuiWindow *ImGui::NavRestoreLastChildNavWindow(ImGuiWindow *window)
// {
//     if (window->NavLastChildNavWindow && window->NavLastChildNavWindow->WasActive)
//         return window->NavLastChildNavWindow;
//     return window;
// }

// void ImGui::NavRestoreLayer(ImGuiNavLayer layer)
// {
//     ImGuiContext &g = *GImGui;
//     if (layer == ImGuiNavLayer_Main)
//         g.NavWindow = NavRestoreLastChildNavWindow(g.NavWindow);
//     ImGuiWindow *window = g.NavWindow;
//     if (window->NavLastIds[layer] != 0)
//     {
//         SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
//         g.NavDisableHighlight = false;
//         g.NavDisableMouseHover = g.NavMousePosDirty = true;
//     }
//     else
//     {
//         g.NavLayer = layer;
//         NavInitWindow(window, true);
//     }
// }

// static int ImGui::FindWindowFocusIndex(ImGuiWindow *window)
// {
//     ImGuiContext &g = *GImGui;
//     IM_UNUSED(g);
//     int order = window->FocusOrder;
//     IM_ASSERT(g.WindowsFocusOrder[order] == window);
//     return order;
// }

// static void ImGui::ErrorCheckNewFrameSanityChecks()
// {
//     ImGuiContext &g = *GImGui;

//     // Check user IM_ASSERT macro
//     // (IF YOU GET A WARNING OR COMPILE ERROR HERE: it means your assert macro is incorrectly defined!
//     //  If your macro uses multiple statements, it NEEDS to be surrounded by a 'do { ... } while (0)' block.
//     //  This is a common C/C++ idiom to allow multiple statements macros to be used in control flow blocks.)
//     // #define IM_ASSERT(EXPR)   if (SomeCode(EXPR)) SomeMoreCode();                    // Wrong!
//     // #define IM_ASSERT(EXPR)   do { if (SomeCode(EXPR)) SomeMoreCode(); } while (0)   // Correct!
//     if (true)
//         IM_ASSERT(1);
//     else
//         IM_ASSERT(0);

//     // Check user data
//     // (We pass an error message in the assert expression to make it visible to programmers who are not using a
//     // debugger, as most assert handlers display their argument)
//     IM_ASSERT(g.Initialized);
//     IM_ASSERT((g.IO.DeltaTime > 0.0f || g.FrameCount == 0) && "Need a positive DeltaTime!");
//     IM_ASSERT((g.FrameCount == 0 || g.FrameCountEnded == g.FrameCount) &&
//               "Forgot to call Render() or EndFrame() at the end of the previous frame?");
//     IM_ASSERT(g.IO.DisplaySize.x >= 0.0f && g.IO.DisplaySize.y >= 0.0f && "Invalid DisplaySize value!");
//     IM_ASSERT(g.IO.Fonts->Fonts.Size > 0 &&
//               "Font Atlas not built. Did you call io.Fonts->GetTexDataAsRGBA32() / GetTexDataAsAlpha8()?");
//     IM_ASSERT(g.IO.Fonts->Fonts[0]->IsLoaded() &&
//               "Font Atlas not built. Did you call io.Fonts->GetTexDataAsRGBA32() / GetTexDataAsAlpha8()?");
//     IM_ASSERT(g.Style.CurveTessellationTol > 0.0f && "Invalid style setting!");
//     IM_ASSERT(g.Style.CircleTessellationMaxError > 0.0f && "Invalid style setting!");
//     IM_ASSERT(g.Style.Alpha >= 0.0f && g.Style.Alpha <= 1.0f &&
//               "Invalid style setting!"); // Allows us to avoid a few clamps in color computations
//     IM_ASSERT(g.Style.WindowMinSize.x >= 1.0f && g.Style.WindowMinSize.y >= 1.0f && "Invalid style setting.");
//     IM_ASSERT(g.Style.WindowMenuButtonPosition == ImGuiDir_None || g.Style.WindowMenuButtonPosition == ImGuiDir_Left ||
//               g.Style.WindowMenuButtonPosition == ImGuiDir_Right);
//     for (int n = 0; n < ImGuiKey_COUNT; n++)
//         IM_ASSERT(g.IO.KeyMap[n] >= -1 && g.IO.KeyMap[n] < IM_ARRAYSIZE(g.IO.KeysDown) &&
//                   "io.KeyMap[] contains an out of bound value (need to be 0..512, or -1 for unmapped key)");

//     // Check: required key mapping (we intentionally do NOT check all keys to not pressure user into setting up
//     // everything, but Space is required and was only added in 1.60 WIP)
//     if (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)
//         IM_ASSERT(g.IO.KeyMap[ImGuiKey_Space] != -1 &&
//                   "ImGuiKey_Space is not mapped, required for keyboard navigation.");

//     // Check: the io.ConfigWindowsResizeFromEdges option requires backend to honor mouse cursor changes and set the
//     // ImGuiBackendFlags_HasMouseCursors flag accordingly.
//     if (g.IO.ConfigWindowsResizeFromEdges && !(g.IO.BackendFlags & ImGuiBackendFlags_HasMouseCursors))
//         g.IO.ConfigWindowsResizeFromEdges = false;
// }

// static void ImGui::ErrorCheckEndFrameSanityChecks()
// {
//     ImGuiContext &g = *GImGui;

//     // Verify that io.KeyXXX fields haven't been tampered with. Key mods should not be modified between NewFrame() and
//     // EndFrame() One possible reason leading to this assert is that your backends update inputs _AFTER_ NewFrame(). It
//     // is known that when some modal native windows called mid-frame takes focus away, some backends such as GLFW will
//     // send key release events mid-frame. This would normally trigger this assertion and lead to sheared inputs.
//     // We silently accommodate for this case by ignoring/ the case where all io.KeyXXX modifiers were released (aka
//     // key_mod_flags == 0), while still correctly asserting on mid-frame key press events.
//     const ImGuiKeyModFlags key_mod_flags = GetMergedKeyModFlags();
//     IM_ASSERT((key_mod_flags == 0 || g.IO.KeyMods == key_mod_flags) &&
//               "Mismatching io.KeyCtrl/io.KeyShift/io.KeyAlt/io.KeySuper vs io.KeyMods");
//     IM_UNUSED(key_mod_flags);

//     // Recover from errors
//     // ErrorCheckEndFrameRecover();

//     // Report when there is a mismatch of Begin/BeginChild vs End/EndChild calls. Important: Remember that the
//     // Begin/BeginChild API requires you to always call End/EndChild even if Begin/BeginChild returns false! (this is
//     // unfortunately inconsistent with most other Begin* API).
//     if (g.CurrentWindowStack.Size != 1)
//     {
//         if (g.CurrentWindowStack.Size > 1)
//         {
//             IM_ASSERT_USER_ERROR(
//                 g.CurrentWindowStack.Size == 1,
//                 "Mismatched Begin/BeginChild vs End/EndChild calls: did you forget to call End/EndChild?");
//             while (g.CurrentWindowStack.Size > 1)
//                 End();
//         }
//         else
//         {
//             IM_ASSERT_USER_ERROR(
//                 g.CurrentWindowStack.Size == 1,
//                 "Mismatched Begin/BeginChild vs End/EndChild calls: did you call End/EndChild too much?");
//         }
//     }

//     IM_ASSERT_USER_ERROR(g.GroupStack.Size == 0, "Missing EndGroup call!");
// }

// // Called by NewFrame()
// void ImGui::UpdateSettings()
// {
//     // Load settings on first frame (if not explicitly loaded manually before)
//     ImGuiContext &g = *GImGui;
//     if (!g.SettingsLoaded)
//     {
//         IM_ASSERT(g.SettingsWindows.empty());
//         if (g.IO.IniFilename)
//             LoadIniSettingsFromDisk(g.IO.IniFilename);
//         g.SettingsLoaded = true;
//     }

//     // Save settings (with a delay after the last modification, so we don't spam disk too much)
//     if (g.SettingsDirtyTimer > 0.0f)
//     {
//         g.SettingsDirtyTimer -= g.IO.DeltaTime;
//         if (g.SettingsDirtyTimer <= 0.0f)
//         {
//             if (g.IO.IniFilename != NULL)
//                 SaveIniSettingsToDisk(g.IO.IniFilename);
//             else
//                 g.IO.WantSaveIniSettings = true; // Let user know they can call SaveIniSettingsToMemory(). user will
//                                                  // need to clear io.WantSaveIniSettings themselves.
//             g.SettingsDirtyTimer = 0.0f;
//         }
//     }
// }

// static void ImGui::UpdateMouseInputs()
// {
//     ImGuiContext &g = *GImGui;

//     // Round mouse position to avoid spreading non-rounded position (e.g. UpdateManualResize doesn't support them well)
//     if (IsMousePosValid(&g.IO.MousePos))
//         g.IO.MousePos = g.LastValidMousePos = ImFloor(g.IO.MousePos);

//     // If mouse just appeared or disappeared (usually denoted by -FLT_MAX components) we cancel out movement in
//     // MouseDelta
//     if (IsMousePosValid(&g.IO.MousePos) && IsMousePosValid(&g.IO.MousePosPrev))
//         g.IO.MouseDelta = g.IO.MousePos - g.IO.MousePosPrev;
//     else
//         g.IO.MouseDelta = ImVec2(0.0f, 0.0f);
//     if (g.IO.MouseDelta.x != 0.0f || g.IO.MouseDelta.y != 0.0f)
//         g.NavDisableMouseHover = false;

//     g.IO.MousePosPrev = g.IO.MousePos;
//     for (int i = 0; i < IM_ARRAYSIZE(g.IO.MouseDown); i++)
//     {
//         g.IO.MouseClicked[i] = g.IO.MouseDown[i] && g.IO.MouseDownDuration[i] < 0.0f;
//         g.IO.MouseReleased[i] = !g.IO.MouseDown[i] && g.IO.MouseDownDuration[i] >= 0.0f;
//         g.IO.MouseDownDurationPrev[i] = g.IO.MouseDownDuration[i];
//         g.IO.MouseDownDuration[i] =
//             g.IO.MouseDown[i] ? (g.IO.MouseDownDuration[i] < 0.0f ? 0.0f : g.IO.MouseDownDuration[i] + g.IO.DeltaTime)
//                               : -1.0f;
//         g.IO.MouseDoubleClicked[i] = false;
//         if (g.IO.MouseClicked[i])
//         {
//             if ((float)(g.Time - g.IO.MouseClickedTime[i]) < g.IO.MouseDoubleClickTime)
//             {
//                 ImVec2 delta_from_click_pos =
//                     IsMousePosValid(&g.IO.MousePos) ? (g.IO.MousePos - g.IO.MouseClickedPos[i]) : ImVec2(0.0f, 0.0f);
//                 if (ImLengthSqr(delta_from_click_pos) < g.IO.MouseDoubleClickMaxDist * g.IO.MouseDoubleClickMaxDist)
//                     g.IO.MouseDoubleClicked[i] = true;
//                 g.IO.MouseClickedTime[i] =
//                     -g.IO.MouseDoubleClickTime *
//                     2.0f; // Mark as "old enough" so the third click isn't turned into a double-click
//             }
//             else
//             {
//                 g.IO.MouseClickedTime[i] = g.Time;
//             }
//             g.IO.MouseClickedPos[i] = g.IO.MousePos;
//             g.IO.MouseDownWasDoubleClick[i] = g.IO.MouseDoubleClicked[i];
//             g.IO.MouseDragMaxDistanceAbs[i] = ImVec2(0.0f, 0.0f);
//             g.IO.MouseDragMaxDistanceSqr[i] = 0.0f;
//         }
//         else if (g.IO.MouseDown[i])
//         {
//             // Maintain the maximum distance we reaching from the initial click position, which is used with dragging
//             // threshold
//             ImVec2 delta_from_click_pos =
//                 IsMousePosValid(&g.IO.MousePos) ? (g.IO.MousePos - g.IO.MouseClickedPos[i]) : ImVec2(0.0f, 0.0f);
//             g.IO.MouseDragMaxDistanceSqr[i] = ImMax(g.IO.MouseDragMaxDistanceSqr[i], ImLengthSqr(delta_from_click_pos));
//             g.IO.MouseDragMaxDistanceAbs[i].x =
//                 ImMax(g.IO.MouseDragMaxDistanceAbs[i].x,
//                       delta_from_click_pos.x < 0.0f ? -delta_from_click_pos.x : delta_from_click_pos.x);
//             g.IO.MouseDragMaxDistanceAbs[i].y =
//                 ImMax(g.IO.MouseDragMaxDistanceAbs[i].y,
//                       delta_from_click_pos.y < 0.0f ? -delta_from_click_pos.y : delta_from_click_pos.y);
//         }
//         if (!g.IO.MouseDown[i] && !g.IO.MouseReleased[i])
//             g.IO.MouseDownWasDoubleClick[i] = false;
//         if (g.IO.MouseClicked[i]) // Clicking any mouse button reactivate mouse hovering which may have been deactivated
//                                   // by gamepad/keyboard navigation
//             g.NavDisableMouseHover = false;
//     }
// }

// static void StartLockWheelingWindow(ImGuiWindow *window)
// {
//     ImGuiContext &g = *GImGui;
//     if (g.WheelingWindow == window)
//         return;
//     g.WheelingWindow = window;
//     g.WheelingWindowRefMousePos = g.IO.MousePos;
//     g.WheelingWindowTimer = WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER;
// }

// void ImGui::UpdateMouseWheel()
// {
//     ImGuiContext &g = *GImGui;

//     // Reset the locked window if we move the mouse or after the timer elapses
//     if (g.WheelingWindow != NULL)
//     {
//         g.WheelingWindowTimer -= g.IO.DeltaTime;
//         if (IsMousePosValid() && ImLengthSqr(g.IO.MousePos - g.WheelingWindowRefMousePos) >
//                                      g.IO.MouseDragThreshold * g.IO.MouseDragThreshold)
//             g.WheelingWindowTimer = 0.0f;
//         if (g.WheelingWindowTimer <= 0.0f)
//         {
//             g.WheelingWindow = NULL;
//             g.WheelingWindowTimer = 0.0f;
//         }
//     }

//     if (g.IO.MouseWheel == 0.0f && g.IO.MouseWheelH == 0.0f)
//         return;

//     if ((g.ActiveId != 0 && g.ActiveIdUsingMouseWheel) ||
//         (g.HoveredIdPreviousFrame != 0 && g.HoveredIdPreviousFrameUsingMouseWheel))
//         return;

//     ImGuiWindow *window = g.WheelingWindow ? g.WheelingWindow : g.HoveredWindow;
//     if (!window || window->Collapsed)
//         return;

//     // Zoom / Scale window
//     // FIXME-OBSOLETE: This is an old feature, it still works but pretty much nobody is using it and may be best
//     // redesigned.
//     if (g.IO.MouseWheel != 0.0f && g.IO.KeyCtrl && g.IO.FontAllowUserScaling)
//     {
//         StartLockWheelingWindow(window);
//         const float new_font_scale = ImClamp(window->FontWindowScale + g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
//         const float scale = new_font_scale / window->FontWindowScale;
//         window->FontWindowScale = new_font_scale;
//         if (window == window->RootWindow)
//         {
//             const ImVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
//             SetWindowPos(window, window->Pos + offset, 0);
//             window->Size = ImFloor(window->Size * scale);
//             window->SizeFull = ImFloor(window->SizeFull * scale);
//         }
//         return;
//     }

//     // Mouse wheel scrolling
//     // If a child window has the ImGuiWindowFlags_NoScrollWithMouse flag, we give a chance to scroll its parent
//     if (g.IO.KeyCtrl)
//         return;

//     // As a standard behavior holding SHIFT while using Vertical Mouse Wheel triggers Horizontal scroll instead
//     // (we avoid doing it on OSX as it the OS input layer handles this already)
//     const bool swap_axis = g.IO.KeyShift && !g.IO.ConfigMacOSXBehaviors;
//     const float wheel_y = swap_axis ? 0.0f : g.IO.MouseWheel;
//     const float wheel_x = swap_axis ? g.IO.MouseWheel : g.IO.MouseWheelH;

//     // Vertical Mouse Wheel scrolling
//     if (wheel_y != 0.0f)
//     {
//         StartLockWheelingWindow(window);
//         while ((window->Flags & ImGuiWindowFlags_ChildWindow) &&
//                ((window->ScrollMax.y == 0.0f) || ((window->Flags & ImGuiWindowFlags_NoScrollWithMouse) &&
//                                                   !(window->Flags & ImGuiWindowFlags_NoMouseInputs))))
//             window = window->ParentWindow;
//         if (!(window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & ImGuiWindowFlags_NoMouseInputs))
//         {
//             float max_step = window->InnerRect.GetHeight() * 0.67f;
//             float scroll_step = ImFloor(ImMin(5 * window->CalcFontSize(), max_step));
//             SetScrollY(window, window->Scroll.y - wheel_y * scroll_step);
//         }
//     }

//     // Horizontal Mouse Wheel scrolling, or Vertical Mouse Wheel w/ Shift held
//     if (wheel_x != 0.0f)
//     {
//         StartLockWheelingWindow(window);
//         while ((window->Flags & ImGuiWindowFlags_ChildWindow) &&
//                ((window->ScrollMax.x == 0.0f) || ((window->Flags & ImGuiWindowFlags_NoScrollWithMouse) &&
//                                                   !(window->Flags & ImGuiWindowFlags_NoMouseInputs))))
//             window = window->ParentWindow;
//         if (!(window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & ImGuiWindowFlags_NoMouseInputs))
//         {
//             float max_step = window->InnerRect.GetWidth() * 0.67f;
//             float scroll_step = ImFloor(ImMin(2 * window->CalcFontSize(), max_step));
//             SetScrollX(window, window->Scroll.x - wheel_x * scroll_step);
//         }
//     }
// }

// void ImGui::UpdateTabFocus()
// {
//     ImGuiContext &g = *GImGui;

//     // Pressing TAB activate widget focus
//     g.TabFocusPressed = (g.NavWindow && g.NavWindow->Active && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs) &&
//                          !g.IO.KeyCtrl && IsKeyPressedMap(ImGuiKey_Tab));
//     if (g.ActiveId == 0 && g.TabFocusPressed)
//     {
//         // - This path is only taken when no widget are active/tabbed-into yet.
//         //   Subsequent tabbing will be processed by FocusableItemRegister()
//         // - Note that SetKeyboardFocusHere() sets the Next fields mid-frame. To be consistent we also
//         //   manipulate the Next fields here even though they will be turned into Curr fields below.
//         g.TabFocusRequestNextWindow = g.NavWindow;
//         g.TabFocusRequestNextCounterRegular = INT_MAX;
//         if (g.NavId != 0 && g.NavIdTabCounter != INT_MAX)
//             g.TabFocusRequestNextCounterTabStop = g.NavIdTabCounter + (g.IO.KeyShift ? -1 : 0);
//         else
//             g.TabFocusRequestNextCounterTabStop = g.IO.KeyShift ? -1 : 0;
//     }

//     // Turn queued focus request into current one
//     g.TabFocusRequestCurrWindow = NULL;
//     g.TabFocusRequestCurrCounterRegular = g.TabFocusRequestCurrCounterTabStop = INT_MAX;
//     if (g.TabFocusRequestNextWindow != NULL)
//     {
//         ImGuiWindow *window = g.TabFocusRequestNextWindow;
//         g.TabFocusRequestCurrWindow = window;
//         if (g.TabFocusRequestNextCounterRegular != INT_MAX && window->DC.FocusCounterRegular != -1)
//             g.TabFocusRequestCurrCounterRegular =
//                 ImModPositive(g.TabFocusRequestNextCounterRegular, window->DC.FocusCounterRegular + 1);
//         if (g.TabFocusRequestNextCounterTabStop != INT_MAX && window->DC.FocusCounterTabStop != -1)
//             g.TabFocusRequestCurrCounterTabStop =
//                 ImModPositive(g.TabFocusRequestNextCounterTabStop, window->DC.FocusCounterTabStop + 1);
//         g.TabFocusRequestNextWindow = NULL;
//         g.TabFocusRequestNextCounterRegular = g.TabFocusRequestNextCounterTabStop = INT_MAX;
//     }

//     g.NavIdTabCounter = INT_MAX;
// }

// // [DEBUG] Item picker tool - start with DebugStartItemPicker() - useful to visually select an item and break into its
// // call-stack.
// void ImGui::UpdateDebugToolItemPicker()
// {
//     ImGuiContext &g = *GImGui;
//     g.DebugItemPickerBreakId = 0;
//     if (g.DebugItemPickerActive)
//     {
//         const ImGuiID hovered_id = g.HoveredIdPreviousFrame;
//         ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
//         if (ImGui::IsKeyPressedMap(ImGuiKey_Escape))
//             g.DebugItemPickerActive = false;
//         if (ImGui::IsMouseClicked(0) && hovered_id)
//         {
//             g.DebugItemPickerBreakId = hovered_id;
//             g.DebugItemPickerActive = false;
//         }
//         ImGui::SetNextWindowBgAlpha(0.60f);
//         ImGui::BeginTooltip();
//         ImGui::Text("HoveredId: 0x%08X", hovered_id);
//         ImGui::Text("Press ESC to abort picking.");
//         ImGui::TextColored(GetStyleColorVec4(hovered_id ? ImGuiCol_Text : ImGuiCol_TextDisabled),
//                            "Click to break in debugger!");
//         ImGui::EndTooltip();
//     }
// }

// static ImRect GetResizeBorderRect(ImGuiWindow *window, int border_n, float perp_padding, float thickness)
// {
//     ImRect rect = window->Rect();
//     if (thickness == 0.0f)
//         rect.Max -= ImVec2(1, 1);
//     if (border_n == ImGuiDir_Left)
//     {
//         return ImRect(rect.Min.x - thickness, rect.Min.y + perp_padding, rect.Min.x + thickness,
//                       rect.Max.y - perp_padding);
//     }
//     if (border_n == ImGuiDir_Right)
//     {
//         return ImRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x + thickness,
//                       rect.Max.y - perp_padding);
//     }
//     if (border_n == ImGuiDir_Up)
//     {
//         return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness, rect.Max.x - perp_padding,
//                       rect.Min.y + thickness);
//     }
//     if (border_n == ImGuiDir_Down)
//     {
//         return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding,
//                       rect.Max.y + thickness);
//     }
//     IM_ASSERT(0);
//     return ImRect();
// }

// // Handle resize for: Resize Grips, Borders, Gamepad
// // Return true when using auto-fit (double click on resize grip)
// static bool ImGui::UpdateWindowManualResize(ImGuiWindow *window, const ImVec2 &size_auto_fit, int *border_held,
//                                             int resize_grip_count, ImU32 resize_grip_col[4],
//                                             const ImRect &visibility_rect)
// {
//     ImGuiContext &g = *GImGui;
//     ImGuiWindowFlags flags = window->Flags;

//     if ((flags & ImGuiWindowFlags_NoResize) || (flags & ImGuiWindowFlags_AlwaysAutoResize) ||
//         window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
//         return false;
//     if (window->WasActive ==
//         false) // Early out to avoid running this code for e.g. an hidden implicit/fallback Debug window.
//         return false;

//     bool ret_auto_fit = false;
//     const int resize_border_count = g.IO.ConfigWindowsResizeFromEdges ? 4 : 0;
//     const float grip_draw_size = IM_FLOOR(ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
//     const float grip_hover_inner_size = IM_FLOOR(grip_draw_size * 0.75f);
//     const float grip_hover_outer_size = g.IO.ConfigWindowsResizeFromEdges ? WINDOWS_HOVER_PADDING : 0.0f;

//     ImVec2 pos_target(FLT_MAX, FLT_MAX);
//     ImVec2 size_target(FLT_MAX, FLT_MAX);

//     // Resize grips and borders are on layer 1
//     window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

//     // Manual resize grips
//     PushID("#RESIZE");
//     for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
//     {
//         const ImGuiResizeGripDef &def = resize_grip_def[resize_grip_n];
//         const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, def.CornerPosN);

//         // Using the FlattenChilds button flag we make the resize button accessible even if we are hovering over a child
//         // window
//         bool hovered, held;
//         ImRect resize_rect(corner - def.InnerDir * grip_hover_outer_size,
//                            corner + def.InnerDir * grip_hover_inner_size);
//         if (resize_rect.Min.x > resize_rect.Max.x)
//             ImSwap(resize_rect.Min.x, resize_rect.Max.x);
//         if (resize_rect.Min.y > resize_rect.Max.y)
//             ImSwap(resize_rect.Min.y, resize_rect.Max.y);
//         ImGuiID resize_grip_id = window->GetID(resize_grip_n); // == GetWindowResizeCornerID()
//         ButtonBehavior(resize_rect, resize_grip_id, &hovered, &held,
//                        ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);
//         // GetForegroundDrawList(window)->AddRect(resize_rect.Min, resize_rect.Max, IM_COL32(255, 255, 0, 255));
//         if (hovered || held)
//             g.MouseCursor = (resize_grip_n & 1) ? ImGuiMouseCursor_ResizeNESW : ImGuiMouseCursor_ResizeNWSE;

//         if (held && g.IO.MouseDoubleClicked[0] && resize_grip_n == 0)
//         {
//             // Manual auto-fit when double-clicking
//             size_target = CalcWindowSizeAfterConstraint(window, size_auto_fit);
//             ret_auto_fit = true;
//             ClearActiveID();
//         }
//         else if (held)
//         {
//             // Resize from any of the four corners
//             // We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
//             ImVec2 clamp_min = ImVec2(def.CornerPosN.x == 1.0f ? visibility_rect.Min.x : -FLT_MAX,
//                                       def.CornerPosN.y == 1.0f ? visibility_rect.Min.y : -FLT_MAX);
//             ImVec2 clamp_max = ImVec2(def.CornerPosN.x == 0.0f ? visibility_rect.Max.x : +FLT_MAX,
//                                       def.CornerPosN.y == 0.0f ? visibility_rect.Max.y : +FLT_MAX);
//             ImVec2 corner_target = g.IO.MousePos - g.ActiveIdClickOffset +
//                                    ImLerp(def.InnerDir * grip_hover_outer_size, def.InnerDir * -grip_hover_inner_size,
//                                           def.CornerPosN); // Corner of the window corresponding to our corner grip
//             corner_target = ImClamp(corner_target, clamp_min, clamp_max);
//             CalcResizePosSizeFromAnyCorner(window, corner_target, def.CornerPosN, &pos_target, &size_target);
//         }

//         // Only lower-left grip is visible before hovering/activating
//         if (resize_grip_n == 0 || held || hovered)
//             resize_grip_col[resize_grip_n] = GetColorU32(held      ? ImGuiCol_ResizeGripActive
//                                                          : hovered ? ImGuiCol_ResizeGripHovered
//                                                                    : ImGuiCol_ResizeGrip);
//     }
//     for (int border_n = 0; border_n < resize_border_count; border_n++)
//     {
//         const ImGuiResizeBorderDef &def = resize_border_def[border_n];
//         const ImGuiAxis axis = (border_n == ImGuiDir_Left || border_n == ImGuiDir_Right) ? ImGuiAxis_X : ImGuiAxis_Y;

//         bool hovered, held;
//         ImRect border_rect = GetResizeBorderRect(window, border_n, grip_hover_inner_size, WINDOWS_HOVER_PADDING);
//         ImGuiID border_id = window->GetID(border_n + 4); // == GetWindowResizeBorderID()
//         ButtonBehavior(border_rect, border_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren);
//         // GetForegroundDrawLists(window)->AddRect(border_rect.Min, border_rect.Max, IM_COL32(255, 255, 0, 255));
//         if ((hovered && g.HoveredIdTimer > WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER) || held)
//         {
//             g.MouseCursor = (axis == ImGuiAxis_X) ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS;
//             if (held)
//                 *border_held = border_n;
//         }
//         if (held)
//         {
//             ImVec2 clamp_min(border_n == ImGuiDir_Right ? visibility_rect.Min.x : -FLT_MAX,
//                              border_n == ImGuiDir_Down ? visibility_rect.Min.y : -FLT_MAX);
//             ImVec2 clamp_max(border_n == ImGuiDir_Left ? visibility_rect.Max.x : +FLT_MAX,
//                              border_n == ImGuiDir_Up ? visibility_rect.Max.y : +FLT_MAX);
//             ImVec2 border_target = window->Pos;
//             border_target[axis] = g.IO.MousePos[axis] - g.ActiveIdClickOffset[axis] + WINDOWS_HOVER_PADDING;
//             border_target = ImClamp(border_target, clamp_min, clamp_max);
//             CalcResizePosSizeFromAnyCorner(window, border_target, ImMin(def.SegmentN1, def.SegmentN2), &pos_target,
//                                            &size_target);
//         }
//     }
//     PopID();

//     // Restore nav layer
//     window->DC.NavLayerCurrent = ImGuiNavLayer_Main;

//     // Navigation resize (keyboard/gamepad)
//     if (g.NavWindowingTarget && g.NavWindowingTarget->RootWindow == window)
//     {
//         ImVec2 nav_resize_delta;
//         if (g.NavInputSource == ImGuiInputSource_Keyboard && g.IO.KeyShift)
//             nav_resize_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard, ImGuiInputReadMode_Down);
//         if (g.NavInputSource == ImGuiInputSource_Gamepad)
//             nav_resize_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_PadDPad, ImGuiInputReadMode_Down);
//         if (nav_resize_delta.x != 0.0f || nav_resize_delta.y != 0.0f)
//         {
//             const float NAV_RESIZE_SPEED = 600.0f;
//             nav_resize_delta *= ImFloor(NAV_RESIZE_SPEED * g.IO.DeltaTime *
//                                         ImMin(g.IO.DisplayFramebufferScale.x, g.IO.DisplayFramebufferScale.y));
//             nav_resize_delta = ImMax(nav_resize_delta, visibility_rect.Min - window->Pos - window->Size);
//             g.NavWindowingToggleLayer = false;
//             g.NavDisableMouseHover = true;
//             resize_grip_col[0] = GetColorU32(ImGuiCol_ResizeGripActive);
//             // FIXME-NAV: Should store and accumulate into a separate size buffer to handle sizing constraints properly,
//             // right now a constraint will make us stuck.
//             size_target = CalcWindowSizeAfterConstraint(window, window->SizeFull + nav_resize_delta);
//         }
//     }

//     // Apply back modified position/size to window
//     if (size_target.x != FLT_MAX)
//     {
//         window->SizeFull = size_target;
//         MarkIniSettingsDirty(window);
//     }
//     if (pos_target.x != FLT_MAX)
//     {
//         window->Pos = ImFloor(pos_target);
//         MarkIniSettingsDirty(window);
//     }

//     window->Size = window->SizeFull;
//     return ret_auto_fit;
// }

// static void ImGui::RenderWindowOuterBorders(ImGuiWindow *window)
// {
//     ImGuiContext &g = *GImGui;
//     float rounding = window->WindowRounding;
//     float border_size = window->WindowBorderSize;
//     if (border_size > 0.0f && !(window->Flags & ImGuiWindowFlags_NoBackground))
//         window->DrawList->AddRect(window->Pos, window->Pos + window->Size, GetColorU32(ImGuiCol_Border), rounding, 0,
//                                   border_size);

//     int border_held = window->ResizeBorderHeld;
//     if (border_held != -1)
//     {
//         const ImGuiResizeBorderDef &def = resize_border_def[border_held];
//         ImRect border_r = GetResizeBorderRect(window, border_held, rounding, 0.0f);
//         window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN1) + ImVec2(0.5f, 0.5f) +
//                                         def.InnerDir * rounding,
//                                     rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
//         window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN2) + ImVec2(0.5f, 0.5f) +
//                                         def.InnerDir * rounding,
//                                     rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
//         window->DrawList->PathStroke(GetColorU32(ImGuiCol_SeparatorActive), 0,
//                                      ImMax(2.0f, border_size)); // Thicker than usual
//     }
//     if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar))
//     {
//         float y = window->Pos.y + window->TitleBarHeight() - 1;
//         window->DrawList->AddLine(ImVec2(window->Pos.x + border_size, y),
//                                   ImVec2(window->Pos.x + window->Size.x - border_size, y), GetColorU32(ImGuiCol_Border),
//                                   g.Style.FrameBorderSize);
//     }
// }

// static ImGuiCol GetWindowBgColorIdxFromFlags(ImGuiWindowFlags flags)
// {
//     if (flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup))
//         return ImGuiCol_PopupBg;
//     if (flags & ImGuiWindowFlags_ChildWindow)
//         return ImGuiCol_ChildBg;
//     return ImGuiCol_WindowBg;
// }

// // Draw background and borders
// // Draw and handle scrollbars
// void ImGui::RenderWindowDecorations(ImGuiWindow *window, const ImRect &title_bar_rect, bool title_bar_is_highlight,
//                                     int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size)
// {
//     ImGuiContext &g = *GImGui;
//     ImGuiStyle &style = g.Style;
//     ImGuiWindowFlags flags = window->Flags;

//     // Ensure that ScrollBar doesn't read last frame's SkipItems
//     IM_ASSERT(window->BeginCount == 0);
//     window->SkipItems = false;

//     // Draw window + handle manual resize
//     // As we highlight the title bar when want_focus is set, multiple reappearing windows will have have their title bar
//     // highlighted on their reappearing frame.
//     const float window_rounding = window->WindowRounding;
//     const float window_border_size = window->WindowBorderSize;
//     if (window->Collapsed)
//     {
//         // Title bar only
//         float backup_border_size = style.FrameBorderSize;
//         g.Style.FrameBorderSize = window->WindowBorderSize;
//         ImU32 title_bar_col = GetColorU32(
//             (title_bar_is_highlight && !g.NavDisableHighlight) ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBgCollapsed);
//         RenderFrame(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, true, window_rounding);
//         g.Style.FrameBorderSize = backup_border_size;
//     }
//     else
//     {
//         // Window background
//         if (!(flags & ImGuiWindowFlags_NoBackground))
//         {
//             ImU32 bg_col = GetColorU32(GetWindowBgColorIdxFromFlags(flags));
//             bool override_alpha = false;
//             float alpha = 1.0f;
//             if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasBgAlpha)
//             {
//                 alpha = g.NextWindowData.BgAlphaVal;
//                 override_alpha = true;
//             }
//             if (override_alpha)
//                 bg_col = (bg_col & ~IM_COL32_A_MASK) | (IM_F32_TO_INT8_SAT(alpha) << IM_COL32_A_SHIFT);
//             window->DrawList->AddRectFilled(window->Pos + ImVec2(0, window->TitleBarHeight()),
//                                             window->Pos + window->Size, bg_col, window_rounding,
//                                             (flags & ImGuiWindowFlags_NoTitleBar) ? 0 : ImDrawFlags_RoundCornersBottom);
//         }

//         // Title bar
//         if (!(flags & ImGuiWindowFlags_NoTitleBar))
//         {
//             ImU32 title_bar_col = GetColorU32(title_bar_is_highlight ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg);
//             window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, window_rounding,
//                                             ImDrawFlags_RoundCornersTop);
//         }

//         // Menu bar
//         if (flags & ImGuiWindowFlags_MenuBar)
//         {
//             ImRect menu_bar_rect = window->MenuBarRect();
//             menu_bar_rect.ClipWith(window->Rect()); // Soft clipping, in particular child window don't have minimum size
//                                                     // covering the menu bar so this is useful for them.
//             window->DrawList->AddRectFilled(
//                 menu_bar_rect.Min + ImVec2(window_border_size, 0), menu_bar_rect.Max - ImVec2(window_border_size, 0),
//                 GetColorU32(ImGuiCol_MenuBarBg), (flags & ImGuiWindowFlags_NoTitleBar) ? window_rounding : 0.0f,
//                 ImDrawFlags_RoundCornersTop);
//             if (style.FrameBorderSize > 0.0f && menu_bar_rect.Max.y < window->Pos.y + window->Size.y)
//                 window->DrawList->AddLine(menu_bar_rect.GetBL(), menu_bar_rect.GetBR(), GetColorU32(ImGuiCol_Border),
//                                           style.FrameBorderSize);
//         }

//         // Scrollbars
//         if (window->ScrollbarX)
//             Scrollbar(ImGuiAxis_X);
//         if (window->ScrollbarY)
//             Scrollbar(ImGuiAxis_Y);

//         // Render resize grips (after their input handling so we don't have a frame of latency)
//         if (!(flags & ImGuiWindowFlags_NoResize))
//         {
//             for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
//             {
//                 const ImGuiResizeGripDef &grip = resize_grip_def[resize_grip_n];
//                 const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, grip.CornerPosN);
//                 window->DrawList->PathLineTo(
//                     corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(window_border_size, resize_grip_draw_size)
//                                                                   : ImVec2(resize_grip_draw_size, window_border_size)));
//                 window->DrawList->PathLineTo(
//                     corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(resize_grip_draw_size, window_border_size)
//                                                                   : ImVec2(window_border_size, resize_grip_draw_size)));
//                 window->DrawList->PathArcToFast(
//                     ImVec2(corner.x + grip.InnerDir.x * (window_rounding + window_border_size),
//                            corner.y + grip.InnerDir.y * (window_rounding + window_border_size)),
//                     window_rounding, grip.AngleMin12, grip.AngleMax12);
//                 window->DrawList->PathFillConvex(resize_grip_col[resize_grip_n]);
//             }
//         }

//         // Borders
//         RenderWindowOuterBorders(window);
//     }
// }

// // Render title text, collapse button, close button
// void ImGui::RenderWindowTitleBarContents(ImGuiWindow *window, const ImRect &title_bar_rect, const char *name,
//                                          bool *p_open)
// {
//     ImGuiContext &g = *GImGui;
//     ImGuiStyle &style = g.Style;
//     ImGuiWindowFlags flags = window->Flags;

//     const bool has_close_button = (p_open != NULL);
//     const bool has_collapse_button =
//         !(flags & ImGuiWindowFlags_NoCollapse) && (style.WindowMenuButtonPosition != ImGuiDir_None);

//     // Close & Collapse button are on the Menu NavLayer and don't default focus (unless there's nothing else on that
//     // layer)
//     const ImGuiItemFlags item_flags_backup = g.CurrentItemFlags;
//     g.CurrentItemFlags |= ImGuiItemFlags_NoNavDefaultFocus;
//     window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

//     // Layout buttons
//     // FIXME: Would be nice to generalize the subtleties expressed here into reusable code.
//     float pad_l = style.FramePadding.x;
//     float pad_r = style.FramePadding.x;
//     float button_sz = g.FontSize;
//     ImVec2 close_button_pos;
//     ImVec2 collapse_button_pos;
//     if (has_close_button)
//     {
//         pad_r += button_sz;
//         close_button_pos = ImVec2(title_bar_rect.Max.x - pad_r - style.FramePadding.x, title_bar_rect.Min.y);
//     }
//     if (has_collapse_button && style.WindowMenuButtonPosition == ImGuiDir_Right)
//     {
//         pad_r += button_sz;
//         collapse_button_pos = ImVec2(title_bar_rect.Max.x - pad_r - style.FramePadding.x, title_bar_rect.Min.y);
//     }
//     if (has_collapse_button && style.WindowMenuButtonPosition == ImGuiDir_Left)
//     {
//         collapse_button_pos = ImVec2(title_bar_rect.Min.x + pad_l - style.FramePadding.x, title_bar_rect.Min.y);
//         pad_l += button_sz;
//     }

//     // Collapse button (submitting first so it gets priority when choosing a navigation init fallback)
//     if (has_collapse_button)
//         if (CollapseButton(window->GetID("#COLLAPSE"), collapse_button_pos))
//             window->WantCollapseToggle =
//                 true; // Defer actual collapsing to next frame as we are too far in the Begin() function

//     // Close button
//     if (has_close_button)
//         if (CloseButton(window->GetID("#CLOSE"), close_button_pos))
//             *p_open = false;

//     window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
//     g.CurrentItemFlags = item_flags_backup;

//     // Title bar text (with: horizontal alignment, avoiding collapse/close button, optional "unsaved document" marker)
//     // FIXME: Refactor text alignment facilities along with RenderText helpers, this is WAY too much messy code..
//     const char *UNSAVED_DOCUMENT_MARKER = "*";
//     const float marker_size_x =
//         (flags & ImGuiWindowFlags_UnsavedDocument) ? CalcTextSize(UNSAVED_DOCUMENT_MARKER, NULL, false).x : 0.0f;
//     const ImVec2 text_size = CalcTextSize(name, NULL, true) + ImVec2(marker_size_x, 0.0f);

//     // As a nice touch we try to ensure that centered title text doesn't get affected by visibility of Close/Collapse
//     // button, while uncentered title text will still reach edges correctly.
//     if (pad_l > style.FramePadding.x)
//         pad_l += g.Style.ItemInnerSpacing.x;
//     if (pad_r > style.FramePadding.x)
//         pad_r += g.Style.ItemInnerSpacing.x;
//     if (style.WindowTitleAlign.x > 0.0f && style.WindowTitleAlign.x < 1.0f)
//     {
//         float centerness =
//             ImSaturate(1.0f - ImFabs(style.WindowTitleAlign.x - 0.5f) * 2.0f); // 0.0f on either edges, 1.0f on center
//         float pad_extend = ImMin(ImMax(pad_l, pad_r), title_bar_rect.GetWidth() - pad_l - pad_r - text_size.x);
//         pad_l = ImMax(pad_l, pad_extend * centerness);
//         pad_r = ImMax(pad_r, pad_extend * centerness);
//     }

//     ImRect layout_r(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y, title_bar_rect.Max.x - pad_r,
//                     title_bar_rect.Max.y);
//     ImRect clip_r(layout_r.Min.x, layout_r.Min.y,
//                   ImMin(layout_r.Max.x + g.Style.ItemInnerSpacing.x, title_bar_rect.Max.x), layout_r.Max.y);
//     // if (g.IO.KeyShift) window->DrawList->AddRect(layout_r.Min, layout_r.Max, IM_COL32(255, 128, 0, 255)); // [DEBUG]
//     // if (g.IO.KeyCtrl) window->DrawList->AddRect(clip_r.Min, clip_r.Max, IM_COL32(255, 128, 0, 255)); // [DEBUG]
//     RenderTextClipped(layout_r.Min, layout_r.Max, name, NULL, &text_size, style.WindowTitleAlign, &clip_r);
//     if (flags & ImGuiWindowFlags_UnsavedDocument)
//     {
//         ImVec2 marker_pos = ImVec2(ImMax(layout_r.Min.x, layout_r.Min.x + (layout_r.GetWidth() - text_size.x) *
//                                                                               style.WindowTitleAlign.x) +
//                                        text_size.x,
//                                    layout_r.Min.y) +
//                             ImVec2(2 - marker_size_x, 0.0f);
//         ImVec2 off = ImVec2(0.0f, IM_FLOOR(-g.FontSize * 0.25f));
//         RenderTextClipped(marker_pos + off, layout_r.Max + off, UNSAVED_DOCUMENT_MARKER, NULL, NULL,
//                           ImVec2(0, style.WindowTitleAlign.y), &clip_r);
//     }
// }

// // Update viewports and monitor infos
// static void ImGui::UpdateViewportsNewFrame()
// {
//     ImGuiContext &g = *GImGui;
//     IM_ASSERT(g.Viewports.Size == 1);

//     // Update main viewport with current platform position.
//     // FIXME-VIEWPORT: Size is driven by backend/user code for backward-compatibility but we should aim to make this
//     // more consistent.
//     ImGuiViewportP *main_viewport = g.Viewports[0];
//     main_viewport->Flags = ImGuiViewportFlags_IsPlatformWindow | ImGuiViewportFlags_OwnedByApp;
//     main_viewport->Pos = ImVec2(0.0f, 0.0f);
//     main_viewport->Size = g.IO.DisplaySize;

//     for (int n = 0; n < g.Viewports.Size; n++)
//     {
//         ImGuiViewportP *viewport = g.Viewports[n];

//         // Lock down space taken by menu bars and status bars, reset the offset for fucntions like BeginMainMenuBar() to
//         // alter them again.
//         viewport->WorkOffsetMin = viewport->BuildWorkOffsetMin;
//         viewport->WorkOffsetMax = viewport->BuildWorkOffsetMax;
//         viewport->BuildWorkOffsetMin = viewport->BuildWorkOffsetMax = ImVec2(0.0f, 0.0f);
//         viewport->UpdateWorkRect();
//     }
// }

static void AddDrawListToDrawData(ImVector<ImDrawList *> *out_list, ImDrawList *draw_list)
{
    // Remove trailing command if unused.
    // Technically we could return directly instead of popping, but this make things looks neat in Metrics/Debugger
    // window as well.
    draw_list->_PopUnusedDrawCmd();
    if (draw_list->CmdBuffer.Size == 0)
        return;

    // Draw list sanity check. Detect mismatch between PrimReserve() calls and incrementing _VtxCurrentIdx, _VtxWritePtr
    // etc. May trigger for you if you are using PrimXXX functions incorrectly.
    IM_ASSERT(draw_list->VtxBuffer.Size == 0 ||
              draw_list->_VtxWritePtr == draw_list->VtxBuffer.Data + draw_list->VtxBuffer.Size);
    IM_ASSERT(draw_list->IdxBuffer.Size == 0 ||
              draw_list->_IdxWritePtr == draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size);
    if (!(draw_list->Flags & ImDrawListFlags_AllowVtxOffset))
        IM_ASSERT((int)draw_list->_VtxCurrentIdx == draw_list->VtxBuffer.Size);

    // Check that draw_list doesn't use more vertices than indexable (default ImDrawIdx = unsigned short = 2 bytes = 64K
    // vertices per ImDrawList = per window) If this assert triggers because you are drawing lots of stuff manually:
    // - First, make sure you are coarse clipping yourself and not trying to draw many things outside visible bounds.
    //   Be mindful that the ImDrawList API doesn't filter vertices. Use the Metrics/Debugger window to inspect draw
    //   list contents.
    // - If you want large meshes with more than 64K vertices, you can either:
    //   (A) Handle the ImDrawCmd::VtxOffset value in your renderer backend, and set 'io.BackendFlags |=
    //   ImGuiBackendFlags_RendererHasVtxOffset'.
    //       Most example backends already support this from 1.71. Pre-1.71 backends won't.
    //       Some graphics API such as GL ES 1/2 don't have a way to offset the starting vertex so it is not supported
    //       for them.
    //   (B) Or handle 32-bit indices in your renderer backend, and uncomment '#define ImDrawIdx unsigned int' line in
    //   imconfig.h.
    //       Most example backends already support this. For example, the OpenGL example code detect index size at
    //       compile-time:
    //         glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT :
    //         GL_UNSIGNED_INT, idx_buffer_offset);
    //       Your own engine or render API may use different parameters or function calls to specify index sizes.
    //       2 and 4 bytes indices are generally supported by most graphics API.
    // - If for some reason neither of those solutions works for you, a workaround is to call BeginChild()/EndChild()
    // before reaching
    //   the 64K limit to split your draw commands in multiple draw lists.
    if (sizeof(ImDrawIdx) == 2)
        IM_ASSERT(draw_list->_VtxCurrentIdx < (1 << 16) &&
                  "Too many vertices in ImDrawList using 16-bit indices. Read comment above");

    out_list->push_back(draw_list);
}

// FIXME: Add a more explicit sort order in the window structure.
static int IMGUI_CDECL ChildWindowComparer(const void *lhs, const void *rhs)
{
    const ImGuiWindow *const a = *(const ImGuiWindow *const *)lhs;
    const ImGuiWindow *const b = *(const ImGuiWindow *const *)rhs;
    if (int d = (a->Flags & ImGuiWindowFlags_Popup) - (b->Flags & ImGuiWindowFlags_Popup))
        return d;
    if (int d = (a->Flags & ImGuiWindowFlags_Tooltip) - (b->Flags & ImGuiWindowFlags_Tooltip))
        return d;
    return (a->BeginOrderWithinParent - b->BeginOrderWithinParent);
}

static void AddWindowToSortBuffer(ImVector<ImGuiWindow *> *out_sorted_windows, ImGuiWindow *window)
{
    out_sorted_windows->push_back(window);
    if (window->Active)
    {
        int count = window->DC.ChildWindows.Size;
        if (count > 1)
            ImQsort(window->DC.ChildWindows.Data, (size_t)count, sizeof(ImGuiWindow *), ChildWindowComparer);
        for (int i = 0; i < count; i++)
        {
            ImGuiWindow *child = window->DC.ChildWindows[i];
            if (child->Active)
                AddWindowToSortBuffer(out_sorted_windows, child);
        }
    }
}
