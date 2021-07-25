#pragma once

#include <ImGui/ImGuiType.h>
#include <ImGui/ImGuiStruct.h>
#include <ImGui/ImGuiIO.h>
#include <ImGui/ImGuiStyle.h>
#include <ImGui/ImGuiFont.h>

#include <ImGui/Widget/ImGuiViewport.h>

//-----------------------------------------------------------------------------
// [SECTION] Generic context hooks
//-----------------------------------------------------------------------------

typedef void (*ImGuiContextHookCallback)(ImGuiContext* ctx, ImGuiContextHook* hook);
enum ImGuiContextHookType { ImGuiContextHookType_NewFramePre, ImGuiContextHookType_NewFramePost, ImGuiContextHookType_EndFramePre, ImGuiContextHookType_EndFramePost, ImGuiContextHookType_RenderPre, ImGuiContextHookType_RenderPost, ImGuiContextHookType_Shutdown, ImGuiContextHookType_PendingRemoval_ };

struct ImGuiContextHook
{
    ImGuiID                     HookId;     // A unique ID assigned by AddContextHook()
    ImGuiContextHookType        Type;
    ImGuiID                     Owner;
    ImGuiContextHookCallback    Callback;
    void*                       UserData;

    ImGuiContextHook()          { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] ImGuiContext (main imgui context)
//-----------------------------------------------------------------------------

struct ImGuiContext
{
    bool                    Initialized;
    bool                    FontAtlasOwnedByContext;            // IO.Fonts-> is owned by the ImGuiContext and will be destructed along with it.
    ImGuiIO                 IO;
    ImGuiStyle              Style;
    ImFont*                 Font;                               // (Shortcut) == FontStack.empty() ? IO.Font : FontStack.back()
    float                   FontSize;                           // (Shortcut) == FontBaseSize * g.CurrentWindow->FontWindowScale == window->FontSize(). Text height for current window.
    float                   FontBaseSize;                       // (Shortcut) == IO.FontGlobalScale * Font->Scale * Font->FontSize. Base text height.
    ImDrawListSharedData    DrawListSharedData;
    double                  Time;
    int                     FrameCount;
    int                     FrameCountEnded;
    int                     FrameCountRendered;
    bool                    WithinFrameScope;                   // Set by NewFrame(), cleared by EndFrame()
    bool                    WithinFrameScopeWithImplicitWindow; // Set by NewFrame(), cleared by EndFrame() when the implicit debug window has been pushed
    bool                    WithinEndChild;                     // Set within EndChild()
    bool                    GcCompactAll;                       // Request full GC
    bool                    TestEngineHookItems;                // Will call test engine hooks: ImGuiTestEngineHook_ItemAdd(), ImGuiTestEngineHook_ItemInfo(), ImGuiTestEngineHook_Log()
    ImGuiID                 TestEngineHookIdInfo;               // Will call test engine hooks: ImGuiTestEngineHook_IdInfo() from GetID()
    void*                   TestEngine;                         // Test engine user data

    // Windows state
    ImVector<ImGuiWindow*>  Windows;                            // Windows, sorted in display order, back to front
    ImVector<ImGuiWindow*>  WindowsFocusOrder;                  // Root windows, sorted in focus order, back to front.
    ImVector<ImGuiWindow*>  WindowsTempSortBuffer;              // Temporary buffer used in EndFrame() to reorder windows so parents are kept before their child
    ImVector<ImGuiWindow*>  CurrentWindowStack;
    ImGuiStorage            WindowsById;                        // Map window's ImGuiID to ImGuiWindow*
    int                     WindowsActiveCount;                 // Number of unique windows submitted by frame
    ImVec2                  WindowsHoverPadding;                // Padding around resizable windows for which hovering on counts as hovering the window == ImMax(style.TouchExtraPadding, WINDOWS_HOVER_PADDING)
    ImGuiWindow*            CurrentWindow;                      // Window being drawn into
    ImGuiWindow*            HoveredWindow;                      // Window the mouse is hovering. Will typically catch mouse inputs.
    ImGuiWindow*            HoveredWindowUnderMovingWindow;     // Hovered window ignoring MovingWindow. Only set if MovingWindow is set.
    ImGuiWindow*            MovingWindow;                       // Track the window we clicked on (in order to preserve focus). The actual window that is moved is generally MovingWindow->RootWindow.
    ImGuiWindow*            WheelingWindow;                     // Track the window we started mouse-wheeling on. Until a timer elapse or mouse has moved, generally keep scrolling the same window even if during the course of scrolling the mouse ends up hovering a child window.
    ImVec2                  WheelingWindowRefMousePos;
    float                   WheelingWindowTimer;

    // Item/widgets state and tracking information
    ImGuiItemFlags          CurrentItemFlags;                   // == g.ItemFlagsStack.back()
    ImGuiID                 HoveredId;                          // Hovered widget, filled during the frame
    ImGuiID                 HoveredIdPreviousFrame;
    bool                    HoveredIdAllowOverlap;
    bool                    HoveredIdUsingMouseWheel;           // Hovered widget will use mouse wheel. Blocks scrolling the underlying window.
    bool                    HoveredIdPreviousFrameUsingMouseWheel;
    bool                    HoveredIdDisabled;                  // At least one widget passed the rect test, but has been discarded by disabled flag or popup inhibit. May be true even if HoveredId == 0.
    float                   HoveredIdTimer;                     // Measure contiguous hovering time
    float                   HoveredIdNotActiveTimer;            // Measure contiguous hovering time where the item has not been active
    ImGuiID                 ActiveId;                           // Active widget
    ImGuiID                 ActiveIdIsAlive;                    // Active widget has been seen this frame (we can't use a bool as the ActiveId may change within the frame)
    float                   ActiveIdTimer;
    bool                    ActiveIdIsJustActivated;            // Set at the time of activation for one frame
    bool                    ActiveIdAllowOverlap;               // Active widget allows another widget to steal active id (generally for overlapping widgets, but not always)
    bool                    ActiveIdNoClearOnFocusLoss;         // Disable losing active id if the active id window gets unfocused.
    bool                    ActiveIdHasBeenPressedBefore;       // Track whether the active id led to a press (this is to allow changing between PressOnClick and PressOnRelease without pressing twice). Used by range_select branch.
    bool                    ActiveIdHasBeenEditedBefore;        // Was the value associated to the widget Edited over the course of the Active state.
    bool                    ActiveIdHasBeenEditedThisFrame;
    bool                    ActiveIdUsingMouseWheel;            // Active widget will want to read mouse wheel. Blocks scrolling the underlying window.
    ImU32                   ActiveIdUsingNavDirMask;            // Active widget will want to read those nav move requests (e.g. can activate a button and move away from it)
    ImU32                   ActiveIdUsingNavInputMask;          // Active widget will want to read those nav inputs.
    ImU64                   ActiveIdUsingKeyInputMask;          // Active widget will want to read those key inputs. When we grow the ImGuiKey enum we'll need to either to order the enum to make useful keys come first, either redesign this into e.g. a small array.
    ImVec2                  ActiveIdClickOffset;                // Clicked offset from upper-left corner, if applicable (currently only set by ButtonBehavior)
    ImGuiWindow*            ActiveIdWindow;
    ImGuiInputSource        ActiveIdSource;                     // Activating with mouse or nav (gamepad/keyboard)
    int                     ActiveIdMouseButton;
    ImGuiID                 ActiveIdPreviousFrame;
    bool                    ActiveIdPreviousFrameIsAlive;
    bool                    ActiveIdPreviousFrameHasBeenEditedBefore;
    ImGuiWindow*            ActiveIdPreviousFrameWindow;
    ImGuiID                 LastActiveId;                       // Store the last non-zero ActiveId, useful for animation.
    float                   LastActiveIdTimer;                  // Store the last non-zero ActiveId timer since the beginning of activation, useful for animation.

    // Next window/item data
    ImGuiNextWindowData     NextWindowData;                     // Storage for SetNextWindow** functions
    ImGuiNextItemData       NextItemData;                       // Storage for SetNextItem** functions

    // Shared stacks
    ImVector<ImGuiColorMod> ColorStack;                         // Stack for PushStyleColor()/PopStyleColor() - inherited by Begin()
    ImVector<ImGuiStyleMod> StyleVarStack;                      // Stack for PushStyleVar()/PopStyleVar() - inherited by Begin()
    ImVector<ImFont*>       FontStack;                          // Stack for PushFont()/PopFont() - inherited by Begin()
    ImVector<ImGuiID>       FocusScopeStack;                    // Stack for PushFocusScope()/PopFocusScope() - not inherited by Begin(), unless child window
    ImVector<ImGuiItemFlags>ItemFlagsStack;                     // Stack for PushItemFlag()/PopItemFlag() - inherited by Begin()
    ImVector<ImGuiGroupData>GroupStack;                         // Stack for BeginGroup()/EndGroup() - not inherited by Begin()
    ImVector<ImGuiPopupData>OpenPopupStack;                     // Which popups are open (persistent)
    ImVector<ImGuiPopupData>BeginPopupStack;                    // Which level of BeginPopup() we are in (reset every frame)

    // Viewports
    ImVector<ImGuiViewportP*> Viewports;                        // Active viewports (Size==1 in 'master' branch). Each viewports hold their copy of ImDrawData.

    // Gamepad/keyboard Navigation
    ImGuiWindow*            NavWindow;                          // Focused window for navigation. Could be called 'FocusWindow'
    ImGuiID                 NavId;                              // Focused item for navigation
    ImGuiID                 NavFocusScopeId;                    // Identify a selection scope (selection code often wants to "clear other items" when landing on an item of the selection set)
    ImGuiID                 NavActivateId;                      // ~~ (g.ActiveId == 0) && IsNavInputPressed(ImGuiNavInput_Activate) ? NavId : 0, also set when calling ActivateItem()
    ImGuiID                 NavActivateDownId;                  // ~~ IsNavInputDown(ImGuiNavInput_Activate) ? NavId : 0
    ImGuiID                 NavActivatePressedId;               // ~~ IsNavInputPressed(ImGuiNavInput_Activate) ? NavId : 0
    ImGuiID                 NavInputId;                         // ~~ IsNavInputPressed(ImGuiNavInput_Input) ? NavId : 0
    ImGuiID                 NavJustTabbedId;                    // Just tabbed to this id.
    ImGuiID                 NavJustMovedToId;                   // Just navigated to this id (result of a successfully MoveRequest).
    ImGuiID                 NavJustMovedToFocusScopeId;         // Just navigated to this focus scope id (result of a successfully MoveRequest).
    ImGuiKeyModFlags        NavJustMovedToKeyMods;
    ImGuiID                 NavNextActivateId;                  // Set by ActivateItem(), queued until next frame.
    ImGuiInputSource        NavInputSource;                     // Keyboard or Gamepad mode? THIS WILL ONLY BE None or NavGamepad or NavKeyboard.
    ImRect                  NavScoringRect;                     // Rectangle used for scoring, in screen space. Based of window->NavRectRel[], modified for directional navigation scoring.
    int                     NavScoringCount;                    // Metrics for debugging
    ImGuiNavLayer           NavLayer;                           // Layer we are navigating on. For now the system is hard-coded for 0=main contents and 1=menu/title bar, may expose layers later.
    int                     NavIdTabCounter;                    // == NavWindow->DC.FocusIdxTabCounter at time of NavId processing
    bool                    NavIdIsAlive;                       // Nav widget has been seen this frame ~~ NavRectRel is valid
    bool                    NavMousePosDirty;                   // When set we will update mouse position if (io.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos) if set (NB: this not enabled by default)
    bool                    NavDisableHighlight;                // When user starts using mouse, we hide gamepad/keyboard highlight (NB: but they are still available, which is why NavDisableHighlight isn't always != NavDisableMouseHover)
    bool                    NavDisableMouseHover;               // When user starts using gamepad/keyboard, we hide mouse hovering highlight until mouse is touched again.
    bool                    NavAnyRequest;                      // ~~ NavMoveRequest || NavInitRequest
    bool                    NavInitRequest;                     // Init request for appearing window to select first item
    bool                    NavInitRequestFromMove;
    ImGuiID                 NavInitResultId;                    // Init request result (first item of the window, or one for which SetItemDefaultFocus() was called)
    ImRect                  NavInitResultRectRel;               // Init request result rectangle (relative to parent window)
    bool                    NavMoveRequest;                     // Move request for this frame
    ImGuiNavMoveFlags       NavMoveRequestFlags;
    ImGuiNavForward         NavMoveRequestForward;              // None / ForwardQueued / ForwardActive (this is used to navigate sibling parent menus from a child menu)
    ImGuiKeyModFlags        NavMoveRequestKeyMods;
    ImGuiDir                NavMoveDir, NavMoveDirLast;         // Direction of the move request (left/right/up/down), direction of the previous move request
    ImGuiDir                NavMoveClipDir;                     // FIXME-NAV: Describe the purpose of this better. Might want to rename?
    ImGuiNavItemData        NavMoveResultLocal;                 // Best move request candidate within NavWindow
    ImGuiNavItemData        NavMoveResultLocalVisibleSet;       // Best move request candidate within NavWindow that are mostly visible (when using ImGuiNavMoveFlags_AlsoScoreVisibleSet flag)
    ImGuiNavItemData        NavMoveResultOther;                 // Best move request candidate within NavWindow's flattened hierarchy (when using ImGuiWindowFlags_NavFlattened flag)
    ImGuiWindow*            NavWrapRequestWindow;               // Window which requested trying nav wrap-around.
    ImGuiNavMoveFlags       NavWrapRequestFlags;                // Wrap-around operation flags.

    // Navigation: Windowing (CTRL+TAB for list, or Menu button + keys or directional pads to move/resize)
    ImGuiWindow*            NavWindowingTarget;                 // Target window when doing CTRL+Tab (or Pad Menu + FocusPrev/Next), this window is temporarily displayed top-most!
    ImGuiWindow*            NavWindowingTargetAnim;             // Record of last valid NavWindowingTarget until DimBgRatio and NavWindowingHighlightAlpha becomes 0.0f, so the fade-out can stay on it.
    ImGuiWindow*            NavWindowingListWindow;             // Internal window actually listing the CTRL+Tab contents
    float                   NavWindowingTimer;
    float                   NavWindowingHighlightAlpha;
    bool                    NavWindowingToggleLayer;

    // Legacy Focus/Tabbing system (older than Nav, active even if Nav is disabled, misnamed. FIXME-NAV: This needs a redesign!)
    ImGuiWindow*            TabFocusRequestCurrWindow;          //
    ImGuiWindow*            TabFocusRequestNextWindow;          //
    int                     TabFocusRequestCurrCounterRegular;  // Any item being requested for focus, stored as an index (we on layout to be stable between the frame pressing TAB and the next frame, semi-ouch)
    int                     TabFocusRequestCurrCounterTabStop;  // Tab item being requested for focus, stored as an index
    int                     TabFocusRequestNextCounterRegular;  // Stored for next frame
    int                     TabFocusRequestNextCounterTabStop;  // "
    bool                    TabFocusPressed;                    // Set in NewFrame() when user pressed Tab

    // Render
    float                   DimBgRatio;                         // 0.0..1.0 animation when fading in a dimming background (for modal window and CTRL+TAB list)
    ImGuiMouseCursor        MouseCursor;

    // Drag and Drop
    bool                    DragDropActive;
    bool                    DragDropWithinSource;               // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag source.
    bool                    DragDropWithinTarget;               // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag target.
    ImGuiDragDropFlags      DragDropSourceFlags;
    int                     DragDropSourceFrameCount;
    int                     DragDropMouseButton;
    ImGuiPayload            DragDropPayload;
    ImRect                  DragDropTargetRect;                 // Store rectangle of current target candidate (we favor small targets when overlapping)
    ImGuiID                 DragDropTargetId;
    ImGuiDragDropFlags      DragDropAcceptFlags;
    float                   DragDropAcceptIdCurrRectSurface;    // Target item surface (we resolve overlapping targets by prioritizing the smaller surface)
    ImGuiID                 DragDropAcceptIdCurr;               // Target item id (set at the time of accepting the payload)
    ImGuiID                 DragDropAcceptIdPrev;               // Target item id from previous frame (we need to store this to allow for overlapping drag and drop targets)
    int                     DragDropAcceptFrameCount;           // Last time a target expressed a desire to accept the source
    ImGuiID                 DragDropHoldJustPressedId;          // Set when holding a payload just made ButtonBehavior() return a press.
    ImVector<unsigned char> DragDropPayloadBufHeap;             // We don't expose the ImVector<> directly, ImGuiPayload only holds pointer+size
    unsigned char           DragDropPayloadBufLocal[16];        // Local buffer for small payloads

    // Table
    ImGuiTable*                     CurrentTable;
    int                             CurrentTableStackIdx;
    ImPool<ImGuiTable>              Tables;
    ImVector<ImGuiTableTempData>    TablesTempDataStack;
    ImVector<float>                 TablesLastTimeActive;       // Last used timestamp of each tables (SOA, for efficient GC)
    ImVector<ImDrawChannel>         DrawChannelsTempMergeBuffer;

    // Tab bars
    ImGuiTabBar*                    CurrentTabBar;
    ImPool<ImGuiTabBar>             TabBars;
    ImVector<ImGuiPtrOrIndex>       CurrentTabBarStack;
    ImVector<ImGuiShrinkWidthItem>  ShrinkWidthBuffer;

    // Widget state
    ImVec2                  LastValidMousePos;
    ImGuiInputTextState     InputTextState;
    ImFont                  InputTextPasswordFont;
    ImGuiID                 TempInputId;                        // Temporary text input when CTRL+clicking on a slider, etc.
    ImGuiColorEditFlags     ColorEditOptions;                   // Store user options for color edit widgets
    float                   ColorEditLastHue;                   // Backup of last Hue associated to LastColor[3], so we can restore Hue in lossy RGB<>HSV round trips
    float                   ColorEditLastSat;                   // Backup of last Saturation associated to LastColor[3], so we can restore Saturation in lossy RGB<>HSV round trips
    float                   ColorEditLastColor[3];
    ImVec4                  ColorPickerRef;                     // Initial/reference color at the time of opening the color picker.
    float                   SliderCurrentAccum;                 // Accumulated slider delta when using navigation controls.
    bool                    SliderCurrentAccumDirty;            // Has the accumulated slider delta changed since last time we tried to apply it?
    bool                    DragCurrentAccumDirty;
    float                   DragCurrentAccum;                   // Accumulator for dragging modification. Always high-precision, not rounded by end-user precision settings
    float                   DragSpeedDefaultRatio;              // If speed == 0.0f, uses (max-min) * DragSpeedDefaultRatio
    float                   ScrollbarClickDeltaToGrabCenter;    // Distance between mouse and center of grab box, normalized in parent space. Use storage?
    int                     TooltipOverrideCount;
    float                   TooltipSlowDelay;                   // Time before slow tooltips appears (FIXME: This is temporary until we merge in tooltip timer+priority work)
    ImVector<char>          ClipboardHandlerData;               // If no custom clipboard handler is defined
    ImVector<ImGuiID>       MenusIdSubmittedThisFrame;          // A list of menu IDs that were rendered at least once

    // Platform support
    ImVec2                  PlatformImePos;                     // Cursor position request & last passed to the OS Input Method Editor
    ImVec2                  PlatformImeLastPos;
    char                    PlatformLocaleDecimalPoint;         // '.' or *localeconv()->decimal_point

    // Settings
    bool                    SettingsLoaded;
    float                   SettingsDirtyTimer;                 // Save .ini Settings to memory when time reaches zero
    ImGuiTextBuffer         SettingsIniData;                    // In memory .ini settings
    ImVector<ImGuiSettingsHandler>      SettingsHandlers;       // List of .ini settings handlers
    ImChunkStream<ImGuiWindowSettings>  SettingsWindows;        // ImGuiWindow .ini settings entries
    ImChunkStream<ImGuiTableSettings>   SettingsTables;         // ImGuiTable .ini settings entries
    ImVector<ImGuiContextHook>          Hooks;                  // Hooks for extensions (e.g. test engine)
    ImGuiID                             HookIdNext;             // Next available HookId

    // Capture/Logging
    bool                    LogEnabled;                         // Currently capturing
    ImGuiLogType            LogType;                            // Capture target
    ImFileHandle            LogFile;                            // If != NULL log to stdout/ file
    ImGuiTextBuffer         LogBuffer;                          // Accumulation buffer when log to clipboard. This is pointer so our GImGui static constructor doesn't call heap allocators.
    const char*             LogNextPrefix;
    const char*             LogNextSuffix;
    float                   LogLinePosY;
    bool                    LogLineFirstItem;
    int                     LogDepthRef;
    int                     LogDepthToExpand;
    int                     LogDepthToExpandDefault;            // Default/stored value for LogDepthMaxExpand if not specified in the LogXXX function call.

    // Debug Tools
    bool                    DebugItemPickerActive;              // Item picker is active (started with DebugStartItemPicker())
    ImGuiID                 DebugItemPickerBreakId;             // Will call IM_DEBUG_BREAK() when encountering this id
    ImGuiMetricsConfig      DebugMetricsConfig;

    // Misc
    float                   FramerateSecPerFrame[120];          // Calculate estimate of framerate for user over the last 2 seconds.
    int                     FramerateSecPerFrameIdx;
    int                     FramerateSecPerFrameCount;
    float                   FramerateSecPerFrameAccum;
    int                     WantCaptureMouseNextFrame;          // Explicit capture via CaptureKeyboardFromApp()/CaptureMouseFromApp() sets those flags
    int                     WantCaptureKeyboardNextFrame;
    int                     WantTextInputNextFrame;
    char                    TempBuffer[1024 * 3 + 1];           // Temporary text buffer

    ImGuiContext(ImFontAtlas* shared_font_atlas)
    {
        Initialized = false;
        FontAtlasOwnedByContext = shared_font_atlas ? false : true;
        Font = NULL;
        FontSize = FontBaseSize = 0.0f;
        IO.Fonts = shared_font_atlas ? shared_font_atlas : IM_NEW(ImFontAtlas)();
        Time = 0.0f;
        FrameCount = 0;
        FrameCountEnded = FrameCountRendered = -1;
        WithinFrameScope = WithinFrameScopeWithImplicitWindow = WithinEndChild = false;
        GcCompactAll = false;
        TestEngineHookItems = false;
        TestEngineHookIdInfo = 0;
        TestEngine = NULL;

        WindowsActiveCount = 0;
        CurrentWindow = NULL;
        HoveredWindow = NULL;
        HoveredWindowUnderMovingWindow = NULL;
        MovingWindow = NULL;
        WheelingWindow = NULL;
        WheelingWindowTimer = 0.0f;

        CurrentItemFlags = ImGuiItemFlags_None;
        HoveredId = HoveredIdPreviousFrame = 0;
        HoveredIdAllowOverlap = false;
        HoveredIdUsingMouseWheel = HoveredIdPreviousFrameUsingMouseWheel = false;
        HoveredIdDisabled = false;
        HoveredIdTimer = HoveredIdNotActiveTimer = 0.0f;
        ActiveId = 0;
        ActiveIdIsAlive = 0;
        ActiveIdTimer = 0.0f;
        ActiveIdIsJustActivated = false;
        ActiveIdAllowOverlap = false;
        ActiveIdNoClearOnFocusLoss = false;
        ActiveIdHasBeenPressedBefore = false;
        ActiveIdHasBeenEditedBefore = false;
        ActiveIdHasBeenEditedThisFrame = false;
        ActiveIdUsingMouseWheel = false;
        ActiveIdUsingNavDirMask = 0x00;
        ActiveIdUsingNavInputMask = 0x00;
        ActiveIdUsingKeyInputMask = 0x00;
        ActiveIdClickOffset = ImVec2(-1, -1);
        ActiveIdWindow = NULL;
        ActiveIdSource = ImGuiInputSource_None;
        ActiveIdMouseButton = -1;
        ActiveIdPreviousFrame = 0;
        ActiveIdPreviousFrameIsAlive = false;
        ActiveIdPreviousFrameHasBeenEditedBefore = false;
        ActiveIdPreviousFrameWindow = NULL;
        LastActiveId = 0;
        LastActiveIdTimer = 0.0f;

        NavWindow = NULL;
        NavId = NavFocusScopeId = NavActivateId = NavActivateDownId = NavActivatePressedId = NavInputId = 0;
        NavJustTabbedId = NavJustMovedToId = NavJustMovedToFocusScopeId = NavNextActivateId = 0;
        NavJustMovedToKeyMods = ImGuiKeyModFlags_None;
        NavInputSource = ImGuiInputSource_None;
        NavScoringRect = ImRect();
        NavScoringCount = 0;
        NavLayer = ImGuiNavLayer_Main;
        NavIdTabCounter = INT_MAX;
        NavIdIsAlive = false;
        NavMousePosDirty = false;
        NavDisableHighlight = true;
        NavDisableMouseHover = false;
        NavAnyRequest = false;
        NavInitRequest = false;
        NavInitRequestFromMove = false;
        NavInitResultId = 0;
        NavMoveRequest = false;
        NavMoveRequestFlags = ImGuiNavMoveFlags_None;
        NavMoveRequestForward = ImGuiNavForward_None;
        NavMoveRequestKeyMods = ImGuiKeyModFlags_None;
        NavMoveDir = NavMoveDirLast = NavMoveClipDir = ImGuiDir_None;
        NavWrapRequestWindow = NULL;
        NavWrapRequestFlags = ImGuiNavMoveFlags_None;

        NavWindowingTarget = NavWindowingTargetAnim = NavWindowingListWindow = NULL;
        NavWindowingTimer = NavWindowingHighlightAlpha = 0.0f;
        NavWindowingToggleLayer = false;

        TabFocusRequestCurrWindow = TabFocusRequestNextWindow = NULL;
        TabFocusRequestCurrCounterRegular = TabFocusRequestCurrCounterTabStop = INT_MAX;
        TabFocusRequestNextCounterRegular = TabFocusRequestNextCounterTabStop = INT_MAX;
        TabFocusPressed = false;

        DimBgRatio = 0.0f;
        MouseCursor = ImGuiMouseCursor_Arrow;

        DragDropActive = DragDropWithinSource = DragDropWithinTarget = false;
        DragDropSourceFlags = ImGuiDragDropFlags_None;
        DragDropSourceFrameCount = -1;
        DragDropMouseButton = -1;
        DragDropTargetId = 0;
        DragDropAcceptFlags = ImGuiDragDropFlags_None;
        DragDropAcceptIdCurrRectSurface = 0.0f;
        DragDropAcceptIdPrev = DragDropAcceptIdCurr = 0;
        DragDropAcceptFrameCount = -1;
        DragDropHoldJustPressedId = 0;
        memset(DragDropPayloadBufLocal, 0, sizeof(DragDropPayloadBufLocal));

        CurrentTable = NULL;
        CurrentTableStackIdx = -1;
        CurrentTabBar = NULL;

        LastValidMousePos = ImVec2(0.0f, 0.0f);
        TempInputId = 0;
        ColorEditOptions = ImGuiColorEditFlags__OptionsDefault;
        ColorEditLastHue = ColorEditLastSat = 0.0f;
        ColorEditLastColor[0] = ColorEditLastColor[1] = ColorEditLastColor[2] = FLT_MAX;
        SliderCurrentAccum = 0.0f;
        SliderCurrentAccumDirty = false;
        DragCurrentAccumDirty = false;
        DragCurrentAccum = 0.0f;
        DragSpeedDefaultRatio = 1.0f / 100.0f;
        ScrollbarClickDeltaToGrabCenter = 0.0f;
        TooltipOverrideCount = 0;
        TooltipSlowDelay = 0.50f;

        PlatformImePos = PlatformImeLastPos = ImVec2(FLT_MAX, FLT_MAX);
        PlatformLocaleDecimalPoint = '.';

        SettingsLoaded = false;
        SettingsDirtyTimer = 0.0f;
        HookIdNext = 0;

        LogEnabled = false;
        LogType = ImGuiLogType_None;
        LogNextPrefix = LogNextSuffix = NULL;
        LogFile = NULL;
        LogLinePosY = FLT_MAX;
        LogLineFirstItem = false;
        LogDepthRef = 0;
        LogDepthToExpand = LogDepthToExpandDefault = 2;

        DebugItemPickerActive = false;
        DebugItemPickerBreakId = 0;

        memset(FramerateSecPerFrame, 0, sizeof(FramerateSecPerFrame));
        FramerateSecPerFrameIdx = FramerateSecPerFrameCount = 0;
        FramerateSecPerFrameAccum = 0.0f;
        WantCaptureMouseNextFrame = WantCaptureKeyboardNextFrame = WantTextInputNextFrame = -1;
        memset(TempBuffer, 0, sizeof(TempBuffer));
    }
};
