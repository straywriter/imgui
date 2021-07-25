#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{


// Viewports
// - Currently represents the Platform Window created by the application which is hosting our Dear ImGui windows.
// - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple active viewports.
// - In the future we will extend this concept further to also represent Platform Monitor and support a "no main
// platform window" operation mode.
IMGUI_API ImGuiViewport *GetMainViewport(); // return primary/default viewport. This can never be NULL.

// // Miscellaneous Utilities
// IMGUI_API bool IsRectVisible(
//     const ImVec2 &size); // test if rectangle (of given size, starting from cursor position) is visible / not clipped.
// IMGUI_API bool IsRectVisible(const ImVec2 &rect_min,
//                              const ImVec2 &rect_max); // test if rectangle (in screen space) is visible / not clipped.
//                                                       // to perform coarse clipping on user's side.
// IMGUI_API double GetTime();                           // get global imgui time. incremented by io.DeltaTime every frame.
// IMGUI_API int GetFrameCount();                        // get global imgui frame count. incremented by 1 every frame.
// IMGUI_API ImDrawList *GetBackgroundDrawList(); // this draw list will be the first rendering one. Useful to quickly draw
//                                                // shapes/text behind dear imgui contents.
// IMGUI_API ImDrawList *GetForegroundDrawList(); // this draw list will be the last rendered one. Useful to quickly draw
//                                                // shapes/text over dear imgui contents.
// IMGUI_API ImDrawListSharedData *GetDrawListSharedData(); // you may use this when creating your own ImDrawList
//                                                          // instances.
// IMGUI_API const char *GetStyleColorName(
//     ImGuiCol idx); // get a string corresponding to the enum value (for display, saving, etc.).
// IMGUI_API void SetStateStorage(ImGuiStorage *storage); // replace current window storage with our own (if you want to
//                                                        // manipulate it yourself, typically clear subsection of it)
// IMGUI_API ImGuiStorage *GetStateStorage();
// IMGUI_API void CalcListClipping(
//     int items_count, float items_height, int *out_items_display_start,
//     int *out_items_display_end); // calculate coarse clipping for large list of evenly sized items. Prefer using the
//                                  // ImGuiListClipper higher-level helper if you can.
// IMGUI_API bool BeginChildFrame(ImGuiID id, const ImVec2 &size,
//                                ImGuiWindowFlags flags = 0); // helper to create a child window / scrolling region that
//                                                             // looks like a normal widget frame
// IMGUI_API void EndChildFrame(); // always call EndChildFrame() regardless of BeginChildFrame() return values (which
//                                 // indicates a collapsed/clipped window)



} // namespace ImGui


//-----------------------------------------------------------------------------
// [SECTION] Viewports
//-----------------------------------------------------------------------------

// Flags stored in ImGuiViewport::Flags
enum ImGuiViewportFlags_
{
    ImGuiViewportFlags_None                     = 0,
    ImGuiViewportFlags_IsPlatformWindow         = 1 << 0,   // Represent a Platform Window
    ImGuiViewportFlags_IsPlatformMonitor        = 1 << 1,   // Represent a Platform Monitor (unused yet)
    ImGuiViewportFlags_OwnedByApp               = 1 << 2    // Platform Window: is created/managed by the application (rather than a dear imgui backend)
};

// - Currently represents the Platform Window created by the application which is hosting our Dear ImGui windows.
// - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple active viewports.
// - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
// - About Main Area vs Work Area:
//   - Main Area = entire viewport.
//   - Work Area = entire viewport minus sections used by main menu bars (for platform windows), or by task bar (for platform monitor).
//   - Windows are generally trying to stay within the Work Area of their host viewport.
struct ImGuiViewport
{
    ImGuiViewportFlags  Flags;                  // See ImGuiViewportFlags_
    ImVec2              Pos;                    // Main Area: Position of the viewport (Dear ImGui coordinates are the same as OS desktop/native coordinates)
    ImVec2              Size;                   // Main Area: Size of the viewport.
    ImVec2              WorkPos;                // Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)
    ImVec2              WorkSize;               // Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)

    ImGuiViewport()     { memset(this, 0, sizeof(*this)); }

    // Helpers
    ImVec2              GetCenter() const       { return ImVec2(Pos.x + Size.x * 0.5f, Pos.y + Size.y * 0.5f); }
    ImVec2              GetWorkCenter() const   { return ImVec2(WorkPos.x + WorkSize.x * 0.5f, WorkPos.y + WorkSize.y * 0.5f); }
};


//-----------------------------------------------------------------------------
// [SECTION] Viewport support
//-----------------------------------------------------------------------------

// ImGuiViewport Private/Internals fields (cardinal sin: we are using inheritance!)
// Every instance of ImGuiViewport is in fact a ImGuiViewportP.
struct ImGuiViewportP : public ImGuiViewport
{
    int                 DrawListsLastFrame[2];  // Last frame number the background (0) and foreground (1) draw lists were used
    ImDrawList*         DrawLists[2];           // Convenience background (0) and foreground (1) draw lists. We use them to draw software mouser cursor when io.MouseDrawCursor is set and to draw most debug overlays.
    ImDrawData          DrawDataP;
    ImDrawDataBuilder   DrawDataBuilder;

    ImVec2              WorkOffsetMin;          // Work Area: Offset from Pos to top-left corner of Work Area. Generally (0,0) or (0,+main_menu_bar_height). Work Area is Full Area but without menu-bars/status-bars (so WorkArea always fit inside Pos/Size!)
    ImVec2              WorkOffsetMax;          // Work Area: Offset from Pos+Size to bottom-right corner of Work Area. Generally (0,0) or (0,-status_bar_height).
    ImVec2              BuildWorkOffsetMin;     // Work Area: Offset being built during current frame. Generally >= 0.0f.
    ImVec2              BuildWorkOffsetMax;     // Work Area: Offset being built during current frame. Generally <= 0.0f.

    ImGuiViewportP()    { DrawListsLastFrame[0] = DrawListsLastFrame[1] = -1; DrawLists[0] = DrawLists[1] = NULL; }
    ~ImGuiViewportP()   { if (DrawLists[0]) IM_DELETE(DrawLists[0]); if (DrawLists[1]) IM_DELETE(DrawLists[1]); }

    // Calculate work rect pos/size given a set of offset (we have 1 pair of offset for rect locked from last frame data, and 1 pair for currently building rect)
    ImVec2  CalcWorkRectPos(const ImVec2& off_min) const                            { return ImVec2(Pos.x + off_min.x, Pos.y + off_min.y); }
    ImVec2  CalcWorkRectSize(const ImVec2& off_min, const ImVec2& off_max) const    { return ImVec2(ImMax(0.0f, Size.x - off_min.x + off_max.x), ImMax(0.0f, Size.y - off_min.y + off_max.y)); }
    void    UpdateWorkRect()            { WorkPos = CalcWorkRectPos(WorkOffsetMin); WorkSize = CalcWorkRectSize(WorkOffsetMin, WorkOffsetMax); } // Update public fields

    // Helpers to retrieve ImRect (we don't need to store BuildWorkRect as every access tend to change it, hence the code asymmetry)
    ImRect  GetMainRect() const         { return ImRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    ImRect  GetWorkRect() const         { return ImRect(WorkPos.x, WorkPos.y, WorkPos.x + WorkSize.x, WorkPos.y + WorkSize.y); }
    ImRect  GetBuildWorkRect() const    { ImVec2 pos = CalcWorkRectPos(BuildWorkOffsetMin); ImVec2 size = CalcWorkRectSize(BuildWorkOffsetMin, BuildWorkOffsetMax); return ImRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y); }
};