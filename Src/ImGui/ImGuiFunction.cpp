#include <imgui.h>
#include <ImGui/ImGuiInternal.h>
#include <ImGui/ImGuiFunction.h>
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
// Note that we still point to some static data and members (such as GFontAtlas), so the state instance you end up using will point to the static data within its module
ImGuiContext* ImGui::GetCurrentContext()
{
    return GImGui;
}

void ImGui::SetCurrentContext(ImGuiContext* ctx)
{
#ifdef IMGUI_SET_CURRENT_CONTEXT_FUNC
    IMGUI_SET_CURRENT_CONTEXT_FUNC(ctx); // For custom thread-based hackery you may want to have control over this.
#else
    GImGui = ctx;
#endif
}

ImGuiIO& ImGui::GetIO()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->IO;
}

ImGuiStyle& ImGui::GetStyle()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->Style;
}


// void ImGui::NewFrame()
// {
//     IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
//     ImGuiContext& g = *GImGui;

//     // Remove pending delete hooks before frame start.
//     // This deferred removal avoid issues of removal while iterating the hook vector
//     for (int n = g.Hooks.Size - 1; n >= 0; n--)
//         if (g.Hooks[n].Type == ImGuiContextHookType_PendingRemoval_)
//             g.Hooks.erase(&g.Hooks[n]);

//     CallContextHooks(&g, ImGuiContextHookType_NewFramePre);

//     // Check and assert for various common IO and Configuration mistakes
//     ErrorCheckNewFrameSanityChecks();

//     // Load settings on first frame, save settings when modified (after a delay)
//     UpdateSettings();

//     g.Time += g.IO.DeltaTime;
//     g.WithinFrameScope = true;
//     g.FrameCount += 1;
//     g.TooltipOverrideCount = 0;
//     g.WindowsActiveCount = 0;
//     g.MenusIdSubmittedThisFrame.resize(0);

//     // Calculate frame-rate for the user, as a purely luxurious feature
//     g.FramerateSecPerFrameAccum += g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
//     g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
//     g.FramerateSecPerFrameIdx = (g.FramerateSecPerFrameIdx + 1) % IM_ARRAYSIZE(g.FramerateSecPerFrame);
//     g.FramerateSecPerFrameCount = ImMin(g.FramerateSecPerFrameCount + 1, IM_ARRAYSIZE(g.FramerateSecPerFrame));
//     g.IO.Framerate = (g.FramerateSecPerFrameAccum > 0.0f) ? (1.0f / (g.FramerateSecPerFrameAccum / (float)g.FramerateSecPerFrameCount)) : FLT_MAX;

//     UpdateViewportsNewFrame();

//     // Setup current font and draw list shared data
//     g.IO.Fonts->Locked = true;
//     SetCurrentFont(GetDefaultFont());
//     IM_ASSERT(g.Font->IsLoaded());
//     ImRect virtual_space(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
//     for (int n = 0; n < g.Viewports.Size; n++)
//         virtual_space.Add(g.Viewports[n]->GetMainRect());
//     g.DrawListSharedData.ClipRectFullscreen = virtual_space.ToVec4();
//     g.DrawListSharedData.CurveTessellationTol = g.Style.CurveTessellationTol;
//     g.DrawListSharedData.SetCircleTessellationMaxError(g.Style.CircleTessellationMaxError);
//     g.DrawListSharedData.InitialFlags = ImDrawListFlags_None;
//     if (g.Style.AntiAliasedLines)
//         g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLines;
//     if (g.Style.AntiAliasedLinesUseTex && !(g.Font->ContainerAtlas->Flags & ImFontAtlasFlags_NoBakedLines))
//         g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLinesUseTex;
//     if (g.Style.AntiAliasedFill)
//         g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedFill;
//     if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)
//         g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AllowVtxOffset;

//     // Mark rendering data as invalid to prevent user who may have a handle on it to use it.
//     for (int n = 0; n < g.Viewports.Size; n++)
//     {
//         ImGuiViewportP* viewport = g.Viewports[n];
//         viewport->DrawDataP.Clear();
//     }

//     // Drag and drop keep the source ID alive so even if the source disappear our state is consistent
//     if (g.DragDropActive && g.DragDropPayload.SourceId == g.ActiveId)
//         KeepAliveID(g.DragDropPayload.SourceId);

//     // Update HoveredId data
//     if (!g.HoveredIdPreviousFrame)
//         g.HoveredIdTimer = 0.0f;
//     if (!g.HoveredIdPreviousFrame || (g.HoveredId && g.ActiveId == g.HoveredId))
//         g.HoveredIdNotActiveTimer = 0.0f;
//     if (g.HoveredId)
//         g.HoveredIdTimer += g.IO.DeltaTime;
//     if (g.HoveredId && g.ActiveId != g.HoveredId)
//         g.HoveredIdNotActiveTimer += g.IO.DeltaTime;
//     g.HoveredIdPreviousFrame = g.HoveredId;
//     g.HoveredIdPreviousFrameUsingMouseWheel = g.HoveredIdUsingMouseWheel;
//     g.HoveredId = 0;
//     g.HoveredIdAllowOverlap = false;
//     g.HoveredIdUsingMouseWheel = false;
//     g.HoveredIdDisabled = false;

//     // Update ActiveId data (clear reference to active widget if the widget isn't alive anymore)
//     if (g.ActiveIdIsAlive != g.ActiveId && g.ActiveIdPreviousFrame == g.ActiveId && g.ActiveId != 0)
//         ClearActiveID();
//     if (g.ActiveId)
//         g.ActiveIdTimer += g.IO.DeltaTime;
//     g.LastActiveIdTimer += g.IO.DeltaTime;
//     g.ActiveIdPreviousFrame = g.ActiveId;
//     g.ActiveIdPreviousFrameWindow = g.ActiveIdWindow;
//     g.ActiveIdPreviousFrameHasBeenEditedBefore = g.ActiveIdHasBeenEditedBefore;
//     g.ActiveIdIsAlive = 0;
//     g.ActiveIdHasBeenEditedThisFrame = false;
//     g.ActiveIdPreviousFrameIsAlive = false;
//     g.ActiveIdIsJustActivated = false;
//     if (g.TempInputId != 0 && g.ActiveId != g.TempInputId)
//         g.TempInputId = 0;
//     if (g.ActiveId == 0)
//     {
//         g.ActiveIdUsingNavDirMask = 0x00;
//         g.ActiveIdUsingNavInputMask = 0x00;
//         g.ActiveIdUsingKeyInputMask = 0x00;
//     }

//     // Drag and drop
//     g.DragDropAcceptIdPrev = g.DragDropAcceptIdCurr;
//     g.DragDropAcceptIdCurr = 0;
//     g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
//     g.DragDropWithinSource = false;
//     g.DragDropWithinTarget = false;
//     g.DragDropHoldJustPressedId = 0;

//     // Update keyboard input state
//     // Synchronize io.KeyMods with individual modifiers io.KeyXXX bools
//     g.IO.KeyMods = GetMergedKeyModFlags();
//     memcpy(g.IO.KeysDownDurationPrev, g.IO.KeysDownDuration, sizeof(g.IO.KeysDownDuration));
//     for (int i = 0; i < IM_ARRAYSIZE(g.IO.KeysDown); i++)
//         g.IO.KeysDownDuration[i] = g.IO.KeysDown[i] ? (g.IO.KeysDownDuration[i] < 0.0f ? 0.0f : g.IO.KeysDownDuration[i] + g.IO.DeltaTime) : -1.0f;

//     // Update gamepad/keyboard navigation
//     NavUpdate();

//     // Update mouse input state
//     UpdateMouseInputs();

//     // Find hovered window
//     // (needs to be before UpdateMouseMovingWindowNewFrame so we fill g.HoveredWindowUnderMovingWindow on the mouse release frame)
//     UpdateHoveredWindowAndCaptureFlags();

//     // Handle user moving window with mouse (at the beginning of the frame to avoid input lag or sheering)
//     UpdateMouseMovingWindowNewFrame();

//     // Background darkening/whitening
//     if (GetTopMostPopupModal() != NULL || (g.NavWindowingTarget != NULL && g.NavWindowingHighlightAlpha > 0.0f))
//         g.DimBgRatio = ImMin(g.DimBgRatio + g.IO.DeltaTime * 6.0f, 1.0f);
//     else
//         g.DimBgRatio = ImMax(g.DimBgRatio - g.IO.DeltaTime * 10.0f, 0.0f);

//     g.MouseCursor = ImGuiMouseCursor_Arrow;
//     g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame = g.WantTextInputNextFrame = -1;
//     g.PlatformImePos = ImVec2(1.0f, 1.0f); // OS Input Method Editor showing on top-left of our window by default

//     // Mouse wheel scrolling, scale
//     UpdateMouseWheel();

//     // Update legacy TAB focus
//     UpdateTabFocus();

//     // Mark all windows as not visible and compact unused memory.
//     IM_ASSERT(g.WindowsFocusOrder.Size <= g.Windows.Size);
//     const float memory_compact_start_time = (g.GcCompactAll || g.IO.ConfigMemoryCompactTimer < 0.0f) ? FLT_MAX : (float)g.Time - g.IO.ConfigMemoryCompactTimer;
//     for (int i = 0; i != g.Windows.Size; i++)
//     {
//         ImGuiWindow* window = g.Windows[i];
//         window->WasActive = window->Active;
//         window->BeginCount = 0;
//         window->Active = false;
//         window->WriteAccessed = false;

//         // Garbage collect transient buffers of recently unused windows
//         if (!window->WasActive && !window->MemoryCompacted && window->LastTimeActive < memory_compact_start_time)
//             GcCompactTransientWindowBuffers(window);
//     }

//     // Garbage collect transient buffers of recently unused tables
//     for (int i = 0; i < g.TablesLastTimeActive.Size; i++)
//         if (g.TablesLastTimeActive[i] >= 0.0f && g.TablesLastTimeActive[i] < memory_compact_start_time)
//             TableGcCompactTransientBuffers(g.Tables.GetByIndex(i));
//     for (int i = 0; i < g.TablesTempDataStack.Size; i++)
//         if (g.TablesTempDataStack[i].LastTimeActive >= 0.0f && g.TablesTempDataStack[i].LastTimeActive < memory_compact_start_time)
//             TableGcCompactTransientBuffers(&g.TablesTempDataStack[i]);
//     if (g.GcCompactAll)
//         GcCompactTransientMiscBuffers();
//     g.GcCompactAll = false;

//     // Closing the focused window restore focus to the first active root window in descending z-order
//     if (g.NavWindow && !g.NavWindow->WasActive)
//         FocusTopMostWindowUnderOne(NULL, NULL);

//     // No window should be open at the beginning of the frame.
//     // But in order to allow the user to call NewFrame() multiple times without calling Render(), we are doing an explicit clear.
//     g.CurrentWindowStack.resize(0);
//     g.BeginPopupStack.resize(0);
//     g.ItemFlagsStack.resize(0);
//     g.ItemFlagsStack.push_back(ImGuiItemFlags_None);
//     g.GroupStack.resize(0);
//     ClosePopupsOverWindow(g.NavWindow, false);

//     // [DEBUG] Item picker tool - start with DebugStartItemPicker() - useful to visually select an item and break into its call-stack.
//     UpdateDebugToolItemPicker();

//     // Create implicit/fallback window - which we will only render it if the user has added something to it.
//     // We don't use "Debug" to avoid colliding with user trying to create a "Debug" window with custom flags.
//     // This fallback is particularly important as it avoid ImGui:: calls from crashing.
//     g.WithinFrameScopeWithImplicitWindow = true;
//     SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
//     Begin("Debug##Default");
//     IM_ASSERT(g.CurrentWindow->IsFallbackWindow == true);

//     CallContextHooks(&g, ImGuiContextHookType_NewFramePost);
// }

// // This is normally called by Render(). You may want to call it directly if you want to avoid calling Render() but the gain will be very minimal.
// void ImGui::EndFrame()
// {
//     ImGuiContext& g = *GImGui;
//     IM_ASSERT(g.Initialized);

//     // Don't process EndFrame() multiple times.
//     if (g.FrameCountEnded == g.FrameCount)
//         return;
//     IM_ASSERT(g.WithinFrameScope && "Forgot to call ImGui::NewFrame()?");

//     CallContextHooks(&g, ImGuiContextHookType_EndFramePre);

//     ErrorCheckEndFrameSanityChecks();

//     // Notify OS when our Input Method Editor cursor has moved (e.g. CJK inputs using Microsoft IME)
//     if (g.IO.ImeSetInputScreenPosFn && (g.PlatformImeLastPos.x == FLT_MAX || ImLengthSqr(g.PlatformImeLastPos - g.PlatformImePos) > 0.0001f))
//     {
//         g.IO.ImeSetInputScreenPosFn((int)g.PlatformImePos.x, (int)g.PlatformImePos.y);
//         g.PlatformImeLastPos = g.PlatformImePos;
//     }

//     // Hide implicit/fallback "Debug" window if it hasn't been used
//     g.WithinFrameScopeWithImplicitWindow = false;
//     if (g.CurrentWindow && !g.CurrentWindow->WriteAccessed)
//         g.CurrentWindow->Active = false;
//     End();

//     // Update navigation: CTRL+Tab, wrap-around requests
//     NavEndFrame();

//     // Drag and Drop: Elapse payload (if delivered, or if source stops being submitted)
//     if (g.DragDropActive)
//     {
//         bool is_delivered = g.DragDropPayload.Delivery;
//         bool is_elapsed = (g.DragDropPayload.DataFrameCount + 1 < g.FrameCount) && ((g.DragDropSourceFlags & ImGuiDragDropFlags_SourceAutoExpirePayload) || !IsMouseDown(g.DragDropMouseButton));
//         if (is_delivered || is_elapsed)
//             ClearDragDrop();
//     }

//     // Drag and Drop: Fallback for source tooltip. This is not ideal but better than nothing.
//     if (g.DragDropActive && g.DragDropSourceFrameCount < g.FrameCount && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
//     {
//         g.DragDropWithinSource = true;
//         SetTooltip("...");
//         g.DragDropWithinSource = false;
//     }

//     // End frame
//     g.WithinFrameScope = false;
//     g.FrameCountEnded = g.FrameCount;

//     // Initiate moving window + handle left-click and right-click focus
//     UpdateMouseMovingWindowEndFrame();

//     // Sort the window list so that all child windows are after their parent
//     // We cannot do that on FocusWindow() because children may not exist yet
//     g.WindowsTempSortBuffer.resize(0);
//     g.WindowsTempSortBuffer.reserve(g.Windows.Size);
//     for (int i = 0; i != g.Windows.Size; i++)
//     {
//         ImGuiWindow* window = g.Windows[i];
//         if (window->Active && (window->Flags & ImGuiWindowFlags_ChildWindow))       // if a child is active its parent will add it
//             continue;
//         AddWindowToSortBuffer(&g.WindowsTempSortBuffer, window);
//     }

//     // This usually assert if there is a mismatch between the ImGuiWindowFlags_ChildWindow / ParentWindow values and DC.ChildWindows[] in parents, aka we've done something wrong.
//     IM_ASSERT(g.Windows.Size == g.WindowsTempSortBuffer.Size);
//     g.Windows.swap(g.WindowsTempSortBuffer);
//     g.IO.MetricsActiveWindows = g.WindowsActiveCount;

//     // Unlock font atlas
//     g.IO.Fonts->Locked = false;

//     // Clear Input data for next frame
//     g.IO.MouseWheel = g.IO.MouseWheelH = 0.0f;
//     g.IO.InputQueueCharacters.resize(0);
//     memset(g.IO.NavInputs, 0, sizeof(g.IO.NavInputs));

//     CallContextHooks(&g, ImGuiContextHookType_EndFramePost);
// }


// void ImGui::Render()
// {
//     ImGuiContext& g = *GImGui;
//     IM_ASSERT(g.Initialized);

//     if (g.FrameCountEnded != g.FrameCount)
//         EndFrame();
//     g.FrameCountRendered = g.FrameCount;
//     g.IO.MetricsRenderWindows = 0;

//     CallContextHooks(&g, ImGuiContextHookType_RenderPre);

//     // Add background ImDrawList (for each active viewport)
//     for (int n = 0; n != g.Viewports.Size; n++)
//     {
//         ImGuiViewportP* viewport = g.Viewports[n];
//         viewport->DrawDataBuilder.Clear();
//         if (viewport->DrawLists[0] != NULL)
//             AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], GetBackgroundDrawList(viewport));
//     }

//     // Add ImDrawList to render
//     ImGuiWindow* windows_to_render_top_most[2];
//     windows_to_render_top_most[0] = (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) ? g.NavWindowingTarget->RootWindow : NULL;
//     windows_to_render_top_most[1] = (g.NavWindowingTarget ? g.NavWindowingListWindow : NULL);
//     for (int n = 0; n != g.Windows.Size; n++)
//     {
//         ImGuiWindow* window = g.Windows[n];
//         IM_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
//         if (IsWindowActiveAndVisible(window) && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0 && window != windows_to_render_top_most[0] && window != windows_to_render_top_most[1])
//             AddRootWindowToDrawData(window);
//     }
//     for (int n = 0; n < IM_ARRAYSIZE(windows_to_render_top_most); n++)
//         if (windows_to_render_top_most[n] && IsWindowActiveAndVisible(windows_to_render_top_most[n])) // NavWindowingTarget is always temporarily displayed as the top-most window
//             AddRootWindowToDrawData(windows_to_render_top_most[n]);

//     // Setup ImDrawData structures for end-user
//     g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = 0;
//     for (int n = 0; n < g.Viewports.Size; n++)
//     {
//         ImGuiViewportP* viewport = g.Viewports[n];
//         viewport->DrawDataBuilder.FlattenIntoSingleLayer();

//         // Draw software mouse cursor if requested by io.MouseDrawCursor flag
//         if (g.IO.MouseDrawCursor)
//             RenderMouseCursor(GetForegroundDrawList(viewport), g.IO.MousePos, g.Style.MouseCursorScale, g.MouseCursor, IM_COL32_WHITE, IM_COL32_BLACK, IM_COL32(0, 0, 0, 48));

//         // Add foreground ImDrawList (for each active viewport)
//         if (viewport->DrawLists[1] != NULL)
//             AddDrawListToDrawData(&viewport->DrawDataBuilder.Layers[0], GetForegroundDrawList(viewport));

//         SetupViewportDrawData(viewport, &viewport->DrawDataBuilder.Layers[0]);
//         ImDrawData* draw_data = &viewport->DrawDataP;
//         g.IO.MetricsRenderVertices += draw_data->TotalVtxCount;
//         g.IO.MetricsRenderIndices += draw_data->TotalIdxCount;
//     }

//     CallContextHooks(&g, ImGuiContextHookType_RenderPost);
// }


