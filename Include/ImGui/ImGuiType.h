#pragma once
#include <ImGui/ImGuiDefine.h>

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// Forward declarations
struct ImDrawChannel; // Temporary storage to output draw commands out of order, used by ImDrawListSplitter and
                      // ImDrawList::ChannelsSplit()
struct ImDrawCmd; // A single draw command within a parent ImDrawList (generally maps to 1 GPU draw call, unless it is a
                  // callback)
struct ImDrawData; // All draw command lists required to render the frame + pos/size coordinates to use for the
                   // projection matrix.
struct ImDrawList; // A single draw command list (generally one per window, conceptually you may see this as a dynamic
                   // "mesh" builder)
struct ImDrawListSharedData; // Data shared among multiple draw lists (typically owned by parent ImGui context, but you
                             // may create one yourself)
struct ImDrawListSplitter;   // Helper to split a draw list into different layers which can be drawn into out of order,
                             // then flattened back.
struct ImDrawVert;           // A single vertex (pos + uv + col = 20 bytes by default. Override layout with
                             // IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
struct ImFont;               // Runtime data for a single font within a parent ImFontAtlas
struct ImFontAtlas; // Runtime data for multiple fonts, bake multiple fonts into a single texture, TTF/OTF font loader
struct ImFontBuilderIO;          // Opaque interface to a font builder (stb_truetype or FreeType).
struct ImFontConfig;             // Configuration data when adding a font or merging fonts
struct ImFontGlyph;              // A single font glyph (code point + coordinates within in ImFontAtlas + offset)
struct ImFontGlyphRangesBuilder; // Helper to build glyph ranges from text/string data
struct ImColor; // Helper functions to create a color that can be converted to either u32 or float4 (*OBSOLETE* please
                // avoid using)
struct ImGuiContext;               // Dear ImGui context (opaque structure, unless including imgui_internal.h)
struct ImGuiIO;                    // Main configuration and I/O between your application and ImGui
struct ImGuiInputTextCallbackData; // Shared state of InputText() when using custom ImGuiInputTextCallback
                                   // (rare/advanced use)
struct ImGuiListClipper;           // Helper to manually clip large list of items
struct ImGuiOnceUponAFrame;        // Helper for running a block of code not more than once a frame, used by
                                   // IMGUI_ONCE_UPON_A_FRAME macro
struct ImGuiPayload;               // User data payload for drag and drop operations
struct ImGuiSizeCallbackData;      // Callback data when using SetNextWindowSizeConstraints() (rare/advanced use)
struct ImGuiStorage;               // Helper for key->value storage
struct ImGuiStyle;                 // Runtime data for styling/colors
struct ImGuiTableSortSpecs;        // Sorting specifications for a table (often handling sort specs for a single column,
                                   // occasionally more)
struct ImGuiTableColumnSortSpecs;  // Sorting specification for one column of a table
struct ImGuiTextBuffer;            // Helper to hold and append into a text buffer (~string builder)
struct ImGuiTextFilter;            // Helper to parse and apply text filters (e.g. "aaaaa[,bbbbb][,ccccc]")
struct ImGuiViewport; // A Platform Window (always only one in 'master' branch), in the future may represent Platform
                      // Monitor

// Enums/Flags (declared as int for compatibility with old C++, to allow using as flags and to not pollute the top of
// this file)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual
// flags/enum lists!
//   In Visual Studio IDE: CTRL+comma ("Edit.NavigateTo") can follow symbols in comments, whereas CTRL+F12
//   ("Edit.GoToImplementation") cannot. With Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also
//   follow symbols in comments.
typedef int ImGuiCol;      // -> enum ImGuiCol_             // Enum: A color identifier for styling
typedef int ImGuiCond;     // -> enum ImGuiCond_            // Enum: A condition for many Set*() functions
typedef int ImGuiDataType; // -> enum ImGuiDataType_        // Enum: A primary data type
typedef int ImGuiDir;      // -> enum ImGuiDir_             // Enum: A cardinal direction
typedef int ImGuiKey;      // -> enum ImGuiKey_             // Enum: A key identifier (ImGui-side enum)
typedef int ImGuiNavInput; // -> enum ImGuiNavInput_        // Enum: An input identifier for navigation
typedef int
    ImGuiMouseButton; // -> enum ImGuiMouseButton_     // Enum: A mouse button identifier (0=left, 1=right, 2=middle)
typedef int ImGuiMouseCursor;    // -> enum ImGuiMouseCursor_     // Enum: A mouse cursor identifier
typedef int ImGuiSortDirection;  // -> enum ImGuiSortDirection_   // Enum: A sorting direction (ascending or descending)
typedef int ImGuiStyleVar;       // -> enum ImGuiStyleVar_        // Enum: A variable identifier for styling
typedef int ImGuiTableBgTarget;  // -> enum ImGuiTableBgTarget_   // Enum: A color target for TableSetBgColor()
typedef int ImDrawFlags;         // -> enum ImDrawFlags_          // Flags: for ImDrawList functions
typedef int ImDrawListFlags;     // -> enum ImDrawListFlags_      // Flags: for ImDrawList instance
typedef int ImFontAtlasFlags;    // -> enum ImFontAtlasFlags_     // Flags: for ImFontAtlas build
typedef int ImGuiBackendFlags;   // -> enum ImGuiBackendFlags_    // Flags: for io.BackendFlags
typedef int ImGuiButtonFlags;    // -> enum ImGuiButtonFlags_     // Flags: for InvisibleButton()
typedef int ImGuiColorEditFlags; // -> enum ImGuiColorEditFlags_  // Flags: for ColorEdit4(), ColorPicker4() etc.
typedef int ImGuiConfigFlags;    // -> enum ImGuiConfigFlags_     // Flags: for io.ConfigFlags
typedef int ImGuiComboFlags;     // -> enum ImGuiComboFlags_      // Flags: for BeginCombo()
typedef int
    ImGuiDragDropFlags; // -> enum ImGuiDragDropFlags_   // Flags: for BeginDragDropSource(), AcceptDragDropPayload()
typedef int ImGuiFocusedFlags;   // -> enum ImGuiFocusedFlags_    // Flags: for IsWindowFocused()
typedef int ImGuiHoveredFlags;   // -> enum ImGuiHoveredFlags_    // Flags: for IsItemHovered(), IsWindowHovered() etc.
typedef int ImGuiInputTextFlags; // -> enum ImGuiInputTextFlags_  // Flags: for InputText(), InputTextMultiline()
typedef int ImGuiKeyModFlags;    // -> enum ImGuiKeyModFlags_     // Flags: for io.KeyMods (Ctrl/Shift/Alt/Super)
typedef int
    ImGuiPopupFlags; // -> enum ImGuiPopupFlags_      // Flags: for OpenPopup*(), BeginPopupContext*(), IsPopupOpen()
typedef int ImGuiSelectableFlags;  // -> enum ImGuiSelectableFlags_ // Flags: for Selectable()
typedef int ImGuiSliderFlags;      // -> enum ImGuiSliderFlags_     // Flags: for DragFloat(), DragInt(), SliderFloat(),
                                   // SliderInt() etc.
typedef int ImGuiTabBarFlags;      // -> enum ImGuiTabBarFlags_     // Flags: for BeginTabBar()
typedef int ImGuiTabItemFlags;     // -> enum ImGuiTabItemFlags_    // Flags: for BeginTabItem()
typedef int ImGuiTableFlags;       // -> enum ImGuiTableFlags_      // Flags: For BeginTable()
typedef int ImGuiTableColumnFlags; // -> enum ImGuiTableColumnFlags_// Flags: For TableSetupColumn()
typedef int ImGuiTableRowFlags;    // -> enum ImGuiTableRowFlags_   // Flags: For TableNextRow()
typedef int
    ImGuiTreeNodeFlags; // -> enum ImGuiTreeNodeFlags_   // Flags: for TreeNode(), TreeNodeEx(), CollapsingHeader()
typedef int ImGuiViewportFlags; // -> enum ImGuiViewportFlags_   // Flags: for ImGuiViewport
typedef int ImGuiWindowFlags;   // -> enum ImGuiWindowFlags_     // Flags: for Begin(), BeginChild()

// Other types
#ifndef ImTextureID        // ImTextureID [configurable type: override in imconfig.h with '#define ImTextureID xxx']
typedef void *ImTextureID; // User data for rendering backend to identify a texture. This is whatever to you want it to
                           // be! read the FAQ about ImTextureID for details.
#endif
typedef unsigned int ImGuiID; // A unique ID used by widgets, typically hashed from a stack of string.
typedef int (*ImGuiInputTextCallback)(ImGuiInputTextCallbackData *data); // Callback function for ImGui::InputText()
typedef void (*ImGuiSizeCallback)(
    ImGuiSizeCallbackData *data); // Callback function for ImGui::SetNextWindowSizeConstraints()
typedef void *(*ImGuiMemAllocFunc)(size_t sz, void *user_data); // Function signature for ImGui::SetAllocatorFunctions()
typedef void (*ImGuiMemFreeFunc)(void *ptr, void *user_data);   // Function signature for ImGui::SetAllocatorFunctions()

// Character types
// (we generally use UTF-8 encoded string in the API. This is storage specifically for a decoded character used for
// keyboard input and display)
typedef unsigned short
    ImWchar16; // A single decoded U16 character/code point. We encode them as multi bytes UTF-8 when used in strings.
typedef unsigned int
    ImWchar32; // A single decoded U32 character/code point. We encode them as multi bytes UTF-8 when used in strings.
#ifdef IMGUI_USE_WCHAR32 // ImWchar [configurable type: override in imconfig.h with '#define IMGUI_USE_WCHAR32' to
                         // support Unicode planes 1-16]
typedef ImWchar32 ImWchar;
#else
typedef ImWchar16 ImWchar;
#endif

// Basic scalar data types
typedef signed char ImS8;     // 8-bit signed integer
typedef unsigned char ImU8;   // 8-bit unsigned integer
typedef signed short ImS16;   // 16-bit signed integer
typedef unsigned short ImU16; // 16-bit unsigned integer
typedef signed int ImS32;     // 32-bit signed integer == int
typedef unsigned int ImU32;   // 32-bit unsigned integer (often used to store packed colors)
#if defined(_MSC_VER) && !defined(__clang__)
typedef signed __int64 ImS64;   // 64-bit signed integer (pre and post C++11 with Visual Studio)
typedef unsigned __int64 ImU64; // 64-bit unsigned integer (pre and post C++11 with Visual Studio)
#elif (defined(__clang__) || defined(__GNUC__)) && (__cplusplus < 201100)
#include <stdint.h>
typedef int64_t ImS64;  // 64-bit signed integer (pre C++11)
typedef uint64_t ImU64; // 64-bit unsigned integer (pre C++11)
#else
typedef signed long long ImS64;   // 64-bit signed integer (post C++11)
typedef unsigned long long ImU64; // 64-bit unsigned integer (post C++11)
#endif

// 2D vector (often used to store positions or sizes)
IM_MSVC_RUNTIME_CHECKS_OFF
struct ImVec2
{
    float x, y;
    ImVec2()
    {
        x = y = 0.0f;
    }
    ImVec2(float _x, float _y)
    {
        x = _x;
        y = _y;
    }
    float operator[](size_t idx) const
    {
        IM_ASSERT(idx <= 1);
        return (&x)[idx];
    } // We very rarely use this [] operator, the assert overhead is fine.
    float &operator[](size_t idx)
    {
        IM_ASSERT(idx <= 1);
        return (&x)[idx];
    } // We very rarely use this [] operator, the assert overhead is fine.
#ifdef IM_VEC2_CLASS_EXTRA
    IM_VEC2_CLASS_EXTRA // Define additional constructors and implicit cast operators in imconfig.h to convert back and
                        // forth between your math types and ImVec2.
#endif
};

// 4D vector (often used to store floating-point colors)
struct ImVec4
{
    float x, y, z, w;
    ImVec4()
    {
        x = y = z = w = 0.0f;
    }
    ImVec4(float _x, float _y, float _z, float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }
#ifdef IM_VEC4_CLASS_EXTRA
    IM_VEC4_CLASS_EXTRA // Define additional constructors and implicit cast operators in imconfig.h to convert back and
                        // forth between your math types and ImVec4.
#endif
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations
//-----------------------------------------------------------------------------

struct ImBitVector;             // Store 1-bit per value
struct ImRect;                  // An axis-aligned rectangle (2 points)
struct ImDrawDataBuilder;       // Helper to build a ImDrawData instance
struct ImDrawListSharedData;    // Data shared between all ImDrawList instances
struct ImGuiColorMod;           // Stacked color modifier, backup of modified data so we can restore it
struct ImGuiContext;            // Main Dear ImGui context
struct ImGuiContextHook;        // Hook for extensions like ImGuiTestEngine
struct ImGuiDataTypeInfo;       // Type information associated to a ImGuiDataType enum
struct ImGuiGroupData;          // Stacked storage data for BeginGroup()/EndGroup()
struct ImGuiInputTextState;     // Internal state of the currently focused/edited text input box
struct ImGuiLastItemDataBackup; // Backup and restore IsItemHovered() internal data
struct ImGuiMenuColumns;        // Simple column measurement, currently used for MenuItem() only
struct ImGuiNavItemData;        // Result of a gamepad/keyboard directional navigation move query result
struct ImGuiMetricsConfig;      // Storage for ShowMetricsWindow() and DebugNodeXXX() functions
struct ImGuiNextWindowData;     // Storage for SetNextWindow** functions
struct ImGuiNextItemData;       // Storage for SetNextItem** functions
struct ImGuiOldColumnData;      // Storage data for a single column for legacy Columns() api
struct ImGuiOldColumns;         // Storage data for a columns set for legacy Columns() api
struct ImGuiPopupData;          // Storage for current popup stack
struct ImGuiSettingsHandler;    // Storage for one type registered in the .ini file
struct ImGuiStackSizes;         // Storage of stack sizes for debugging/asserting
struct ImGuiStyleMod;           // Stacked style modifier, backup of modified data so we can restore it
struct ImGuiTabBar;             // Storage for a tab bar
struct ImGuiTabItem;            // Storage for a tab item (within a tab bar)
struct ImGuiTable;              // Storage for a table
struct ImGuiTableColumn;        // Storage for one column of a table
struct ImGuiTableTempData;      // Temporary storage for one table (one per table in the stack), shared between tables.
struct ImGuiTableSettings;      // Storage for a table .ini settings
struct ImGuiTableColumnsSettings; // Storage for a column .ini settings
struct ImGuiWindow;               // Storage for one window
struct ImGuiWindowTempData; // Temporary storage for one window (that's the data which in theory we could ditch at the
                            // end of the frame, in practice we currently keep it for each window)
struct ImGuiWindowSettings; // Storage for a window .ini settings (we keep one of those even if the actual window wasn't
                            // instanced during this session)

// Use your programming IDE "Go to definition" facility on the names of the center columns to find the actual flags/enum
// lists.
typedef int ImGuiLayoutType;          // -> enum ImGuiLayoutType_         // Enum: Horizontal or vertical
typedef int ImGuiItemFlags;           // -> enum ImGuiItemFlags_          // Flags: for PushItemFlag()
typedef int ImGuiItemAddFlags;        // -> enum ImGuiItemAddFlags_       // Flags: for ItemAdd()
typedef int ImGuiItemStatusFlags;     // -> enum ImGuiItemStatusFlags_    // Flags: for DC.LastItemStatusFlags
typedef int ImGuiOldColumnFlags;      // -> enum ImGuiOldColumnFlags_     // Flags: for BeginColumns()
typedef int ImGuiNavHighlightFlags;   // -> enum ImGuiNavHighlightFlags_  // Flags: for RenderNavHighlight()
typedef int ImGuiNavDirSourceFlags;   // -> enum ImGuiNavDirSourceFlags_  // Flags: for GetNavInputAmount2d()
typedef int ImGuiNavMoveFlags;        // -> enum ImGuiNavMoveFlags_       // Flags: for navigation requests
typedef int ImGuiNextItemDataFlags;   // -> enum ImGuiNextItemDataFlags_  // Flags: for SetNextItemXXX() functions
typedef int ImGuiNextWindowDataFlags; // -> enum ImGuiNextWindowDataFlags_// Flags: for SetNextWindowXXX() functions
typedef int ImGuiSeparatorFlags;      // -> enum ImGuiSeparatorFlags_     // Flags: for SeparatorEx()
typedef int ImGuiTextFlags;           // -> enum ImGuiTextFlags_          // Flags: for TextEx()
typedef int ImGuiTooltipFlags;        // -> enum ImGuiTooltipFlags_       // Flags: for BeginTooltipEx()

typedef void (*ImGuiErrorLogCallback)(void *user_data, const char *fmt, ...);

//-------------------------------------------------------------------------
// [SECTION] STB libraries includes
//-------------------------------------------------------------------------

struct ImGuiInputTextState; // Internal state of the currently focused/edited text input box

namespace ImStb
{

#undef STB_TEXTEDIT_STRING
#undef STB_TEXTEDIT_CHARTYPE
#define STB_TEXTEDIT_STRING ImGuiInputTextState
#define STB_TEXTEDIT_CHARTYPE ImWchar
#define STB_TEXTEDIT_GETWIDTH_NEWLINE (-1.0f)
#define STB_TEXTEDIT_UNDOSTATECOUNT 99
#define STB_TEXTEDIT_UNDOCHARCOUNT 999
// #include "imstb_textedit.h"
#include <stb_textedit.h>

} // namespace ImStb
