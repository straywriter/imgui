// dear imgui, v1.83 WIP
// (widgets code)

/*

Index of this file:

// [SECTION] Forward Declarations
// [SECTION] Widgets: Text, etc.
// [SECTION] Widgets: Main (Button, Image, Checkbox, RadioButton, ProgressBar, Bullet, etc.)
// [SECTION] Widgets: Low-level Layout helpers (Spacing, Dummy, NewLine, Separator, etc.)
// [SECTION] Widgets: ComboBox
// [SECTION] Data Type and Data Formatting Helpers
// [SECTION] Widgets: DragScalar, DragFloat, DragInt, etc.
// [SECTION] Widgets: SliderScalar, SliderFloat, SliderInt, etc.
// [SECTION] Widgets: InputScalar, InputFloat, InputInt, etc.
// [SECTION] Widgets: InputText, InputTextMultiline
// [SECTION] Widgets: ColorEdit, ColorPicker, ColorButton, etc.
// [SECTION] Widgets: TreeNode, CollapsingHeader, etc.
// [SECTION] Widgets: Selectable
// [SECTION] Widgets: ListBox
// [SECTION] Widgets: PlotLines, PlotHistogram
// [SECTION] Widgets: Value helpers
// [SECTION] Widgets: MenuItem, BeginMenu, EndMenu, etc.
// [SECTION] Widgets: BeginTabBar, EndTabBar, etc.
// [SECTION] Widgets: BeginTabItem, EndTabItem, etc.
// [SECTION] Widgets: Columns, BeginColumns, EndColumns, etc.

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif



#include <ImGui/ImGui.h>
#ifndef IMGUI_DISABLE


#include "ImGui/ImGuiInternal.h"

// System includes
#include <ctype.h>      // toupper
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

//-------------------------------------------------------------------------
// Warnings
//-------------------------------------------------------------------------

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#if defined(_MSC_VER) && _MSC_VER >= 1922 // MSVC 2019 16.2 or later
#pragma warning (disable: 5054)     // operator '|': deprecated between enumerations of different types
#endif
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#pragma warning (disable: 26812)    // [Static Analyzer] The enum type 'xxx' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                      // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                            // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants (typically 0.0f) is ok.
#pragma clang diagnostic ignored "-Wformat-nonliteral"              // warning: format string is not a string literal            // passing non-literal to vsnformat(). yes, user passing incorrect format strings can crash the code.
#pragma clang diagnostic ignored "-Wsign-conversion"                // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                    // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wenum-enum-conversion"           // warning: bitwise operation between different enumeration types ('XXXFlags_' and 'XXXFlagsPrivate_')
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"// warning: bitwise operation between different enumeration types ('XXXFlags_' and 'XXXFlagsPrivate_') is deprecated
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wformat-nonliteral"                // warning: format not a string literal, format string not checked
#pragma GCC diagnostic ignored "-Wclass-memaccess"                  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

//-------------------------------------------------------------------------
// Data
//-------------------------------------------------------------------------

// Widgets
// static const float          DRAGDROP_HOLD_TO_OPEN_TIMER = 0.70f;    // Time for drag-hold to activate items accepting the ImGuiButtonFlags_PressedOnDragDropHold button behavior.
static const float          DRAG_MOUSE_THRESHOLD_FACTOR = 0.50f;    // Multiplier for the default value of io.MouseDragThreshold to make DragFloat/DragInt react faster to mouse drags.

// Those MIN/MAX values are not define because we need to point to them
static const signed char    IM_S8_MIN  = -128;
static const signed char    IM_S8_MAX  = 127;
static const unsigned char  IM_U8_MIN  = 0;
static const unsigned char  IM_U8_MAX  = 0xFF;
static const signed short   IM_S16_MIN = -32768;
static const signed short   IM_S16_MAX = 32767;
static const unsigned short IM_U16_MIN = 0;
static const unsigned short IM_U16_MAX = 0xFFFF;
static const ImS32          IM_S32_MIN = INT_MIN;    // (-2147483647 - 1), (0x80000000);
static const ImS32          IM_S32_MAX = INT_MAX;    // (2147483647), (0x7FFFFFFF)
static const ImU32          IM_U32_MIN = 0;
static const ImU32          IM_U32_MAX = UINT_MAX;   // (0xFFFFFFFF)
#ifdef LLONG_MIN
static const ImS64          IM_S64_MIN = LLONG_MIN;  // (-9223372036854775807ll - 1ll);
static const ImS64          IM_S64_MAX = LLONG_MAX;  // (9223372036854775807ll);
#else
static const ImS64          IM_S64_MIN = -9223372036854775807LL - 1;
static const ImS64          IM_S64_MAX = 9223372036854775807LL;
#endif
static const ImU64          IM_U64_MIN = 0;
#ifdef ULLONG_MAX
static const ImU64          IM_U64_MAX = ULLONG_MAX; // (0xFFFFFFFFFFFFFFFFull);
#else
static const ImU64          IM_U64_MAX = (2ULL * 9223372036854775807LL + 1);
#endif

//-------------------------------------------------------------------------
// [SECTION] Forward Declarations
//-------------------------------------------------------------------------

// For InputTextEx()
// static bool             InputTextFilterCharacter(unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data, ImGuiInputSource input_source);
// static int              InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end);
// static ImVec2           InputTextCalcTextSizeW(const ImWchar* text_begin, const ImWchar* text_end, const ImWchar** remaining = NULL, ImVec2* out_offset = NULL, bool stop_on_new_line = false);



// size_arg (for each axis) < 0.0f: align to end, 0.0f: auto, > 0.0f: specified size
void ImGui::ProgressBar(float fraction, const ImVec2& size_arg, const char* overlay)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f);
    ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, 0))
        return;

    // Render
    fraction = ImSaturate(fraction);
    RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));
    const ImVec2 fill_br = ImVec2(ImLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y);
    RenderRectFilledRangeH(window->DrawList, bb, GetColorU32(ImGuiCol_PlotHistogram), 0.0f, fraction, style.FrameRounding);

    // Default displaying the fraction as percentage string, but user can override it
    char overlay_buf[32];
    if (!overlay)
    {
        ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), "%.0f%%", fraction * 100 + 0.01f);
        overlay = overlay_buf;
    }

    ImVec2 overlay_size = CalcTextSize(overlay, NULL);
    if (overlay_size.x > 0.0f)
        RenderTextClipped(ImVec2(ImClamp(fill_br.x + style.ItemSpacing.x, bb.Min.x, bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y), bb.Max, overlay, NULL, &overlay_size, ImVec2(0.0f, 0.5f), &bb);
}

void ImGui::Bullet()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float line_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + g.Style.FramePadding.y * 2), g.FontSize);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(g.FontSize, line_height));
    ItemSize(bb);
    if (!ItemAdd(bb, 0))
    {
        SameLine(0, style.FramePadding.x * 2);
        return;
    }

    // Render and stay on same line
    ImU32 text_col = GetColorU32(ImGuiCol_Text);
    RenderBullet(window->DrawList, bb.Min + ImVec2(style.FramePadding.x + g.FontSize * 0.5f, line_height * 0.5f), text_col);
    SameLine(0, style.FramePadding.x * 2.0f);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Low-level Layout helpers
//-------------------------------------------------------------------------
// - Spacing()
// - Dummy()
// - NewLine()
// - AlignTextToFramePadding()
// - SeparatorEx() [Internal]
// - Separator()
// - SplitterBehavior() [Internal]
// - ShrinkWidths() [Internal]
//-------------------------------------------------------------------------

void ImGui::Spacing()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    ItemSize(ImVec2(0, 0));
}

void ImGui::Dummy(const ImVec2& size)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ItemSize(size);
    ItemAdd(bb, 0);
}

void ImGui::NewLine()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiLayoutType backup_layout_type = window->DC.LayoutType;
    window->DC.LayoutType = ImGuiLayoutType_Vertical;
    if (window->DC.CurrLineSize.y > 0.0f)     // In the event that we are on a line with items that is smaller that FontSize high, we will preserve its height.
        ItemSize(ImVec2(0, 0));
    else
        ItemSize(ImVec2(0.0f, g.FontSize));
    window->DC.LayoutType = backup_layout_type;
}

void ImGui::AlignTextToFramePadding()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    window->DC.CurrLineSize.y = ImMax(window->DC.CurrLineSize.y, g.FontSize + g.Style.FramePadding.y * 2);
    window->DC.CurrLineTextBaseOffset = ImMax(window->DC.CurrLineTextBaseOffset, g.Style.FramePadding.y);
}

// Horizontal/vertical separating line
void ImGui::SeparatorEx(ImGuiSeparatorFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    IM_ASSERT(ImIsPowerOfTwo(flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical)));   // Check that only 1 option is selected

    float thickness_draw = 1.0f;
    float thickness_layout = 0.0f;
    if (flags & ImGuiSeparatorFlags_Vertical)
    {
        // Vertical separator, for menu bars (use current line height). Not exposed because it is misleading and it doesn't have an effect on regular layout.
        float y1 = window->DC.CursorPos.y;
        float y2 = window->DC.CursorPos.y + window->DC.CurrLineSize.y;
        const ImRect bb(ImVec2(window->DC.CursorPos.x, y1), ImVec2(window->DC.CursorPos.x + thickness_draw, y2));
        ItemSize(ImVec2(thickness_layout, 0.0f));
        if (!ItemAdd(bb, 0))
            return;

        // Draw
        window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_Separator));
        if (g.LogEnabled)
            LogText(" |");
    }
    else if (flags & ImGuiSeparatorFlags_Horizontal)
    {
        // Horizontal Separator
        float x1 = window->Pos.x;
        float x2 = window->Pos.x + window->Size.x;

        // FIXME-WORKRECT: old hack (#205) until we decide of consistent behavior with WorkRect/Indent and Separator
        if (g.GroupStack.Size > 0 && g.GroupStack.back().WindowID == window->ID)
            x1 += window->DC.Indent.x;

        ImGuiOldColumns* columns = (flags & ImGuiSeparatorFlags_SpanAllColumns) ? window->DC.CurrentColumns : NULL;
        if (columns)
            PushColumnsBackground();

        // We don't provide our width to the layout so that it doesn't get feed back into AutoFit
        const ImRect bb(ImVec2(x1, window->DC.CursorPos.y), ImVec2(x2, window->DC.CursorPos.y + thickness_draw));
        ItemSize(ImVec2(0.0f, thickness_layout));
        const bool item_visible = ItemAdd(bb, 0);
        if (item_visible)
        {
            // Draw
            window->DrawList->AddLine(bb.Min, ImVec2(bb.Max.x, bb.Min.y), GetColorU32(ImGuiCol_Separator));
            if (g.LogEnabled)
                LogRenderedText(&bb.Min, "--------------------------------\n");

        }
        if (columns)
        {
            PopColumnsBackground();
            columns->LineMinY = window->DC.CursorPos.y;
        }
    }
}

void ImGui::Separator()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    // Those flags should eventually be overridable by the user
    ImGuiSeparatorFlags flags = (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
    flags |= ImGuiSeparatorFlags_SpanAllColumns;
    SeparatorEx(flags);
}

// Using 'hover_visibility_delay' allows us to hide the highlight and mouse cursor for a short time, which can be convenient to reduce visual noise.
bool ImGui::SplitterBehavior(const ImRect& bb, ImGuiID id, ImGuiAxis axis, float* size1, float* size2, float min_size1, float min_size2, float hover_extend, float hover_visibility_delay)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const ImGuiItemFlags item_flags_backup = g.CurrentItemFlags;
    g.CurrentItemFlags |= ImGuiItemFlags_NoNav | ImGuiItemFlags_NoNavDefaultFocus;
    bool item_add = ItemAdd(bb, id);
    g.CurrentItemFlags = item_flags_backup;
    if (!item_add)
        return false;

    bool hovered, held;
    ImRect bb_interact = bb;
    bb_interact.Expand(axis == ImGuiAxis_Y ? ImVec2(0.0f, hover_extend) : ImVec2(hover_extend, 0.0f));
    ButtonBehavior(bb_interact, id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_AllowItemOverlap);
    if (g.ActiveId != id)
        SetItemAllowOverlap();

    if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id && g.HoveredIdTimer >= hover_visibility_delay))
        SetMouseCursor(axis == ImGuiAxis_Y ? ImGuiMouseCursor_ResizeNS : ImGuiMouseCursor_ResizeEW);

    ImRect bb_render = bb;
    if (held)
    {
        ImVec2 mouse_delta_2d = g.IO.MousePos - g.ActiveIdClickOffset - bb_interact.Min;
        float mouse_delta = (axis == ImGuiAxis_Y) ? mouse_delta_2d.y : mouse_delta_2d.x;

        // Minimum pane size
        float size_1_maximum_delta = ImMax(0.0f, *size1 - min_size1);
        float size_2_maximum_delta = ImMax(0.0f, *size2 - min_size2);
        if (mouse_delta < -size_1_maximum_delta)
            mouse_delta = -size_1_maximum_delta;
        if (mouse_delta > size_2_maximum_delta)
            mouse_delta = size_2_maximum_delta;

        // Apply resize
        if (mouse_delta != 0.0f)
        {
            if (mouse_delta < 0.0f)
                IM_ASSERT(*size1 + mouse_delta >= min_size1);
            if (mouse_delta > 0.0f)
                IM_ASSERT(*size2 - mouse_delta >= min_size2);
            *size1 += mouse_delta;
            *size2 -= mouse_delta;
            bb_render.Translate((axis == ImGuiAxis_X) ? ImVec2(mouse_delta, 0.0f) : ImVec2(0.0f, mouse_delta));
            MarkItemEdited(id);
        }
    }

    // Render
    const ImU32 col = GetColorU32(held ? ImGuiCol_SeparatorActive : (hovered && g.HoveredIdTimer >= hover_visibility_delay) ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator);
    window->DrawList->AddRectFilled(bb_render.Min, bb_render.Max, col, 0.0f);

    return held;
}

static int IMGUI_CDECL ShrinkWidthItemComparer(const void* lhs, const void* rhs)
{
    const ImGuiShrinkWidthItem* a = (const ImGuiShrinkWidthItem*)lhs;
    const ImGuiShrinkWidthItem* b = (const ImGuiShrinkWidthItem*)rhs;
    if (int d = (int)(b->Width - a->Width))
        return d;
    return (b->Index - a->Index);
}

// Shrink excess width from a set of item, by removing width from the larger items first.
// Set items Width to -1.0f to disable shrinking this item.
void ImGui::ShrinkWidths(ImGuiShrinkWidthItem* items, int count, float width_excess)
{
    if (count == 1)
    {
        if (items[0].Width >= 0.0f)
            items[0].Width = ImMax(items[0].Width - width_excess, 1.0f);
        return;
    }
    ImQsort(items, (size_t)count, sizeof(ImGuiShrinkWidthItem), ShrinkWidthItemComparer);
    int count_same_width = 1;
    while (width_excess > 0.0f && count_same_width < count)
    {
        while (count_same_width < count && items[0].Width <= items[count_same_width].Width)
            count_same_width++;
        float max_width_to_remove_per_item = (count_same_width < count && items[count_same_width].Width >= 0.0f) ? (items[0].Width - items[count_same_width].Width) : (items[0].Width - 1.0f);
        if (max_width_to_remove_per_item <= 0.0f)
            break;
        float width_to_remove_per_item = ImMin(width_excess / count_same_width, max_width_to_remove_per_item);
        for (int item_n = 0; item_n < count_same_width; item_n++)
            items[item_n].Width -= width_to_remove_per_item;
        width_excess -= width_to_remove_per_item * count_same_width;
    }

    // Round width and redistribute remainder left-to-right (could make it an option of the function?)
    // Ensure that e.g. the right-most tab of a shrunk tab-bar always reaches exactly at the same distance from the right-most edge of the tab bar separator.
    width_excess = 0.0f;
    for (int n = 0; n < count; n++)
    {
        float width_rounded = ImFloor(items[n].Width);
        width_excess += items[n].Width - width_rounded;
        items[n].Width = width_rounded;
    }
    if (width_excess > 0.0f)
        for (int n = 0; n < count; n++)
            if (items[n].Index < (int)(width_excess + 0.01f))
                items[n].Width += 1.0f;
}

// // Getter for the old Combo() API: const char*[]
static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
    const char* const* items = (const char* const*)data;
    if (out_text)
        *out_text = items[idx];
    return true;
}

//-------------------------------------------------------------------------
// [SECTION] Data Type and Data Formatting Helpers [Internal]
//-------------------------------------------------------------------------
// - PatchFormatStringFloatToInt()
// - DataTypeGetInfo()
// - DataTypeFormatString()
// - DataTypeApplyOp()
// - DataTypeApplyOpFromText()
// - DataTypeClamp()
// - GetMinimumStepAtDecimalPrecision
// - RoundScalarWithFormat<>()
//-------------------------------------------------------------------------

static const ImGuiDataTypeInfo GDataTypeInfo[] =
{
    { sizeof(char),             "S8",   "%d",   "%d"    },  // ImGuiDataType_S8
    { sizeof(unsigned char),    "U8",   "%u",   "%u"    },
    { sizeof(short),            "S16",  "%d",   "%d"    },  // ImGuiDataType_S16
    { sizeof(unsigned short),   "U16",  "%u",   "%u"    },
    { sizeof(int),              "S32",  "%d",   "%d"    },  // ImGuiDataType_S32
    { sizeof(unsigned int),     "U32",  "%u",   "%u"    },
#ifdef _MSC_VER
    { sizeof(ImS64),            "S64",  "%I64d","%I64d" },  // ImGuiDataType_S64
    { sizeof(ImU64),            "U64",  "%I64u","%I64u" },
#else
    { sizeof(ImS64),            "S64",  "%lld", "%lld"  },  // ImGuiDataType_S64
    { sizeof(ImU64),            "U64",  "%llu", "%llu"  },
#endif
    { sizeof(float),            "float", "%.3f","%f"    },  // ImGuiDataType_Float (float are promoted to double in va_arg)
    { sizeof(double),           "double","%f",  "%lf"   },  // ImGuiDataType_Double
};

IM_STATIC_ASSERT(IM_ARRAYSIZE(GDataTypeInfo) == ImGuiDataType_COUNT);

// FIXME-LEGACY: Prior to 1.61 our DragInt() function internally used floats and because of this the compile-time default value for format was "%.0f".
// Even though we changed the compile-time default, we expect users to have carried %f around, which would break the display of DragInt() calls.
// To honor backward compatibility we are rewriting the format string, unless IMGUI_DISABLE_OBSOLETE_FUNCTIONS is enabled. What could possibly go wrong?!
static const char* PatchFormatStringFloatToInt(const char* fmt)
{
    if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' && fmt[4] == 0) // Fast legacy path for "%.0f" which is expected to be the most common case.
        return "%d";
    const char* fmt_start = ImParseFormatFindStart(fmt);    // Find % (if any, and ignore %%)
    const char* fmt_end = ImParseFormatFindEnd(fmt_start);  // Find end of format specifier, which itself is an exercise of confidence/recklessness (because snprintf is dependent on libc or user).
    if (fmt_end > fmt_start && fmt_end[-1] == 'f')
    {
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        if (fmt_start == fmt && fmt_end[0] == 0)
            return "%d";
        ImGuiContext& g = *GImGui;
        ImFormatString(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), "%.*s%%d%s", (int)(fmt_start - fmt), fmt, fmt_end); // Honor leading and trailing decorations, but lose alignment/precision.
        return g.TempBuffer;
#else
        IM_ASSERT(0 && "DragInt(): Invalid format string!"); // Old versions used a default parameter of "%.0f", please replace with e.g. "%d"
#endif
    }
    return fmt;
}

const ImGuiDataTypeInfo* ImGui::DataTypeGetInfo(ImGuiDataType data_type)
{
    IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
    return &GDataTypeInfo[data_type];
}

int ImGui::DataTypeFormatString(char* buf, int buf_size, ImGuiDataType data_type, const void* p_data, const char* format)
{
    // Signedness doesn't matter when pushing integer arguments
    if (data_type == ImGuiDataType_S32 || data_type == ImGuiDataType_U32)
        return ImFormatString(buf, buf_size, format, *(const ImU32*)p_data);
    if (data_type == ImGuiDataType_S64 || data_type == ImGuiDataType_U64)
        return ImFormatString(buf, buf_size, format, *(const ImU64*)p_data);
    if (data_type == ImGuiDataType_Float)
        return ImFormatString(buf, buf_size, format, *(const float*)p_data);
    if (data_type == ImGuiDataType_Double)
        return ImFormatString(buf, buf_size, format, *(const double*)p_data);
    if (data_type == ImGuiDataType_S8)
        return ImFormatString(buf, buf_size, format, *(const ImS8*)p_data);
    if (data_type == ImGuiDataType_U8)
        return ImFormatString(buf, buf_size, format, *(const ImU8*)p_data);
    if (data_type == ImGuiDataType_S16)
        return ImFormatString(buf, buf_size, format, *(const ImS16*)p_data);
    if (data_type == ImGuiDataType_U16)
        return ImFormatString(buf, buf_size, format, *(const ImU16*)p_data);
    IM_ASSERT(0);
    return 0;
}

void ImGui::DataTypeApplyOp(ImGuiDataType data_type, int op, void* output, const void* arg1, const void* arg2)
{
    IM_ASSERT(op == '+' || op == '-');
    switch (data_type)
    {
        case ImGuiDataType_S8:
            if (op == '+') { *(ImS8*)output  = ImAddClampOverflow(*(const ImS8*)arg1,  *(const ImS8*)arg2,  IM_S8_MIN,  IM_S8_MAX); }
            if (op == '-') { *(ImS8*)output  = ImSubClampOverflow(*(const ImS8*)arg1,  *(const ImS8*)arg2,  IM_S8_MIN,  IM_S8_MAX); }
            return;
        case ImGuiDataType_U8:
            if (op == '+') { *(ImU8*)output  = ImAddClampOverflow(*(const ImU8*)arg1,  *(const ImU8*)arg2,  IM_U8_MIN,  IM_U8_MAX); }
            if (op == '-') { *(ImU8*)output  = ImSubClampOverflow(*(const ImU8*)arg1,  *(const ImU8*)arg2,  IM_U8_MIN,  IM_U8_MAX); }
            return;
        case ImGuiDataType_S16:
            if (op == '+') { *(ImS16*)output = ImAddClampOverflow(*(const ImS16*)arg1, *(const ImS16*)arg2, IM_S16_MIN, IM_S16_MAX); }
            if (op == '-') { *(ImS16*)output = ImSubClampOverflow(*(const ImS16*)arg1, *(const ImS16*)arg2, IM_S16_MIN, IM_S16_MAX); }
            return;
        case ImGuiDataType_U16:
            if (op == '+') { *(ImU16*)output = ImAddClampOverflow(*(const ImU16*)arg1, *(const ImU16*)arg2, IM_U16_MIN, IM_U16_MAX); }
            if (op == '-') { *(ImU16*)output = ImSubClampOverflow(*(const ImU16*)arg1, *(const ImU16*)arg2, IM_U16_MIN, IM_U16_MAX); }
            return;
        case ImGuiDataType_S32:
            if (op == '+') { *(ImS32*)output = ImAddClampOverflow(*(const ImS32*)arg1, *(const ImS32*)arg2, IM_S32_MIN, IM_S32_MAX); }
            if (op == '-') { *(ImS32*)output = ImSubClampOverflow(*(const ImS32*)arg1, *(const ImS32*)arg2, IM_S32_MIN, IM_S32_MAX); }
            return;
        case ImGuiDataType_U32:
            if (op == '+') { *(ImU32*)output = ImAddClampOverflow(*(const ImU32*)arg1, *(const ImU32*)arg2, IM_U32_MIN, IM_U32_MAX); }
            if (op == '-') { *(ImU32*)output = ImSubClampOverflow(*(const ImU32*)arg1, *(const ImU32*)arg2, IM_U32_MIN, IM_U32_MAX); }
            return;
        case ImGuiDataType_S64:
            if (op == '+') { *(ImS64*)output = ImAddClampOverflow(*(const ImS64*)arg1, *(const ImS64*)arg2, IM_S64_MIN, IM_S64_MAX); }
            if (op == '-') { *(ImS64*)output = ImSubClampOverflow(*(const ImS64*)arg1, *(const ImS64*)arg2, IM_S64_MIN, IM_S64_MAX); }
            return;
        case ImGuiDataType_U64:
            if (op == '+') { *(ImU64*)output = ImAddClampOverflow(*(const ImU64*)arg1, *(const ImU64*)arg2, IM_U64_MIN, IM_U64_MAX); }
            if (op == '-') { *(ImU64*)output = ImSubClampOverflow(*(const ImU64*)arg1, *(const ImU64*)arg2, IM_U64_MIN, IM_U64_MAX); }
            return;
        case ImGuiDataType_Float:
            if (op == '+') { *(float*)output = *(const float*)arg1 + *(const float*)arg2; }
            if (op == '-') { *(float*)output = *(const float*)arg1 - *(const float*)arg2; }
            return;
        case ImGuiDataType_Double:
            if (op == '+') { *(double*)output = *(const double*)arg1 + *(const double*)arg2; }
            if (op == '-') { *(double*)output = *(const double*)arg1 - *(const double*)arg2; }
            return;
        case ImGuiDataType_COUNT: break;
    }
    IM_ASSERT(0);
}

// User can input math operators (e.g. +100) to edit a numerical values.
// NB: This is _not_ a full expression evaluator. We should probably add one and replace this dumb mess..
bool ImGui::DataTypeApplyOpFromText(const char* buf, const char* initial_value_buf, ImGuiDataType data_type, void* p_data, const char* format)
{
    while (ImCharIsBlankA(*buf))
        buf++;

    // We don't support '-' op because it would conflict with inputing negative value.
    // Instead you can use +-100 to subtract from an existing value
    char op = buf[0];
    if (op == '+' || op == '*' || op == '/')
    {
        buf++;
        while (ImCharIsBlankA(*buf))
            buf++;
    }
    else
    {
        op = 0;
    }
    if (!buf[0])
        return false;

    // Copy the value in an opaque buffer so we can compare at the end of the function if it changed at all.
    const ImGuiDataTypeInfo* type_info = DataTypeGetInfo(data_type);
    ImGuiDataTypeTempStorage data_backup;
    memcpy(&data_backup, p_data, type_info->Size);

    if (format == NULL)
        format = type_info->ScanFmt;

    // FIXME-LEGACY: The aim is to remove those operators and write a proper expression evaluator at some point..
    int arg1i = 0;
    if (data_type == ImGuiDataType_S32)
    {
        int* v = (int*)p_data;
        int arg0i = *v;
        float arg1f = 0.0f;
        if (op && sscanf(initial_value_buf, format, &arg0i) < 1)
            return false;
        // Store operand in a float so we can use fractional value for multipliers (*1.1), but constant always parsed as integer so we can fit big integers (e.g. 2000000003) past float precision
        if (op == '+')      { if (sscanf(buf, "%d", &arg1i)) *v = (int)(arg0i + arg1i); }                   // Add (use "+-" to subtract)
        else if (op == '*') { if (sscanf(buf, "%f", &arg1f)) *v = (int)(arg0i * arg1f); }                   // Multiply
        else if (op == '/') { if (sscanf(buf, "%f", &arg1f) && arg1f != 0.0f) *v = (int)(arg0i / arg1f); }  // Divide
        else                { if (sscanf(buf, format, &arg1i) == 1) *v = arg1i; }                           // Assign constant
    }
    else if (data_type == ImGuiDataType_Float)
    {
        // For floats we have to ignore format with precision (e.g. "%.2f") because sscanf doesn't take them in
        format = "%f";
        float* v = (float*)p_data;
        float arg0f = *v, arg1f = 0.0f;
        if (op && sscanf(initial_value_buf, format, &arg0f) < 1)
            return false;
        if (sscanf(buf, format, &arg1f) < 1)
            return false;
        if (op == '+')      { *v = arg0f + arg1f; }                    // Add (use "+-" to subtract)
        else if (op == '*') { *v = arg0f * arg1f; }                    // Multiply
        else if (op == '/') { if (arg1f != 0.0f) *v = arg0f / arg1f; } // Divide
        else                { *v = arg1f; }                            // Assign constant
    }
    else if (data_type == ImGuiDataType_Double)
    {
        format = "%lf"; // scanf differentiate float/double unlike printf which forces everything to double because of ellipsis
        double* v = (double*)p_data;
        double arg0f = *v, arg1f = 0.0;
        if (op && sscanf(initial_value_buf, format, &arg0f) < 1)
            return false;
        if (sscanf(buf, format, &arg1f) < 1)
            return false;
        if (op == '+')      { *v = arg0f + arg1f; }                    // Add (use "+-" to subtract)
        else if (op == '*') { *v = arg0f * arg1f; }                    // Multiply
        else if (op == '/') { if (arg1f != 0.0f) *v = arg0f / arg1f; } // Divide
        else                { *v = arg1f; }                            // Assign constant
    }
    else if (data_type == ImGuiDataType_U32 || data_type == ImGuiDataType_S64 || data_type == ImGuiDataType_U64)
    {
        // All other types assign constant
        // We don't bother handling support for legacy operators since they are a little too crappy. Instead we will later implement a proper expression evaluator in the future.
        if (sscanf(buf, format, p_data) < 1)
            return false;
    }
    else
    {
        // Small types need a 32-bit buffer to receive the result from scanf()
        int v32;
        if (sscanf(buf, format, &v32) < 1)
            return false;
        if (data_type == ImGuiDataType_S8)
            *(ImS8*)p_data = (ImS8)ImClamp(v32, (int)IM_S8_MIN, (int)IM_S8_MAX);
        else if (data_type == ImGuiDataType_U8)
            *(ImU8*)p_data = (ImU8)ImClamp(v32, (int)IM_U8_MIN, (int)IM_U8_MAX);
        else if (data_type == ImGuiDataType_S16)
            *(ImS16*)p_data = (ImS16)ImClamp(v32, (int)IM_S16_MIN, (int)IM_S16_MAX);
        else if (data_type == ImGuiDataType_U16)
            *(ImU16*)p_data = (ImU16)ImClamp(v32, (int)IM_U16_MIN, (int)IM_U16_MAX);
        else
            IM_ASSERT(0);
    }

    return memcmp(&data_backup, p_data, type_info->Size) != 0;
}

template<typename T>
static int DataTypeCompareT(const T* lhs, const T* rhs)
{
    if (*lhs < *rhs) return -1;
    if (*lhs > *rhs) return +1;
    return 0;
}

int ImGui::DataTypeCompare(ImGuiDataType data_type, const void* arg_1, const void* arg_2)
{
    switch (data_type)
    {
    case ImGuiDataType_S8:     return DataTypeCompareT<ImS8  >((const ImS8*  )arg_1, (const ImS8*  )arg_2);
    case ImGuiDataType_U8:     return DataTypeCompareT<ImU8  >((const ImU8*  )arg_1, (const ImU8*  )arg_2);
    case ImGuiDataType_S16:    return DataTypeCompareT<ImS16 >((const ImS16* )arg_1, (const ImS16* )arg_2);
    case ImGuiDataType_U16:    return DataTypeCompareT<ImU16 >((const ImU16* )arg_1, (const ImU16* )arg_2);
    case ImGuiDataType_S32:    return DataTypeCompareT<ImS32 >((const ImS32* )arg_1, (const ImS32* )arg_2);
    case ImGuiDataType_U32:    return DataTypeCompareT<ImU32 >((const ImU32* )arg_1, (const ImU32* )arg_2);
    case ImGuiDataType_S64:    return DataTypeCompareT<ImS64 >((const ImS64* )arg_1, (const ImS64* )arg_2);
    case ImGuiDataType_U64:    return DataTypeCompareT<ImU64 >((const ImU64* )arg_1, (const ImU64* )arg_2);
    case ImGuiDataType_Float:  return DataTypeCompareT<float >((const float* )arg_1, (const float* )arg_2);
    case ImGuiDataType_Double: return DataTypeCompareT<double>((const double*)arg_1, (const double*)arg_2);
    case ImGuiDataType_COUNT:  break;
    }
    IM_ASSERT(0);
    return 0;
}

template<typename T>
static bool DataTypeClampT(T* v, const T* v_min, const T* v_max)
{
    // Clamp, both sides are optional, return true if modified
    if (v_min && *v < *v_min) { *v = *v_min; return true; }
    if (v_max && *v > *v_max) { *v = *v_max; return true; }
    return false;
}

bool ImGui::DataTypeClamp(ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max)
{
    switch (data_type)
    {
    case ImGuiDataType_S8:     return DataTypeClampT<ImS8  >((ImS8*  )p_data, (const ImS8*  )p_min, (const ImS8*  )p_max);
    case ImGuiDataType_U8:     return DataTypeClampT<ImU8  >((ImU8*  )p_data, (const ImU8*  )p_min, (const ImU8*  )p_max);
    case ImGuiDataType_S16:    return DataTypeClampT<ImS16 >((ImS16* )p_data, (const ImS16* )p_min, (const ImS16* )p_max);
    case ImGuiDataType_U16:    return DataTypeClampT<ImU16 >((ImU16* )p_data, (const ImU16* )p_min, (const ImU16* )p_max);
    case ImGuiDataType_S32:    return DataTypeClampT<ImS32 >((ImS32* )p_data, (const ImS32* )p_min, (const ImS32* )p_max);
    case ImGuiDataType_U32:    return DataTypeClampT<ImU32 >((ImU32* )p_data, (const ImU32* )p_min, (const ImU32* )p_max);
    case ImGuiDataType_S64:    return DataTypeClampT<ImS64 >((ImS64* )p_data, (const ImS64* )p_min, (const ImS64* )p_max);
    case ImGuiDataType_U64:    return DataTypeClampT<ImU64 >((ImU64* )p_data, (const ImU64* )p_min, (const ImU64* )p_max);
    case ImGuiDataType_Float:  return DataTypeClampT<float >((float* )p_data, (const float* )p_min, (const float* )p_max);
    case ImGuiDataType_Double: return DataTypeClampT<double>((double*)p_data, (const double*)p_min, (const double*)p_max);
    case ImGuiDataType_COUNT:  break;
    }
    IM_ASSERT(0);
    return false;
}


template<typename TYPE>
static const char* ImAtoi(const char* src, TYPE* output)
{
    int negative = 0;
    if (*src == '-') { negative = 1; src++; }
    if (*src == '+') { src++; }
    TYPE v = 0;
    while (*src >= '0' && *src <= '9')
        v = (v * 10) + (*src++ - '0');
    *output = negative ? -v : v;
    return src;
}


#endif // IMGUI_DISABLE_OBSOLETE_FUNCTIONS



//-------------------------------------------------------------------------
// [SECTION] Widgets: InputScalar, InputFloat, InputInt, etc.
//-------------------------------------------------------------------------
// - ImParseFormatFindStart() [Internal]
// - ImParseFormatFindEnd() [Internal]
// - ImParseFormatTrimDecorations() [Internal]
// - ImParseFormatPrecision() [Internal]
// - TempInputTextScalar() [Internal]
// - InputScalar()
// - InputScalarN()
// - InputFloat()
// - InputFloat2()
// - InputFloat3()
// - InputFloat4()
// - InputInt()
// - InputInt2()
// - InputInt3()
// - InputInt4()
// - InputDouble()
//-------------------------------------------------------------------------

// We don't use strchr() because our strings are usually very short and often start with '%'
const char* ImParseFormatFindStart(const char* fmt)
{
    while (char c = fmt[0])
    {
        if (c == '%' && fmt[1] != '%')
            return fmt;
        else if (c == '%')
            fmt++;
        fmt++;
    }
    return fmt;
}

const char* ImParseFormatFindEnd(const char* fmt)
{
    // Printf/scanf types modifiers: I/L/h/j/l/t/w/z. Other uppercase letters qualify as types aka end of the format.
    if (fmt[0] != '%')
        return fmt;
    const unsigned int ignored_uppercase_mask = (1 << ('I'-'A')) | (1 << ('L'-'A'));
    const unsigned int ignored_lowercase_mask = (1 << ('h'-'a')) | (1 << ('j'-'a')) | (1 << ('l'-'a')) | (1 << ('t'-'a')) | (1 << ('w'-'a')) | (1 << ('z'-'a'));
    for (char c; (c = *fmt) != 0; fmt++)
    {
        if (c >= 'A' && c <= 'Z' && ((1 << (c - 'A')) & ignored_uppercase_mask) == 0)
            return fmt + 1;
        if (c >= 'a' && c <= 'z' && ((1 << (c - 'a')) & ignored_lowercase_mask) == 0)
            return fmt + 1;
    }
    return fmt;
}

// Extract the format out of a format string with leading or trailing decorations
//  fmt = "blah blah"  -> return fmt
//  fmt = "%.3f"       -> return fmt
//  fmt = "hello %.3f" -> return fmt + 6
//  fmt = "%.3f hello" -> return buf written with "%.3f"
const char* ImParseFormatTrimDecorations(const char* fmt, char* buf, size_t buf_size)
{
    const char* fmt_start = ImParseFormatFindStart(fmt);
    if (fmt_start[0] != '%')
        return fmt;
    const char* fmt_end = ImParseFormatFindEnd(fmt_start);
    if (fmt_end[0] == 0) // If we only have leading decoration, we don't need to copy the data.
        return fmt_start;
    ImStrncpy(buf, fmt_start, ImMin((size_t)(fmt_end - fmt_start) + 1, buf_size));
    return buf;
}

// Parse display precision back from the display format string
// FIXME: This is still used by some navigation code path to infer a minimum tweak step, but we should aim to rework widgets so it isn't needed.
int ImParseFormatPrecision(const char* fmt, int default_precision)
{
    fmt = ImParseFormatFindStart(fmt);
    if (fmt[0] != '%')
        return default_precision;
    fmt++;
    while (*fmt >= '0' && *fmt <= '9')
        fmt++;
    int precision = INT_MAX;
    if (*fmt == '.')
    {
        fmt = ImAtoi<int>(fmt + 1, &precision);
        if (precision < 0 || precision > 99)
            precision = default_precision;
    }
    if (*fmt == 'e' || *fmt == 'E') // Maximum precision with scientific notation
        precision = -1;
    if ((*fmt == 'g' || *fmt == 'G') && precision == INT_MAX)
        precision = -1;
    return (precision == INT_MAX) ? default_precision : precision;
}

// // Create text input in place of another active widget (e.g. used when doing a CTRL+Click on drag/slider widgets)
// // FIXME: Facilitate using this in variety of other situations.
// bool ImGui::TempInputText(const ImRect& bb, ImGuiID id, const char* label, char* buf, int buf_size, ImGuiInputTextFlags flags)
// {
//     // On the first frame, g.TempInputTextId == 0, then on subsequent frames it becomes == id.
//     // We clear ActiveID on the first frame to allow the InputText() taking it back.
//     ImGuiContext& g = *GImGui;
//     const bool init = (g.TempInputId != id);
//     if (init)
//         ClearActiveID();

//     g.CurrentWindow->DC.CursorPos = bb.Min;
//     bool value_changed = InputTextEx(label, NULL, buf, buf_size, bb.GetSize(), flags | ImGuiInputTextFlags_MergedItem);
//     if (init)
//     {
//         // First frame we started displaying the InputText widget, we expect it to take the active id.
//         IM_ASSERT(g.ActiveId == id);
//         g.TempInputId = g.ActiveId;
//     }
//     return value_changed;
// }

// // Note that Drag/Slider functions are only forwarding the min/max values clamping values if the ImGuiSliderFlags_AlwaysClamp flag is set!
// // This is intended: this way we allow CTRL+Click manual input to set a value out of bounds, for maximum flexibility.
// // However this may not be ideal for all uses, as some user code may break on out of bound values.
// bool ImGui::TempInputScalar(const ImRect& bb, ImGuiID id, const char* label, ImGuiDataType data_type, void* p_data, const char* format, const void* p_clamp_min, const void* p_clamp_max)
// {
//     ImGuiContext& g = *GImGui;

//     char fmt_buf[32];
//     char data_buf[32];
//     format = ImParseFormatTrimDecorations(format, fmt_buf, IM_ARRAYSIZE(fmt_buf));
//     DataTypeFormatString(data_buf, IM_ARRAYSIZE(data_buf), data_type, p_data, format);
//     ImStrTrimBlanks(data_buf);

//     ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoMarkEdited;
//     flags |= ((data_type == ImGuiDataType_Float || data_type == ImGuiDataType_Double) ? ImGuiInputTextFlags_CharsScientific : ImGuiInputTextFlags_CharsDecimal);
//     bool value_changed = false;
//     if (TempInputText(bb, id, label, data_buf, IM_ARRAYSIZE(data_buf), flags))
//     {
//         // Backup old value
//         size_t data_type_size = DataTypeGetInfo(data_type)->Size;
//         ImGuiDataTypeTempStorage data_backup;
//         memcpy(&data_backup, p_data, data_type_size);

//         // Apply new value (or operations) then clamp
//         DataTypeApplyOpFromText(data_buf, g.InputTextState.InitialTextA.Data, data_type, p_data, NULL);
//         if (p_clamp_min || p_clamp_max)
//         {
//             if (p_clamp_min && p_clamp_max && DataTypeCompare(data_type, p_clamp_min, p_clamp_max) > 0)
//                 ImSwap(p_clamp_min, p_clamp_max);
//             DataTypeClamp(data_type, p_data, p_clamp_min, p_clamp_max);
//         }

//         // Only mark as edited if new value is different
//         value_changed = memcmp(&data_backup, p_data, data_type_size) != 0;
//         if (value_changed)
//             MarkItemEdited(id);
//     }
//     return value_changed;
// }

// // Note: p_data, p_step, p_step_fast are _pointers_ to a memory address holding the data. For an Input widget, p_step and p_step_fast are optional.
// // Read code of e.g. InputFloat(), InputInt() etc. or examples in 'Demo->Widgets->Data Types' to understand how to use this function directly.
// bool ImGui::InputScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     ImGuiContext& g = *GImGui;
//     ImGuiStyle& style = g.Style;

//     if (format == NULL)
//         format = DataTypeGetInfo(data_type)->PrintFmt;

//     char buf[64];
//     DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, p_data, format);

//     bool value_changed = false;
//     if ((flags & (ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsScientific)) == 0)
//         flags |= ImGuiInputTextFlags_CharsDecimal;
//     flags |= ImGuiInputTextFlags_AutoSelectAll;
//     flags |= ImGuiInputTextFlags_NoMarkEdited;  // We call MarkItemEdited() ourselves by comparing the actual data rather than the string.

//     if (p_step != NULL)
//     {
//         const float button_size = GetFrameHeight();

//         BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
//         PushID(label);
//         SetNextItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
//         if (InputText("", buf, IM_ARRAYSIZE(buf), flags)) // PushId(label) + "" gives us the expected ID from outside point of view
//             value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);

//         // Step buttons
//         const ImVec2 backup_frame_padding = style.FramePadding;
//         style.FramePadding.x = style.FramePadding.y;
//         ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;
//         if (flags & ImGuiInputTextFlags_ReadOnly)
//             button_flags |= ImGuiButtonFlags_Disabled;
//         SameLine(0, style.ItemInnerSpacing.x);
//         if (ButtonEx("-", ImVec2(button_size, button_size), button_flags))
//         {
//             DataTypeApplyOp(data_type, '-', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
//             value_changed = true;
//         }
//         SameLine(0, style.ItemInnerSpacing.x);
//         if (ButtonEx("+", ImVec2(button_size, button_size), button_flags))
//         {
//             DataTypeApplyOp(data_type, '+', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
//             value_changed = true;
//         }

//         const char* label_end = FindRenderedTextEnd(label);
//         if (label != label_end)
//         {
//             SameLine(0, style.ItemInnerSpacing.x);
//             TextEx(label, label_end);
//         }
//         style.FramePadding = backup_frame_padding;

//         PopID();
//         EndGroup();
//     }
//     else
//     {
//         if (InputText(label, buf, IM_ARRAYSIZE(buf), flags))
//             value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);
//     }
//     if (value_changed)
//         MarkItemEdited(window->DC.LastItemId);

//     return value_changed;
// }

// bool ImGui::InputScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     ImGuiContext& g = *GImGui;
//     bool value_changed = false;
//     BeginGroup();
//     PushID(label);
//     PushMultiItemsWidths(components, CalcItemWidth());
//     size_t type_size = GDataTypeInfo[data_type].Size;
//     for (int i = 0; i < components; i++)
//     {
//         PushID(i);
//         if (i > 0)
//             SameLine(0, g.Style.ItemInnerSpacing.x);
//         value_changed |= InputScalar("", data_type, p_data, p_step, p_step_fast, format, flags);
//         PopID();
//         PopItemWidth();
//         p_data = (void*)((char*)p_data + type_size);
//     }
//     PopID();

//     const char* label_end = FindRenderedTextEnd(label);
//     if (label != label_end)
//     {
//         SameLine(0.0f, g.Style.ItemInnerSpacing.x);
//         TextEx(label, label_end);
//     }

//     EndGroup();
//     return value_changed;
// }

// bool ImGui::InputFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags)
// {
//     flags |= ImGuiInputTextFlags_CharsScientific;
//     return InputScalar(label, ImGuiDataType_Float, (void*)v, (void*)(step > 0.0f ? &step : NULL), (void*)(step_fast > 0.0f ? &step_fast : NULL), format, flags);
// }

// bool ImGui::InputFloat2(const char* label, float v[2], const char* format, ImGuiInputTextFlags flags)
// {
//     return InputScalarN(label, ImGuiDataType_Float, v, 2, NULL, NULL, format, flags);
// }

// bool ImGui::InputFloat3(const char* label, float v[3], const char* format, ImGuiInputTextFlags flags)
// {
//     return InputScalarN(label, ImGuiDataType_Float, v, 3, NULL, NULL, format, flags);
// }

// bool ImGui::InputFloat4(const char* label, float v[4], const char* format, ImGuiInputTextFlags flags)
// {
//     return InputScalarN(label, ImGuiDataType_Float, v, 4, NULL, NULL, format, flags);
// }

// bool ImGui::InputInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags)
// {
//     // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
//     const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
//     return InputScalar(label, ImGuiDataType_S32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
// }

// bool ImGui::InputInt2(const char* label, int v[2], ImGuiInputTextFlags flags)
// {
//     return InputScalarN(label, ImGuiDataType_S32, v, 2, NULL, NULL, "%d", flags);
// }

// bool ImGui::InputInt3(const char* label, int v[3], ImGuiInputTextFlags flags)
// {
//     return InputScalarN(label, ImGuiDataType_S32, v, 3, NULL, NULL, "%d", flags);
// }

// bool ImGui::InputInt4(const char* label, int v[4], ImGuiInputTextFlags flags)
// {
//     return InputScalarN(label, ImGuiDataType_S32, v, 4, NULL, NULL, "%d", flags);
// }

// bool ImGui::InputDouble(const char* label, double* v, double step, double step_fast, const char* format, ImGuiInputTextFlags flags)
// {
//     flags |= ImGuiInputTextFlags_CharsScientific;
//     return InputScalar(label, ImGuiDataType_Double, (void*)v, (void*)(step > 0.0 ? &step : NULL), (void*)(step_fast > 0.0 ? &step_fast : NULL), format, flags);
// }

// //-------------------------------------------------------------------------
// // [SECTION] Widgets: InputText, InputTextMultiline, InputTextWithHint
// //-------------------------------------------------------------------------
// // - InputText()
// // - InputTextWithHint()
// // - InputTextMultiline()
// // - InputTextEx() [Internal]
// //-------------------------------------------------------------------------

// bool ImGui::InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
// {
//     IM_ASSERT(!(flags & ImGuiInputTextFlags_Multiline)); // call InputTextMultiline()
//     return InputTextEx(label, NULL, buf, (int)buf_size, ImVec2(0, 0), flags, callback, user_data);
// }

// bool ImGui::InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
// {
//     return InputTextEx(label, NULL, buf, (int)buf_size, size, flags | ImGuiInputTextFlags_Multiline, callback, user_data);
// }

// bool ImGui::InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
// {
//     IM_ASSERT(!(flags & ImGuiInputTextFlags_Multiline)); // call InputTextMultiline()
//     return InputTextEx(label, hint, buf, (int)buf_size, ImVec2(0, 0), flags, callback, user_data);
// }

// static int InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end)
// {
//     int line_count = 0;
//     const char* s = text_begin;
//     while (char c = *s++) // We are only matching for \n so we can ignore UTF-8 decoding
//         if (c == '\n')
//             line_count++;
//     s--;
//     if (s[0] != '\n' && s[0] != '\r')
//         line_count++;
//     *out_text_end = s;
//     return line_count;
// }

// static ImVec2 InputTextCalcTextSizeW(const ImWchar* text_begin, const ImWchar* text_end, const ImWchar** remaining, ImVec2* out_offset, bool stop_on_new_line)
// {
//     ImGuiContext& g = *GImGui;
//     ImFont* font = g.Font;
//     const float line_height = g.FontSize;
//     const float scale = line_height / font->FontSize;

//     ImVec2 text_size = ImVec2(0, 0);
//     float line_width = 0.0f;

//     const ImWchar* s = text_begin;
//     while (s < text_end)
//     {
//         unsigned int c = (unsigned int)(*s++);
//         if (c == '\n')
//         {
//             text_size.x = ImMax(text_size.x, line_width);
//             text_size.y += line_height;
//             line_width = 0.0f;
//             if (stop_on_new_line)
//                 break;
//             continue;
//         }
//         if (c == '\r')
//             continue;

//         const float char_width = font->GetCharAdvance((ImWchar)c) * scale;
//         line_width += char_width;
//     }

//     if (text_size.x < line_width)
//         text_size.x = line_width;

//     if (out_offset)
//         *out_offset = ImVec2(line_width, text_size.y + line_height);  // offset allow for the possibility of sitting after a trailing \n

//     if (line_width > 0 || text_size.y == 0.0f)                        // whereas size.y will ignore the trailing \n
//         text_size.y += line_height;

//     if (remaining)
//         *remaining = s;

//     return text_size;
// }

// extern  ImVec2 InputTextCalcTextSizeW(const ImWchar* text_begin, const ImWchar* text_end, const ImWchar** remaining, ImVec2* out_offset, bool stop_on_new_line);
// // Wrapper for stb_textedit.h to edit text (our wrapper is for: statically sized buffer, single-line, wchar characters. InputText converts between UTF-8 and wchar)
// namespace ImStb
// {

// static int     STB_TEXTEDIT_STRINGLEN(const ImGuiInputTextState* obj)                             { return obj->CurLenW; }
// static ImWchar STB_TEXTEDIT_GETCHAR(const ImGuiInputTextState* obj, int idx)                      { return obj->TextW[idx]; }
// static float   STB_TEXTEDIT_GETWIDTH(ImGuiInputTextState* obj, int line_start_idx, int char_idx)  { ImWchar c = obj->TextW[line_start_idx + char_idx]; if (c == '\n') return STB_TEXTEDIT_GETWIDTH_NEWLINE; ImGuiContext& g = *GImGui; return g.Font->GetCharAdvance(c) * (g.FontSize / g.Font->FontSize); }
// static int     STB_TEXTEDIT_KEYTOTEXT(int key)                                                    { return key >= 0x200000 ? 0 : key; }
// static ImWchar STB_TEXTEDIT_NEWLINE = '\n';
// static void    STB_TEXTEDIT_LAYOUTROW(StbTexteditRow* r, ImGuiInputTextState* obj, int line_start_idx)
// {
//     const ImWchar* text = obj->TextW.Data;
//     const ImWchar* text_remaining = NULL;
//     const ImVec2 size = InputTextCalcTextSizeW(text + line_start_idx, text + obj->CurLenW, &text_remaining, NULL, true);
//     r->x0 = 0.0f;
//     r->x1 = size.x;
//     r->baseline_y_delta = size.y;
//     r->ymin = 0.0f;
//     r->ymax = size.y;
//     r->num_chars = (int)(text_remaining - (text + line_start_idx));
// }

// // When ImGuiInputTextFlags_Password is set, we don't want actions such as CTRL+Arrow to leak the fact that underlying data are blanks or separators.
// static bool is_separator(unsigned int c)                                        { return ImCharIsBlankW(c) || c==',' || c==';' || c=='(' || c==')' || c=='{' || c=='}' || c=='[' || c==']' || c=='|'; }
// static int  is_word_boundary_from_right(ImGuiInputTextState* obj, int idx)      { if (obj->Flags & ImGuiInputTextFlags_Password) return 0; return idx > 0 ? (is_separator(obj->TextW[idx - 1]) && !is_separator(obj->TextW[idx]) ) : 1; }
// static int  STB_TEXTEDIT_MOVEWORDLEFT_IMPL(ImGuiInputTextState* obj, int idx)   { idx--; while (idx >= 0 && !is_word_boundary_from_right(obj, idx)) idx--; return idx < 0 ? 0 : idx; }
// #ifdef __APPLE__    // FIXME: Move setting to IO structure
// static int  is_word_boundary_from_left(ImGuiInputTextState* obj, int idx)       { if (obj->Flags & ImGuiInputTextFlags_Password) return 0; return idx > 0 ? (!is_separator(obj->TextW[idx - 1]) && is_separator(obj->TextW[idx]) ) : 1; }
// static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(ImGuiInputTextState* obj, int idx)  { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_left(obj, idx)) idx++; return idx > len ? len : idx; }
// #else
// static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(ImGuiInputTextState* obj, int idx)  { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_right(obj, idx)) idx++; return idx > len ? len : idx; }
// #endif
// #define STB_TEXTEDIT_MOVEWORDLEFT   STB_TEXTEDIT_MOVEWORDLEFT_IMPL    // They need to be #define for stb_textedit.h
// #define STB_TEXTEDIT_MOVEWORDRIGHT  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL

// static void STB_TEXTEDIT_DELETECHARS(ImGuiInputTextState* obj, int pos, int n)
// {
//     ImWchar* dst = obj->TextW.Data + pos;

//     // We maintain our buffer length in both UTF-8 and wchar formats
//     obj->Edited = true;
//     obj->CurLenA -= ImTextCountUtf8BytesFromStr(dst, dst + n);
//     obj->CurLenW -= n;

//     // Offset remaining text (FIXME-OPT: Use memmove)
//     const ImWchar* src = obj->TextW.Data + pos + n;
//     while (ImWchar c = *src++)
//         *dst++ = c;
//     *dst = '\0';
// }

// static bool STB_TEXTEDIT_INSERTCHARS(ImGuiInputTextState* obj, int pos, const ImWchar* new_text, int new_text_len)
// {
//     const bool is_resizable = (obj->Flags & ImGuiInputTextFlags_CallbackResize) != 0;
//     const int text_len = obj->CurLenW;
//     IM_ASSERT(pos <= text_len);

//     const int new_text_len_utf8 = ImTextCountUtf8BytesFromStr(new_text, new_text + new_text_len);
//     if (!is_resizable && (new_text_len_utf8 + obj->CurLenA + 1 > obj->BufCapacityA))
//         return false;

//     // Grow internal buffer if needed
//     if (new_text_len + text_len + 1 > obj->TextW.Size)
//     {
//         if (!is_resizable)
//             return false;
//         IM_ASSERT(text_len < obj->TextW.Size);
//         obj->TextW.resize(text_len + ImClamp(new_text_len * 4, 32, ImMax(256, new_text_len)) + 1);
//     }

//     ImWchar* text = obj->TextW.Data;
//     if (pos != text_len)
//         memmove(text + pos + new_text_len, text + pos, (size_t)(text_len - pos) * sizeof(ImWchar));
//     memcpy(text + pos, new_text, (size_t)new_text_len * sizeof(ImWchar));

//     obj->Edited = true;
//     obj->CurLenW += new_text_len;
//     obj->CurLenA += new_text_len_utf8;
//     obj->TextW[obj->CurLenW] = '\0';

//     return true;
// }

// // We don't use an enum so we can build even with conflicting symbols (if another user of stb_textedit.h leak their STB_TEXTEDIT_K_* symbols)
// #define STB_TEXTEDIT_K_LEFT         0x200000 // keyboard input to move cursor left
// #define STB_TEXTEDIT_K_RIGHT        0x200001 // keyboard input to move cursor right
// #define STB_TEXTEDIT_K_UP           0x200002 // keyboard input to move cursor up
// #define STB_TEXTEDIT_K_DOWN         0x200003 // keyboard input to move cursor down
// #define STB_TEXTEDIT_K_LINESTART    0x200004 // keyboard input to move cursor to start of line
// #define STB_TEXTEDIT_K_LINEEND      0x200005 // keyboard input to move cursor to end of line
// #define STB_TEXTEDIT_K_TEXTSTART    0x200006 // keyboard input to move cursor to start of text
// #define STB_TEXTEDIT_K_TEXTEND      0x200007 // keyboard input to move cursor to end of text
// #define STB_TEXTEDIT_K_DELETE       0x200008 // keyboard input to delete selection or character under cursor
// #define STB_TEXTEDIT_K_BACKSPACE    0x200009 // keyboard input to delete selection or character left of cursor
// #define STB_TEXTEDIT_K_UNDO         0x20000A // keyboard input to perform undo
// #define STB_TEXTEDIT_K_REDO         0x20000B // keyboard input to perform redo
// #define STB_TEXTEDIT_K_WORDLEFT     0x20000C // keyboard input to move cursor left one word
// #define STB_TEXTEDIT_K_WORDRIGHT    0x20000D // keyboard input to move cursor right one word
// #define STB_TEXTEDIT_K_PGUP         0x20000E // keyboard input to move cursor up a page
// #define STB_TEXTEDIT_K_PGDOWN       0x20000F // keyboard input to move cursor down a page
// #define STB_TEXTEDIT_K_SHIFT        0x400000

// #define STB_TEXTEDIT_IMPLEMENTATION
// #include "imstb_textedit.h"

// // stb_textedit internally allows for a single undo record to do addition and deletion, but somehow, calling
// // the stb_textedit_paste() function creates two separate records, so we perform it manually. (FIXME: Report to nothings/stb?)
// static void stb_textedit_replace(ImGuiInputTextState* str, STB_TexteditState* state, const STB_TEXTEDIT_CHARTYPE* text, int text_len)
// {
//     stb_text_makeundo_replace(str, state, 0, str->CurLenW, text_len);
//     ImStb::STB_TEXTEDIT_DELETECHARS(str, 0, str->CurLenW);
//     if (text_len <= 0)
//         return;
//     if (ImStb::STB_TEXTEDIT_INSERTCHARS(str, 0, text, text_len))
//     {
//         state->cursor = text_len;
//         state->has_preferred_x = 0;
//         return;
//     }
//     IM_ASSERT(0); // Failed to insert character, normally shouldn't happen because of how we currently use stb_textedit_replace()
// }

// } // namespace ImStb

#include <ImGui/ImGuiStb.h>

void ImGuiInputTextState::OnKeyPressed(int key)
{
    stb_textedit_key(this, &Stb, key);
    CursorFollow = true;
    CursorAnimReset();
}

ImGuiInputTextCallbackData::ImGuiInputTextCallbackData()
{
    memset(this, 0, sizeof(*this));
}

// Public API to manipulate UTF-8 text
// We expose UTF-8 to the user (unlike the STB_TEXTEDIT_* functions which are manipulating wchar)
// FIXME: The existence of this rarely exercised code path is a bit of a nuisance.
void ImGuiInputTextCallbackData::DeleteChars(int pos, int bytes_count)
{
    IM_ASSERT(pos + bytes_count <= BufTextLen);
    char* dst = Buf + pos;
    const char* src = Buf + pos + bytes_count;
    while (char c = *src++)
        *dst++ = c;
    *dst = '\0';

    if (CursorPos >= pos + bytes_count)
        CursorPos -= bytes_count;
    else if (CursorPos >= pos)
        CursorPos = pos;
    SelectionStart = SelectionEnd = CursorPos;
    BufDirty = true;
    BufTextLen -= bytes_count;
}

void ImGuiInputTextCallbackData::InsertChars(int pos, const char* new_text, const char* new_text_end)
{
    const bool is_resizable = (Flags & ImGuiInputTextFlags_CallbackResize) != 0;
    const int new_text_len = new_text_end ? (int)(new_text_end - new_text) : (int)strlen(new_text);
    if (new_text_len + BufTextLen >= BufSize)
    {
        if (!is_resizable)
            return;

        // Contrary to STB_TEXTEDIT_INSERTCHARS() this is working in the UTF8 buffer, hence the mildly similar code (until we remove the U16 buffer altogether!)
        ImGuiContext& g = *GImGui;
        ImGuiInputTextState* edit_state = &g.InputTextState;
        IM_ASSERT(edit_state->ID != 0 && g.ActiveId == edit_state->ID);
        IM_ASSERT(Buf == edit_state->TextA.Data);
        int new_buf_size = BufTextLen + ImClamp(new_text_len * 4, 32, ImMax(256, new_text_len)) + 1;
        edit_state->TextA.reserve(new_buf_size + 1);
        Buf = edit_state->TextA.Data;
        BufSize = edit_state->BufCapacityA = new_buf_size;
    }

    if (BufTextLen != pos)
        memmove(Buf + pos + new_text_len, Buf + pos, (size_t)(BufTextLen - pos));
    memcpy(Buf + pos, new_text, (size_t)new_text_len * sizeof(char));
    Buf[BufTextLen + new_text_len] = '\0';

    if (CursorPos >= pos)
        CursorPos += new_text_len;
    SelectionStart = SelectionEnd = CursorPos;
    BufDirty = true;
    BufTextLen += new_text_len;
}

// // Return false to discard a character.
// static bool InputTextFilterCharacter(unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data, ImGuiInputSource input_source)
// {
//     IM_ASSERT(input_source == ImGuiInputSource_Keyboard || input_source == ImGuiInputSource_Clipboard);
//     unsigned int c = *p_char;

//     // Filter non-printable (NB: isprint is unreliable! see #2467)
//     if (c < 0x20)
//     {
//         bool pass = false;
//         pass |= (c == '\n' && (flags & ImGuiInputTextFlags_Multiline));
//         pass |= (c == '\t' && (flags & ImGuiInputTextFlags_AllowTabInput));
//         if (!pass)
//             return false;
//     }

//     if (input_source != ImGuiInputSource_Clipboard)
//     {
//         // We ignore Ascii representation of delete (emitted from Backspace on OSX, see #2578, #2817)
//         if (c == 127)
//             return false;

//         // Filter private Unicode range. GLFW on OSX seems to send private characters for special keys like arrow keys (FIXME)
//         if (c >= 0xE000 && c <= 0xF8FF)
//             return false;
//     }

//     // Filter Unicode ranges we are not handling in this build
//     if (c > IM_UNICODE_CODEPOINT_MAX)
//         return false;

//     // Generic named filters
//     if (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CharsScientific))
//     {
//         // The libc allows overriding locale, with e.g. 'setlocale(LC_NUMERIC, "de_DE.UTF-8");' which affect the output/input of printf/scanf.
//         // The standard mandate that programs starts in the "C" locale where the decimal point is '.'.
//         // We don't really intend to provide widespread support for it, but out of empathy for people stuck with using odd API, we support the bare minimum aka overriding the decimal point.
//         // Change the default decimal_point with:
//         //   ImGui::GetCurrentContext()->PlatformLocaleDecimalPoint = *localeconv()->decimal_point;
//         ImGuiContext& g = *GImGui;
//         const unsigned c_decimal_point = (unsigned int)g.PlatformLocaleDecimalPoint;

//         // Allow 0-9 . - + * /
//         if (flags & ImGuiInputTextFlags_CharsDecimal)
//             if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') && (c != '/'))
//                 return false;

//         // Allow 0-9 . - + * / e E
//         if (flags & ImGuiInputTextFlags_CharsScientific)
//             if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') && (c != '/') && (c != 'e') && (c != 'E'))
//                 return false;

//         // Allow 0-9 a-F A-F
//         if (flags & ImGuiInputTextFlags_CharsHexadecimal)
//             if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
//                 return false;

//         // Turn a-z into A-Z
//         if (flags & ImGuiInputTextFlags_CharsUppercase)
//             if (c >= 'a' && c <= 'z')
//                 *p_char = (c += (unsigned int)('A' - 'a'));

//         if (flags & ImGuiInputTextFlags_CharsNoBlank)
//             if (ImCharIsBlankW(c))
//                 return false;
//     }

//     // Custom callback filter
//     if (flags & ImGuiInputTextFlags_CallbackCharFilter)
//     {
//         ImGuiInputTextCallbackData callback_data;
//         memset(&callback_data, 0, sizeof(ImGuiInputTextCallbackData));
//         callback_data.EventFlag = ImGuiInputTextFlags_CallbackCharFilter;
//         callback_data.EventChar = (ImWchar)c;
//         callback_data.Flags = flags;
//         callback_data.UserData = user_data;
//         if (callback(&callback_data) != 0)
//             return false;
//         *p_char = callback_data.EventChar;
//         if (!callback_data.EventChar)
//             return false;
//     }

//     return true;
// }

// // Edit a string of text
// // - buf_size account for the zero-terminator, so a buf_size of 6 can hold "Hello" but not "Hello!".
// //   This is so we can easily call InputText() on static arrays using ARRAYSIZE() and to match
// //   Note that in std::string world, capacity() would omit 1 byte used by the zero-terminator.
// // - When active, hold on a privately held copy of the text (and apply back to 'buf'). So changing 'buf' while the InputText is active has no effect.
// // - If you want to use ImGui::InputText() with std::string, see misc/cpp/imgui_stdlib.h
// // (FIXME: Rather confusing and messy function, among the worse part of our codebase, expecting to rewrite a V2 at some point.. Partly because we are
// //  doing UTF8 > U16 > UTF8 conversions on the go to easily interface with stb_textedit. Ideally should stay in UTF-8 all the time. See https://github.com/nothings/stb/issues/188)
// bool ImGui::InputTextEx(const char* label, const char* hint, char* buf, int buf_size, const ImVec2& size_arg, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* callback_user_data)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     IM_ASSERT(buf != NULL && buf_size >= 0);
//     IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackHistory) && (flags & ImGuiInputTextFlags_Multiline)));        // Can't use both together (they both use up/down keys)
//     IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackCompletion) && (flags & ImGuiInputTextFlags_AllowTabInput))); // Can't use both together (they both use tab key)

//     ImGuiContext& g = *GImGui;
//     ImGuiIO& io = g.IO;
//     const ImGuiStyle& style = g.Style;

//     const bool RENDER_SELECTION_WHEN_INACTIVE = false;
//     const bool is_multiline = (flags & ImGuiInputTextFlags_Multiline) != 0;
//     const bool is_readonly = (flags & ImGuiInputTextFlags_ReadOnly) != 0;
//     const bool is_password = (flags & ImGuiInputTextFlags_Password) != 0;
//     const bool is_undoable = (flags & ImGuiInputTextFlags_NoUndoRedo) == 0;
//     const bool is_resizable = (flags & ImGuiInputTextFlags_CallbackResize) != 0;
//     if (is_resizable)
//         IM_ASSERT(callback != NULL); // Must provide a callback if you set the ImGuiInputTextFlags_CallbackResize flag!

//     if (is_multiline) // Open group before calling GetID() because groups tracks id created within their scope,
//         BeginGroup();
//     const ImGuiID id = window->GetID(label);
//     const ImVec2 label_size = CalcTextSize(label, NULL, true);
//     const ImVec2 frame_size = CalcItemSize(size_arg, CalcItemWidth(), (is_multiline ? g.FontSize * 8.0f : label_size.y) + style.FramePadding.y * 2.0f); // Arbitrary default of 8 lines high for multi-line
//     const ImVec2 total_size = ImVec2(frame_size.x + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), frame_size.y);

//     const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
//     const ImRect total_bb(frame_bb.Min, frame_bb.Min + total_size);

//     ImGuiWindow* draw_window = window;
//     ImVec2 inner_size = frame_size;
//     if (is_multiline)
//     {
//         if (!ItemAdd(total_bb, id, &frame_bb, ImGuiItemAddFlags_Focusable))
//         {
//             ItemSize(total_bb, style.FramePadding.y);
//             EndGroup();
//             return false;
//         }

//         // We reproduce the contents of BeginChildFrame() in order to provide 'label' so our window internal data are easier to read/debug.
//         PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_FrameBg]);
//         PushStyleVar(ImGuiStyleVar_ChildRounding, style.FrameRounding);
//         PushStyleVar(ImGuiStyleVar_ChildBorderSize, style.FrameBorderSize);
//         bool child_visible = BeginChildEx(label, id, frame_bb.GetSize(), true, ImGuiWindowFlags_NoMove);
//         PopStyleVar(2);
//         PopStyleColor();
//         if (!child_visible)
//         {
//             EndChild();
//             EndGroup();
//             return false;
//         }
//         draw_window = g.CurrentWindow; // Child window
//         draw_window->DC.NavLayersActiveMaskNext |= (1 << draw_window->DC.NavLayerCurrent); // This is to ensure that EndChild() will display a navigation highlight so we can "enter" into it.
//         draw_window->DC.CursorPos += style.FramePadding;
//         inner_size.x -= draw_window->ScrollbarSizes.x;
//     }
//     else
//     {
//         // Support for internal ImGuiInputTextFlags_MergedItem flag, which could be redesigned as an ItemFlags if needed (with test performed in ItemAdd)
//         ItemSize(total_bb, style.FramePadding.y);
//         if (!(flags & ImGuiInputTextFlags_MergedItem))
//             if (!ItemAdd(total_bb, id, &frame_bb, ImGuiItemAddFlags_Focusable))
//                 return false;
//     }
//     const bool hovered = ItemHoverable(frame_bb, id);
//     if (hovered)
//         g.MouseCursor = ImGuiMouseCursor_TextInput;

//     // We are only allowed to access the state if we are already the active widget.
//     ImGuiInputTextState* state = GetInputTextState(id);

//     const bool focus_requested_by_code = (window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_FocusedByCode) != 0;
//     const bool focus_requested_by_tabbing = (window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_FocusedByTabbing) != 0;

//     const bool user_clicked = hovered && io.MouseClicked[0];
//     const bool user_nav_input_start = (g.ActiveId != id) && ((g.NavInputId == id) || (g.NavActivateId == id && g.NavInputSource == ImGuiInputSource_Keyboard));
//     const bool user_scroll_finish = is_multiline && state != NULL && g.ActiveId == 0 && g.ActiveIdPreviousFrame == GetWindowScrollbarID(draw_window, ImGuiAxis_Y);
//     const bool user_scroll_active = is_multiline && state != NULL && g.ActiveId == GetWindowScrollbarID(draw_window, ImGuiAxis_Y);

//     bool clear_active_id = false;
//     bool select_all = (g.ActiveId != id) && ((flags & ImGuiInputTextFlags_AutoSelectAll) != 0 || user_nav_input_start) && (!is_multiline);

//     float scroll_y = is_multiline ? draw_window->Scroll.y : FLT_MAX;

//     const bool init_changed_specs = (state != NULL && state->Stb.single_line != !is_multiline);
//     const bool init_make_active = (user_clicked || user_scroll_finish || user_nav_input_start || focus_requested_by_code || focus_requested_by_tabbing);
//     const bool init_state = (init_make_active || user_scroll_active);
//     if ((init_state && g.ActiveId != id) || init_changed_specs)
//     {
//         // Access state even if we don't own it yet.
//         state = &g.InputTextState;
//         state->CursorAnimReset();

//         // Take a copy of the initial buffer value (both in original UTF-8 format and converted to wchar)
//         // From the moment we focused we are ignoring the content of 'buf' (unless we are in read-only mode)
//         const int buf_len = (int)strlen(buf);
//         state->InitialTextA.resize(buf_len + 1);    // UTF-8. we use +1 to make sure that .Data is always pointing to at least an empty string.
//         memcpy(state->InitialTextA.Data, buf, buf_len + 1);

//         // Start edition
//         const char* buf_end = NULL;
//         state->TextW.resize(buf_size + 1);          // wchar count <= UTF-8 count. we use +1 to make sure that .Data is always pointing to at least an empty string.
//         state->TextA.resize(0);
//         state->TextAIsValid = false;                // TextA is not valid yet (we will display buf until then)
//         state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, buf_size, buf, NULL, &buf_end);
//         state->CurLenA = (int)(buf_end - buf);      // We can't get the result from ImStrncpy() above because it is not UTF-8 aware. Here we'll cut off malformed UTF-8.

//         // Preserve cursor position and undo/redo stack if we come back to same widget
//         // FIXME: For non-readonly widgets we might be able to require that TextAIsValid && TextA == buf ? (untested) and discard undo stack if user buffer has changed.
//         const bool recycle_state = (state->ID == id && !init_changed_specs);
//         if (recycle_state)
//         {
//             // Recycle existing cursor/selection/undo stack but clamp position
//             // Note a single mouse click will override the cursor/position immediately by calling stb_textedit_click handler.
//             state->CursorClamp();
//         }
//         else
//         {
//             state->ID = id;
//             state->ScrollX = 0.0f;
//             stb_textedit_initialize_state(&state->Stb, !is_multiline);
//             if (!is_multiline && focus_requested_by_code)
//                 select_all = true;
//         }
//         if (flags & ImGuiInputTextFlags_AlwaysOverwrite)
//             state->Stb.insert_mode = 1; // stb field name is indeed incorrect (see #2863)
//         if (!is_multiline && (focus_requested_by_tabbing || (user_clicked && io.KeyCtrl)))
//             select_all = true;
//     }

//     if (g.ActiveId != id && init_make_active)
//     {
//         IM_ASSERT(state && state->ID == id);
//         SetActiveID(id, window);
//         SetFocusID(id, window);
//         FocusWindow(window);

//         // Declare our inputs
//         IM_ASSERT(ImGuiNavInput_COUNT < 32);
//         g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
//         if (is_multiline || (flags & ImGuiInputTextFlags_CallbackHistory))
//             g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
//         g.ActiveIdUsingNavInputMask |= (1 << ImGuiNavInput_Cancel);
//         g.ActiveIdUsingKeyInputMask |= ((ImU64)1 << ImGuiKey_Home) | ((ImU64)1 << ImGuiKey_End);
//         if (is_multiline)
//             g.ActiveIdUsingKeyInputMask |= ((ImU64)1 << ImGuiKey_PageUp) | ((ImU64)1 << ImGuiKey_PageDown);
//         if (flags & (ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_AllowTabInput))  // Disable keyboard tabbing out as we will use the \t character.
//             g.ActiveIdUsingKeyInputMask |= ((ImU64)1 << ImGuiKey_Tab);
//     }

//     // We have an edge case if ActiveId was set through another widget (e.g. widget being swapped), clear id immediately (don't wait until the end of the function)
//     if (g.ActiveId == id && state == NULL)
//         ClearActiveID();

//     // Release focus when we click outside
//     if (g.ActiveId == id && io.MouseClicked[0] && !init_state && !init_make_active) //-V560
//         clear_active_id = true;

//     // Lock the decision of whether we are going to take the path displaying the cursor or selection
//     const bool render_cursor = (g.ActiveId == id) || (state && user_scroll_active);
//     bool render_selection = state && state->HasSelection() && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
//     bool value_changed = false;
//     bool enter_pressed = false;

//     // When read-only we always use the live data passed to the function
//     // FIXME-OPT: Because our selection/cursor code currently needs the wide text we need to convert it when active, which is not ideal :(
//     if (is_readonly && state != NULL && (render_cursor || render_selection))
//     {
//         const char* buf_end = NULL;
//         state->TextW.resize(buf_size + 1);
//         state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, state->TextW.Size, buf, NULL, &buf_end);
//         state->CurLenA = (int)(buf_end - buf);
//         state->CursorClamp();
//         render_selection &= state->HasSelection();
//     }

//     // Select the buffer to render.
//     const bool buf_display_from_state = (render_cursor || render_selection || g.ActiveId == id) && !is_readonly && state && state->TextAIsValid;
//     const bool is_displaying_hint = (hint != NULL && (buf_display_from_state ? state->TextA.Data : buf)[0] == 0);

//     // Password pushes a temporary font with only a fallback glyph
//     if (is_password && !is_displaying_hint)
//     {
//         const ImFontGlyph* glyph = g.Font->FindGlyph('*');
//         ImFont* password_font = &g.InputTextPasswordFont;
//         password_font->FontSize = g.Font->FontSize;
//         password_font->Scale = g.Font->Scale;
//         password_font->Ascent = g.Font->Ascent;
//         password_font->Descent = g.Font->Descent;
//         password_font->ContainerAtlas = g.Font->ContainerAtlas;
//         password_font->FallbackGlyph = glyph;
//         password_font->FallbackAdvanceX = glyph->AdvanceX;
//         IM_ASSERT(password_font->Glyphs.empty() && password_font->IndexAdvanceX.empty() && password_font->IndexLookup.empty());
//         PushFont(password_font);
//     }

//     // Process mouse inputs and character inputs
//     int backup_current_text_length = 0;
//     if (g.ActiveId == id)
//     {
//         IM_ASSERT(state != NULL);
//         backup_current_text_length = state->CurLenA;
//         state->Edited = false;
//         state->BufCapacityA = buf_size;
//         state->Flags = flags;
//         state->UserCallback = callback;
//         state->UserCallbackData = callback_user_data;

//         // Although we are active we don't prevent mouse from hovering other elements unless we are interacting right now with the widget.
//         // Down the line we should have a cleaner library-wide concept of Selected vs Active.
//         g.ActiveIdAllowOverlap = !io.MouseDown[0];
//         g.WantTextInputNextFrame = 1;

//         // Edit in progress
//         const float mouse_x = (io.MousePos.x - frame_bb.Min.x - style.FramePadding.x) + state->ScrollX;
//         const float mouse_y = (is_multiline ? (io.MousePos.y - draw_window->DC.CursorPos.y) : (g.FontSize * 0.5f));

//         const bool is_osx = io.ConfigMacOSXBehaviors;
//         if (select_all || (hovered && !is_osx && io.MouseDoubleClicked[0]))
//         {
//             state->SelectAll();
//             state->SelectedAllMouseLock = true;
//         }
//         else if (hovered && is_osx && io.MouseDoubleClicked[0])
//         {
//             // Double-click select a word only, OS X style (by simulating keystrokes)
//             state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT);
//             state->OnKeyPressed(STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
//         }
//         else if (io.MouseClicked[0] && !state->SelectedAllMouseLock)
//         {
//             if (hovered)
//             {
//                 stb_textedit_click(state, &state->Stb, mouse_x, mouse_y);
//                 state->CursorAnimReset();
//             }
//         }
//         else if (io.MouseDown[0] && !state->SelectedAllMouseLock && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f))
//         {
//             stb_textedit_drag(state, &state->Stb, mouse_x, mouse_y);
//             state->CursorAnimReset();
//             state->CursorFollow = true;
//         }
//         if (state->SelectedAllMouseLock && !io.MouseDown[0])
//             state->SelectedAllMouseLock = false;

//         // It is ill-defined whether the backend needs to send a \t character when pressing the TAB keys.
//         // Win32 and GLFW naturally do it but not SDL.
//         const bool ignore_char_inputs = (io.KeyCtrl && !io.KeyAlt) || (is_osx && io.KeySuper);
//         if ((flags & ImGuiInputTextFlags_AllowTabInput) && IsKeyPressedMap(ImGuiKey_Tab) && !ignore_char_inputs && !io.KeyShift && !is_readonly)
//             if (!io.InputQueueCharacters.contains('\t'))
//             {
//                 unsigned int c = '\t'; // Insert TAB
//                 if (InputTextFilterCharacter(&c, flags, callback, callback_user_data, ImGuiInputSource_Keyboard))
//                     state->OnKeyPressed((int)c);
//             }

//         // Process regular text input (before we check for Return because using some IME will effectively send a Return?)
//         // We ignore CTRL inputs, but need to allow ALT+CTRL as some keyboards (e.g. German) use AltGR (which _is_ Alt+Ctrl) to input certain characters.
//         if (io.InputQueueCharacters.Size > 0)
//         {
//             if (!ignore_char_inputs && !is_readonly && !user_nav_input_start)
//                 for (int n = 0; n < io.InputQueueCharacters.Size; n++)
//                 {
//                     // Insert character if they pass filtering
//                     unsigned int c = (unsigned int)io.InputQueueCharacters[n];
//                     if (c == '\t' && io.KeyShift)
//                         continue;
//                     if (InputTextFilterCharacter(&c, flags, callback, callback_user_data, ImGuiInputSource_Keyboard))
//                         state->OnKeyPressed((int)c);
//                 }

//             // Consume characters
//             io.InputQueueCharacters.resize(0);
//         }
//     }

//     // Process other shortcuts/key-presses
//     bool cancel_edit = false;
//     if (g.ActiveId == id && !g.ActiveIdIsJustActivated && !clear_active_id)
//     {
//         IM_ASSERT(state != NULL);
//         IM_ASSERT(io.KeyMods == GetMergedKeyModFlags() && "Mismatching io.KeyCtrl/io.KeyShift/io.KeyAlt/io.KeySuper vs io.KeyMods"); // We rarely do this check, but if anything let's do it here.

//         const int row_count_per_page = ImMax((int)((inner_size.y - style.FramePadding.y) / g.FontSize), 1);
//         state->Stb.row_count_per_page = row_count_per_page;

//         const int k_mask = (io.KeyShift ? STB_TEXTEDIT_K_SHIFT : 0);
//         const bool is_osx = io.ConfigMacOSXBehaviors;
//         const bool is_osx_shift_shortcut = is_osx && (io.KeyMods == (ImGuiKeyModFlags_Super | ImGuiKeyModFlags_Shift));
//         const bool is_wordmove_key_down = is_osx ? io.KeyAlt : io.KeyCtrl;                     // OS X style: Text editing cursor movement using Alt instead of Ctrl
//         const bool is_startend_key_down = is_osx && io.KeySuper && !io.KeyCtrl && !io.KeyAlt;  // OS X style: Line/Text Start and End using Cmd+Arrows instead of Home/End
//         const bool is_ctrl_key_only = (io.KeyMods == ImGuiKeyModFlags_Ctrl);
//         const bool is_shift_key_only = (io.KeyMods == ImGuiKeyModFlags_Shift);
//         const bool is_shortcut_key = g.IO.ConfigMacOSXBehaviors ? (io.KeyMods == ImGuiKeyModFlags_Super) : (io.KeyMods == ImGuiKeyModFlags_Ctrl);

//         const bool is_cut   = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_X)) || (is_shift_key_only && IsKeyPressedMap(ImGuiKey_Delete))) && !is_readonly && !is_password && (!is_multiline || state->HasSelection());
//         const bool is_copy  = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_C)) || (is_ctrl_key_only  && IsKeyPressedMap(ImGuiKey_Insert))) && !is_password && (!is_multiline || state->HasSelection());
//         const bool is_paste = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_V)) || (is_shift_key_only && IsKeyPressedMap(ImGuiKey_Insert))) && !is_readonly;
//         const bool is_undo  = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_Z)) && !is_readonly && is_undoable);
//         const bool is_redo  = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_Y)) || (is_osx_shift_shortcut && IsKeyPressedMap(ImGuiKey_Z))) && !is_readonly && is_undoable;

//         if (IsKeyPressedMap(ImGuiKey_LeftArrow))                        { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINESTART : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDLEFT : STB_TEXTEDIT_K_LEFT) | k_mask); }
//         else if (IsKeyPressedMap(ImGuiKey_RightArrow))                  { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINEEND : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDRIGHT : STB_TEXTEDIT_K_RIGHT) | k_mask); }
//         else if (IsKeyPressedMap(ImGuiKey_UpArrow) && is_multiline)     { if (io.KeyCtrl) SetScrollY(draw_window, ImMax(draw_window->Scroll.y - g.FontSize, 0.0f)); else state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTSTART : STB_TEXTEDIT_K_UP) | k_mask); }
//         else if (IsKeyPressedMap(ImGuiKey_DownArrow) && is_multiline)   { if (io.KeyCtrl) SetScrollY(draw_window, ImMin(draw_window->Scroll.y + g.FontSize, GetScrollMaxY())); else state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTEND : STB_TEXTEDIT_K_DOWN) | k_mask); }
//         else if (IsKeyPressedMap(ImGuiKey_PageUp) && is_multiline)      { state->OnKeyPressed(STB_TEXTEDIT_K_PGUP | k_mask); scroll_y -= row_count_per_page * g.FontSize; }
//         else if (IsKeyPressedMap(ImGuiKey_PageDown) && is_multiline)    { state->OnKeyPressed(STB_TEXTEDIT_K_PGDOWN | k_mask); scroll_y += row_count_per_page * g.FontSize; }
//         else if (IsKeyPressedMap(ImGuiKey_Home))                        { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTSTART | k_mask : STB_TEXTEDIT_K_LINESTART | k_mask); }
//         else if (IsKeyPressedMap(ImGuiKey_End))                         { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTEND | k_mask : STB_TEXTEDIT_K_LINEEND | k_mask); }
//         else if (IsKeyPressedMap(ImGuiKey_Delete) && !is_readonly)      { state->OnKeyPressed(STB_TEXTEDIT_K_DELETE | k_mask); }
//         else if (IsKeyPressedMap(ImGuiKey_Backspace) && !is_readonly)
//         {
//             if (!state->HasSelection())
//             {
//                 if (is_wordmove_key_down)
//                     state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT | STB_TEXTEDIT_K_SHIFT);
//                 else if (is_osx && io.KeySuper && !io.KeyAlt && !io.KeyCtrl)
//                     state->OnKeyPressed(STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_SHIFT);
//             }
//             state->OnKeyPressed(STB_TEXTEDIT_K_BACKSPACE | k_mask);
//         }
//         else if (IsKeyPressedMap(ImGuiKey_Enter) || IsKeyPressedMap(ImGuiKey_KeyPadEnter))
//         {
//             bool ctrl_enter_for_new_line = (flags & ImGuiInputTextFlags_CtrlEnterForNewLine) != 0;
//             if (!is_multiline || (ctrl_enter_for_new_line && !io.KeyCtrl) || (!ctrl_enter_for_new_line && io.KeyCtrl))
//             {
//                 enter_pressed = clear_active_id = true;
//             }
//             else if (!is_readonly)
//             {
//                 unsigned int c = '\n'; // Insert new line
//                 if (InputTextFilterCharacter(&c, flags, callback, callback_user_data, ImGuiInputSource_Keyboard))
//                     state->OnKeyPressed((int)c);
//             }
//         }
//         else if (IsKeyPressedMap(ImGuiKey_Escape))
//         {
//             clear_active_id = cancel_edit = true;
//         }
//         else if (is_undo || is_redo)
//         {
//             state->OnKeyPressed(is_undo ? STB_TEXTEDIT_K_UNDO : STB_TEXTEDIT_K_REDO);
//             state->ClearSelection();
//         }
//         else if (is_shortcut_key && IsKeyPressedMap(ImGuiKey_A))
//         {
//             state->SelectAll();
//             state->CursorFollow = true;
//         }
//         else if (is_cut || is_copy)
//         {
//             // Cut, Copy
//             if (io.SetClipboardTextFn)
//             {
//                 const int ib = state->HasSelection() ? ImMin(state->Stb.select_start, state->Stb.select_end) : 0;
//                 const int ie = state->HasSelection() ? ImMax(state->Stb.select_start, state->Stb.select_end) : state->CurLenW;
//                 const int clipboard_data_len = ImTextCountUtf8BytesFromStr(state->TextW.Data + ib, state->TextW.Data + ie) + 1;
//                 char* clipboard_data = (char*)IM_ALLOC(clipboard_data_len * sizeof(char));
//                 ImTextStrToUtf8(clipboard_data, clipboard_data_len, state->TextW.Data + ib, state->TextW.Data + ie);
//                 SetClipboardText(clipboard_data);
//                 MemFree(clipboard_data);
//             }
//             if (is_cut)
//             {
//                 if (!state->HasSelection())
//                     state->SelectAll();
//                 state->CursorFollow = true;
//                 stb_textedit_cut(state, &state->Stb);
//             }
//         }
//         else if (is_paste)
//         {
//             if (const char* clipboard = GetClipboardText())
//             {
//                 // Filter pasted buffer
//                 const int clipboard_len = (int)strlen(clipboard);
//                 ImWchar* clipboard_filtered = (ImWchar*)IM_ALLOC((clipboard_len + 1) * sizeof(ImWchar));
//                 int clipboard_filtered_len = 0;
//                 for (const char* s = clipboard; *s; )
//                 {
//                     unsigned int c;
//                     s += ImTextCharFromUtf8(&c, s, NULL);
//                     if (c == 0)
//                         break;
//                     if (!InputTextFilterCharacter(&c, flags, callback, callback_user_data, ImGuiInputSource_Clipboard))
//                         continue;
//                     clipboard_filtered[clipboard_filtered_len++] = (ImWchar)c;
//                 }
//                 clipboard_filtered[clipboard_filtered_len] = 0;
//                 if (clipboard_filtered_len > 0) // If everything was filtered, ignore the pasting operation
//                 {
//                     stb_textedit_paste(state, &state->Stb, clipboard_filtered, clipboard_filtered_len);
//                     state->CursorFollow = true;
//                 }
//                 MemFree(clipboard_filtered);
//             }
//         }

//         // Update render selection flag after events have been handled, so selection highlight can be displayed during the same frame.
//         render_selection |= state->HasSelection() && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
//     }

//     // Process callbacks and apply result back to user's buffer.
//     if (g.ActiveId == id)
//     {
//         IM_ASSERT(state != NULL);
//         const char* apply_new_text = NULL;
//         int apply_new_text_length = 0;
//         if (cancel_edit)
//         {
//             // Restore initial value. Only return true if restoring to the initial value changes the current buffer contents.
//             if (!is_readonly && strcmp(buf, state->InitialTextA.Data) != 0)
//             {
//                 // Push records into the undo stack so we can CTRL+Z the revert operation itself
//                 apply_new_text = state->InitialTextA.Data;
//                 apply_new_text_length = state->InitialTextA.Size - 1;
//                 ImVector<ImWchar> w_text;
//                 if (apply_new_text_length > 0)
//                 {
//                     w_text.resize(ImTextCountCharsFromUtf8(apply_new_text, apply_new_text + apply_new_text_length) + 1);
//                     ImTextStrFromUtf8(w_text.Data, w_text.Size, apply_new_text, apply_new_text + apply_new_text_length);
//                 }
//                 stb_textedit_replace(state, &state->Stb, w_text.Data, (apply_new_text_length > 0) ? (w_text.Size - 1) : 0);
//             }
//         }

//         // When using 'ImGuiInputTextFlags_EnterReturnsTrue' as a special case we reapply the live buffer back to the input buffer before clearing ActiveId, even though strictly speaking it wasn't modified on this frame.
//         // If we didn't do that, code like InputInt() with ImGuiInputTextFlags_EnterReturnsTrue would fail.
//         // This also allows the user to use InputText() with ImGuiInputTextFlags_EnterReturnsTrue without maintaining any user-side storage (please note that if you use this property along ImGuiInputTextFlags_CallbackResize you can end up with your temporary string object unnecessarily allocating once a frame, either store your string data, either if you don't then don't use ImGuiInputTextFlags_CallbackResize).
//         bool apply_edit_back_to_user_buffer = !cancel_edit || (enter_pressed && (flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0);
//         if (apply_edit_back_to_user_buffer)
//         {
//             // Apply new value immediately - copy modified buffer back
//             // Note that as soon as the input box is active, the in-widget value gets priority over any underlying modification of the input buffer
//             // FIXME: We actually always render 'buf' when calling DrawList->AddText, making the comment above incorrect.
//             // FIXME-OPT: CPU waste to do this every time the widget is active, should mark dirty state from the stb_textedit callbacks.
//             if (!is_readonly)
//             {
//                 state->TextAIsValid = true;
//                 state->TextA.resize(state->TextW.Size * 4 + 1);
//                 ImTextStrToUtf8(state->TextA.Data, state->TextA.Size, state->TextW.Data, NULL);
//             }

//             // User callback
//             if ((flags & (ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackAlways)) != 0)
//             {
//                 IM_ASSERT(callback != NULL);

//                 // The reason we specify the usage semantic (Completion/History) is that Completion needs to disable keyboard TABBING at the moment.
//                 ImGuiInputTextFlags event_flag = 0;
//                 ImGuiKey event_key = ImGuiKey_COUNT;
//                 if ((flags & ImGuiInputTextFlags_CallbackCompletion) != 0 && IsKeyPressedMap(ImGuiKey_Tab))
//                 {
//                     event_flag = ImGuiInputTextFlags_CallbackCompletion;
//                     event_key = ImGuiKey_Tab;
//                 }
//                 else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressedMap(ImGuiKey_UpArrow))
//                 {
//                     event_flag = ImGuiInputTextFlags_CallbackHistory;
//                     event_key = ImGuiKey_UpArrow;
//                 }
//                 else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressedMap(ImGuiKey_DownArrow))
//                 {
//                     event_flag = ImGuiInputTextFlags_CallbackHistory;
//                     event_key = ImGuiKey_DownArrow;
//                 }
//                 else if ((flags & ImGuiInputTextFlags_CallbackEdit) && state->Edited)
//                 {
//                     event_flag = ImGuiInputTextFlags_CallbackEdit;
//                 }
//                 else if (flags & ImGuiInputTextFlags_CallbackAlways)
//                 {
//                     event_flag = ImGuiInputTextFlags_CallbackAlways;
//                 }

//                 if (event_flag)
//                 {
//                     ImGuiInputTextCallbackData callback_data;
//                     memset(&callback_data, 0, sizeof(ImGuiInputTextCallbackData));
//                     callback_data.EventFlag = event_flag;
//                     callback_data.Flags = flags;
//                     callback_data.UserData = callback_user_data;

//                     callback_data.EventKey = event_key;
//                     callback_data.Buf = state->TextA.Data;
//                     callback_data.BufTextLen = state->CurLenA;
//                     callback_data.BufSize = state->BufCapacityA;
//                     callback_data.BufDirty = false;

//                     // We have to convert from wchar-positions to UTF-8-positions, which can be pretty slow (an incentive to ditch the ImWchar buffer, see https://github.com/nothings/stb/issues/188)
//                     ImWchar* text = state->TextW.Data;
//                     const int utf8_cursor_pos = callback_data.CursorPos = ImTextCountUtf8BytesFromStr(text, text + state->Stb.cursor);
//                     const int utf8_selection_start = callback_data.SelectionStart = ImTextCountUtf8BytesFromStr(text, text + state->Stb.select_start);
//                     const int utf8_selection_end = callback_data.SelectionEnd = ImTextCountUtf8BytesFromStr(text, text + state->Stb.select_end);

//                     // Call user code
//                     callback(&callback_data);

//                     // Read back what user may have modified
//                     IM_ASSERT(callback_data.Buf == state->TextA.Data);  // Invalid to modify those fields
//                     IM_ASSERT(callback_data.BufSize == state->BufCapacityA);
//                     IM_ASSERT(callback_data.Flags == flags);
//                     const bool buf_dirty = callback_data.BufDirty;
//                     if (callback_data.CursorPos != utf8_cursor_pos || buf_dirty)            { state->Stb.cursor = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.CursorPos); state->CursorFollow = true; }
//                     if (callback_data.SelectionStart != utf8_selection_start || buf_dirty)  { state->Stb.select_start = (callback_data.SelectionStart == callback_data.CursorPos) ? state->Stb.cursor : ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionStart); }
//                     if (callback_data.SelectionEnd != utf8_selection_end || buf_dirty)      { state->Stb.select_end = (callback_data.SelectionEnd == callback_data.SelectionStart) ? state->Stb.select_start : ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionEnd); }
//                     if (buf_dirty)
//                     {
//                         IM_ASSERT(callback_data.BufTextLen == (int)strlen(callback_data.Buf)); // You need to maintain BufTextLen if you change the text!
//                         if (callback_data.BufTextLen > backup_current_text_length && is_resizable)
//                             state->TextW.resize(state->TextW.Size + (callback_data.BufTextLen - backup_current_text_length));
//                         state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, state->TextW.Size, callback_data.Buf, NULL);
//                         state->CurLenA = callback_data.BufTextLen;  // Assume correct length and valid UTF-8 from user, saves us an extra strlen()
//                         state->CursorAnimReset();
//                     }
//                 }
//             }

//             // Will copy result string if modified
//             if (!is_readonly && strcmp(state->TextA.Data, buf) != 0)
//             {
//                 apply_new_text = state->TextA.Data;
//                 apply_new_text_length = state->CurLenA;
//             }
//         }

//         // Copy result to user buffer
//         if (apply_new_text)
//         {
//             // We cannot test for 'backup_current_text_length != apply_new_text_length' here because we have no guarantee that the size
//             // of our owned buffer matches the size of the string object held by the user, and by design we allow InputText() to be used
//             // without any storage on user's side.
//             IM_ASSERT(apply_new_text_length >= 0);
//             if (is_resizable)
//             {
//                 ImGuiInputTextCallbackData callback_data;
//                 callback_data.EventFlag = ImGuiInputTextFlags_CallbackResize;
//                 callback_data.Flags = flags;
//                 callback_data.Buf = buf;
//                 callback_data.BufTextLen = apply_new_text_length;
//                 callback_data.BufSize = ImMax(buf_size, apply_new_text_length + 1);
//                 callback_data.UserData = callback_user_data;
//                 callback(&callback_data);
//                 buf = callback_data.Buf;
//                 buf_size = callback_data.BufSize;
//                 apply_new_text_length = ImMin(callback_data.BufTextLen, buf_size - 1);
//                 IM_ASSERT(apply_new_text_length <= buf_size);
//             }
//             //IMGUI_DEBUG_LOG("InputText(\"%s\"): apply_new_text length %d\n", label, apply_new_text_length);

//             // If the underlying buffer resize was denied or not carried to the next frame, apply_new_text_length+1 may be >= buf_size.
//             ImStrncpy(buf, apply_new_text, ImMin(apply_new_text_length + 1, buf_size));
//             value_changed = true;
//         }

//         // Clear temporary user storage
//         state->Flags = ImGuiInputTextFlags_None;
//         state->UserCallback = NULL;
//         state->UserCallbackData = NULL;
//     }

//     // Release active ID at the end of the function (so e.g. pressing Return still does a final application of the value)
//     if (clear_active_id && g.ActiveId == id)
//         ClearActiveID();

//     // Render frame
//     if (!is_multiline)
//     {
//         RenderNavHighlight(frame_bb, id);
//         RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
//     }

//     const ImVec4 clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + inner_size.x, frame_bb.Min.y + inner_size.y); // Not using frame_bb.Max because we have adjusted size
//     ImVec2 draw_pos = is_multiline ? draw_window->DC.CursorPos : frame_bb.Min + style.FramePadding;
//     ImVec2 text_size(0.0f, 0.0f);

//     // Set upper limit of single-line InputTextEx() at 2 million characters strings. The current pathological worst case is a long line
//     // without any carriage return, which would makes ImFont::RenderText() reserve too many vertices and probably crash. Avoid it altogether.
//     // Note that we only use this limit on single-line InputText(), so a pathologically large line on a InputTextMultiline() would still crash.
//     const int buf_display_max_length = 2 * 1024 * 1024;
//     const char* buf_display = buf_display_from_state ? state->TextA.Data : buf; //-V595
//     const char* buf_display_end = NULL; // We have specialized paths below for setting the length
//     if (is_displaying_hint)
//     {
//         buf_display = hint;
//         buf_display_end = hint + strlen(hint);
//     }

//     // Render text. We currently only render selection when the widget is active or while scrolling.
//     // FIXME: We could remove the '&& render_cursor' to keep rendering selection when inactive.
//     if (render_cursor || render_selection)
//     {
//         IM_ASSERT(state != NULL);
//         if (!is_displaying_hint)
//             buf_display_end = buf_display + state->CurLenA;

//         // Render text (with cursor and selection)
//         // This is going to be messy. We need to:
//         // - Display the text (this alone can be more easily clipped)
//         // - Handle scrolling, highlight selection, display cursor (those all requires some form of 1d->2d cursor position calculation)
//         // - Measure text height (for scrollbar)
//         // We are attempting to do most of that in **one main pass** to minimize the computation cost (non-negligible for large amount of text) + 2nd pass for selection rendering (we could merge them by an extra refactoring effort)
//         // FIXME: This should occur on buf_display but we'd need to maintain cursor/select_start/select_end for UTF-8.
//         const ImWchar* text_begin = state->TextW.Data;
//         ImVec2 cursor_offset, select_start_offset;

//         {
//             // Find lines numbers straddling 'cursor' (slot 0) and 'select_start' (slot 1) positions.
//             const ImWchar* searches_input_ptr[2] = { NULL, NULL };
//             int searches_result_line_no[2] = { -1000, -1000 };
//             int searches_remaining = 0;
//             if (render_cursor)
//             {
//                 searches_input_ptr[0] = text_begin + state->Stb.cursor;
//                 searches_result_line_no[0] = -1;
//                 searches_remaining++;
//             }
//             if (render_selection)
//             {
//                 searches_input_ptr[1] = text_begin + ImMin(state->Stb.select_start, state->Stb.select_end);
//                 searches_result_line_no[1] = -1;
//                 searches_remaining++;
//             }

//             // Iterate all lines to find our line numbers
//             // In multi-line mode, we never exit the loop until all lines are counted, so add one extra to the searches_remaining counter.
//             searches_remaining += is_multiline ? 1 : 0;
//             int line_count = 0;
//             //for (const ImWchar* s = text_begin; (s = (const ImWchar*)wcschr((const wchar_t*)s, (wchar_t)'\n')) != NULL; s++)  // FIXME-OPT: Could use this when wchar_t are 16-bit
//             for (const ImWchar* s = text_begin; *s != 0; s++)
//                 if (*s == '\n')
//                 {
//                     line_count++;
//                     if (searches_result_line_no[0] == -1 && s >= searches_input_ptr[0]) { searches_result_line_no[0] = line_count; if (--searches_remaining <= 0) break; }
//                     if (searches_result_line_no[1] == -1 && s >= searches_input_ptr[1]) { searches_result_line_no[1] = line_count; if (--searches_remaining <= 0) break; }
//                 }
//             line_count++;
//             if (searches_result_line_no[0] == -1)
//                 searches_result_line_no[0] = line_count;
//             if (searches_result_line_no[1] == -1)
//                 searches_result_line_no[1] = line_count;

//             // Calculate 2d position by finding the beginning of the line and measuring distance
//             cursor_offset.x = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[0], text_begin), searches_input_ptr[0]).x;
//             cursor_offset.y = searches_result_line_no[0] * g.FontSize;
//             if (searches_result_line_no[1] >= 0)
//             {
//                 select_start_offset.x = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[1], text_begin), searches_input_ptr[1]).x;
//                 select_start_offset.y = searches_result_line_no[1] * g.FontSize;
//             }

//             // Store text height (note that we haven't calculated text width at all, see GitHub issues #383, #1224)
//             if (is_multiline)
//                 text_size = ImVec2(inner_size.x, line_count * g.FontSize);
//         }

//         // Scroll
//         if (render_cursor && state->CursorFollow)
//         {
//             // Horizontal scroll in chunks of quarter width
//             if (!(flags & ImGuiInputTextFlags_NoHorizontalScroll))
//             {
//                 const float scroll_increment_x = inner_size.x * 0.25f;
//                 const float visible_width = inner_size.x - style.FramePadding.x;
//                 if (cursor_offset.x < state->ScrollX)
//                     state->ScrollX = IM_FLOOR(ImMax(0.0f, cursor_offset.x - scroll_increment_x));
//                 else if (cursor_offset.x - visible_width >= state->ScrollX)
//                     state->ScrollX = IM_FLOOR(cursor_offset.x - visible_width + scroll_increment_x);
//             }
//             else
//             {
//                 state->ScrollX = 0.0f;
//             }

//             // Vertical scroll
//             if (is_multiline)
//             {
//                 // Test if cursor is vertically visible
//                 if (cursor_offset.y - g.FontSize < scroll_y)
//                     scroll_y = ImMax(0.0f, cursor_offset.y - g.FontSize);
//                 else if (cursor_offset.y - inner_size.y >= scroll_y)
//                     scroll_y = cursor_offset.y - inner_size.y + style.FramePadding.y * 2.0f;
//                 const float scroll_max_y = ImMax((text_size.y + style.FramePadding.y * 2.0f) - inner_size.y, 0.0f);
//                 scroll_y = ImClamp(scroll_y, 0.0f, scroll_max_y);
//                 draw_pos.y += (draw_window->Scroll.y - scroll_y);   // Manipulate cursor pos immediately avoid a frame of lag
//                 draw_window->Scroll.y = scroll_y;
//             }

//             state->CursorFollow = false;
//         }

//         // Draw selection
//         const ImVec2 draw_scroll = ImVec2(state->ScrollX, 0.0f);
//         if (render_selection)
//         {
//             const ImWchar* text_selected_begin = text_begin + ImMin(state->Stb.select_start, state->Stb.select_end);
//             const ImWchar* text_selected_end = text_begin + ImMax(state->Stb.select_start, state->Stb.select_end);

//             ImU32 bg_color = GetColorU32(ImGuiCol_TextSelectedBg, render_cursor ? 1.0f : 0.6f); // FIXME: current code flow mandate that render_cursor is always true here, we are leaving the transparent one for tests.
//             float bg_offy_up = is_multiline ? 0.0f : -1.0f;    // FIXME: those offsets should be part of the style? they don't play so well with multi-line selection.
//             float bg_offy_dn = is_multiline ? 0.0f : 2.0f;
//             ImVec2 rect_pos = draw_pos + select_start_offset - draw_scroll;
//             for (const ImWchar* p = text_selected_begin; p < text_selected_end; )
//             {
//                 if (rect_pos.y > clip_rect.w + g.FontSize)
//                     break;
//                 if (rect_pos.y < clip_rect.y)
//                 {
//                     //p = (const ImWchar*)wmemchr((const wchar_t*)p, '\n', text_selected_end - p);  // FIXME-OPT: Could use this when wchar_t are 16-bit
//                     //p = p ? p + 1 : text_selected_end;
//                     while (p < text_selected_end)
//                         if (*p++ == '\n')
//                             break;
//                 }
//                 else
//                 {
//                     ImVec2 rect_size = InputTextCalcTextSizeW(p, text_selected_end, &p, NULL, true);
//                     if (rect_size.x <= 0.0f) rect_size.x = IM_FLOOR(g.Font->GetCharAdvance((ImWchar)' ') * 0.50f); // So we can see selected empty lines
//                     ImRect rect(rect_pos + ImVec2(0.0f, bg_offy_up - g.FontSize), rect_pos + ImVec2(rect_size.x, bg_offy_dn));
//                     rect.ClipWith(clip_rect);
//                     if (rect.Overlaps(clip_rect))
//                         draw_window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_color);
//                 }
//                 rect_pos.x = draw_pos.x - draw_scroll.x;
//                 rect_pos.y += g.FontSize;
//             }
//         }

//         // We test for 'buf_display_max_length' as a way to avoid some pathological cases (e.g. single-line 1 MB string) which would make ImDrawList crash.
//         if (is_multiline || (buf_display_end - buf_display) < buf_display_max_length)
//         {
//             ImU32 col = GetColorU32(is_displaying_hint ? ImGuiCol_TextDisabled : ImGuiCol_Text);
//             draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos - draw_scroll, col, buf_display, buf_display_end, 0.0f, is_multiline ? NULL : &clip_rect);
//         }

//         // Draw blinking cursor
//         if (render_cursor)
//         {
//             state->CursorAnim += io.DeltaTime;
//             bool cursor_is_visible = (!g.IO.ConfigInputTextCursorBlink) || (state->CursorAnim <= 0.0f) || ImFmod(state->CursorAnim, 1.20f) <= 0.80f;
//             ImVec2 cursor_screen_pos = ImFloor(draw_pos + cursor_offset - draw_scroll);
//             ImRect cursor_screen_rect(cursor_screen_pos.x, cursor_screen_pos.y - g.FontSize + 0.5f, cursor_screen_pos.x + 1.0f, cursor_screen_pos.y - 1.5f);
//             if (cursor_is_visible && cursor_screen_rect.Overlaps(clip_rect))
//                 draw_window->DrawList->AddLine(cursor_screen_rect.Min, cursor_screen_rect.GetBL(), GetColorU32(ImGuiCol_Text));

//             // Notify OS of text input position for advanced IME (-1 x offset so that Windows IME can cover our cursor. Bit of an extra nicety.)
//             if (!is_readonly)
//                 g.PlatformImePos = ImVec2(cursor_screen_pos.x - 1.0f, cursor_screen_pos.y - g.FontSize);
//         }
//     }
//     else
//     {
//         // Render text only (no selection, no cursor)
//         if (is_multiline)
//             text_size = ImVec2(inner_size.x, InputTextCalcTextLenAndLineCount(buf_display, &buf_display_end) * g.FontSize); // We don't need width
//         else if (!is_displaying_hint && g.ActiveId == id)
//             buf_display_end = buf_display + state->CurLenA;
//         else if (!is_displaying_hint)
//             buf_display_end = buf_display + strlen(buf_display);

//         if (is_multiline || (buf_display_end - buf_display) < buf_display_max_length)
//         {
//             ImU32 col = GetColorU32(is_displaying_hint ? ImGuiCol_TextDisabled : ImGuiCol_Text);
//             draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos, col, buf_display, buf_display_end, 0.0f, is_multiline ? NULL : &clip_rect);
//         }
//     }

//     if (is_password && !is_displaying_hint)
//         PopFont();

//     if (is_multiline)
//     {
//         Dummy(ImVec2(text_size.x, text_size.y + style.FramePadding.y));
//         EndChild();
//         EndGroup();
//     }

//     // Log as text
//     if (g.LogEnabled && (!is_password || is_displaying_hint))
//     {
//         LogSetNextTextDecoration("{", "}");
//         LogRenderedText(&draw_pos, buf_display, buf_display_end);
//     }

//     if (label_size.x > 0)
//         RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

//     if (value_changed && !(flags & ImGuiInputTextFlags_NoMarkEdited))
//         MarkItemEdited(id);

//     IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
//     if ((flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0)
//         return enter_pressed;
//     else
//         return value_changed;
// }

// Initialize/override default color options
void ImGui::SetColorEditOptions(ImGuiColorEditFlags flags)
{
    ImGuiContext& g = *GImGui;
    if ((flags & ImGuiColorEditFlags__DisplayMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__DisplayMask;
    if ((flags & ImGuiColorEditFlags__DataTypeMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__DataTypeMask;
    if ((flags & ImGuiColorEditFlags__PickerMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__PickerMask;
    if ((flags & ImGuiColorEditFlags__InputMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__InputMask;
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__DisplayMask));    // Check only 1 option is selected
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__DataTypeMask));   // Check only 1 option is selected
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__PickerMask));     // Check only 1 option is selected
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__InputMask));      // Check only 1 option is selected
    g.ColorEditOptions = flags;
}

// Note: only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
void ImGui::ColorTooltip(const char* text, const float* col, ImGuiColorEditFlags flags)
{
    ImGuiContext& g = *GImGui;

    BeginTooltipEx(0, ImGuiTooltipFlags_OverridePreviousTooltip);
    const char* text_end = text ? FindRenderedTextEnd(text, NULL) : text;
    if (text_end > text)
    {
        TextEx(text, text_end);
        Separator();
    }

    ImVec2 sz(g.FontSize * 3 + g.Style.FramePadding.y * 2, g.FontSize * 3 + g.Style.FramePadding.y * 2);
    ImVec4 cf(col[0], col[1], col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
    int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]), ca = (flags & ImGuiColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
    ColorButton("##preview", cf, (flags & (ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf)) | ImGuiColorEditFlags_NoTooltip, sz);
    SameLine();
    if ((flags & ImGuiColorEditFlags_InputRGB) || !(flags & ImGuiColorEditFlags__InputMask))
    {
        if (flags & ImGuiColorEditFlags_NoAlpha)
            Text("#%02X%02X%02X\nR: %d, G: %d, B: %d\n(%.3f, %.3f, %.3f)", cr, cg, cb, cr, cg, cb, col[0], col[1], col[2]);
        else
            Text("#%02X%02X%02X%02X\nR:%d, G:%d, B:%d, A:%d\n(%.3f, %.3f, %.3f, %.3f)", cr, cg, cb, ca, cr, cg, cb, ca, col[0], col[1], col[2], col[3]);
    }
    else if (flags & ImGuiColorEditFlags_InputHSV)
    {
        if (flags & ImGuiColorEditFlags_NoAlpha)
            Text("H: %.3f, S: %.3f, V: %.3f", col[0], col[1], col[2]);
        else
            Text("H: %.3f, S: %.3f, V: %.3f, A: %.3f", col[0], col[1], col[2], col[3]);
    }
    EndTooltip();
}

void ImGui::ColorEditOptionsPopup(const float* col, ImGuiColorEditFlags flags)
{
    bool allow_opt_inputs = !(flags & ImGuiColorEditFlags__DisplayMask);
    bool allow_opt_datatype = !(flags & ImGuiColorEditFlags__DataTypeMask);
    if ((!allow_opt_inputs && !allow_opt_datatype) || !BeginPopup("context"))
        return;
    ImGuiContext& g = *GImGui;
    ImGuiColorEditFlags opts = g.ColorEditOptions;
    if (allow_opt_inputs)
    {
        if (RadioButton("RGB", (opts & ImGuiColorEditFlags_DisplayRGB) != 0)) opts = (opts & ~ImGuiColorEditFlags__DisplayMask) | ImGuiColorEditFlags_DisplayRGB;
        if (RadioButton("HSV", (opts & ImGuiColorEditFlags_DisplayHSV) != 0)) opts = (opts & ~ImGuiColorEditFlags__DisplayMask) | ImGuiColorEditFlags_DisplayHSV;
        if (RadioButton("Hex", (opts & ImGuiColorEditFlags_DisplayHex) != 0)) opts = (opts & ~ImGuiColorEditFlags__DisplayMask) | ImGuiColorEditFlags_DisplayHex;
    }
    if (allow_opt_datatype)
    {
        if (allow_opt_inputs) Separator();
        if (RadioButton("0..255",     (opts & ImGuiColorEditFlags_Uint8) != 0)) opts = (opts & ~ImGuiColorEditFlags__DataTypeMask) | ImGuiColorEditFlags_Uint8;
        if (RadioButton("0.00..1.00", (opts & ImGuiColorEditFlags_Float) != 0)) opts = (opts & ~ImGuiColorEditFlags__DataTypeMask) | ImGuiColorEditFlags_Float;
    }

    if (allow_opt_inputs || allow_opt_datatype)
        Separator();
    if (Button("Copy as..", ImVec2(-1, 0)))
        OpenPopup("Copy");
    if (BeginPopup("Copy"))
    {
        int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]), ca = (flags & ImGuiColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
        char buf[64];
        ImFormatString(buf, IM_ARRAYSIZE(buf), "(%.3ff, %.3ff, %.3ff, %.3ff)", col[0], col[1], col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
        if (Selectable(buf))
            SetClipboardText(buf);
        ImFormatString(buf, IM_ARRAYSIZE(buf), "(%d,%d,%d,%d)", cr, cg, cb, ca);
        if (Selectable(buf))
            SetClipboardText(buf);
        ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", cr, cg, cb);
        if (Selectable(buf))
            SetClipboardText(buf);
        if (!(flags & ImGuiColorEditFlags_NoAlpha))
        {
            ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", cr, cg, cb, ca);
            if (Selectable(buf))
                SetClipboardText(buf);
        }
        EndPopup();
    }

    g.ColorEditOptions = opts;
    EndPopup();
}

void ImGui::ColorPickerOptionsPopup(const float* ref_col, ImGuiColorEditFlags flags)
{
    bool allow_opt_picker = !(flags & ImGuiColorEditFlags__PickerMask);
    bool allow_opt_alpha_bar = !(flags & ImGuiColorEditFlags_NoAlpha) && !(flags & ImGuiColorEditFlags_AlphaBar);
    if ((!allow_opt_picker && !allow_opt_alpha_bar) || !BeginPopup("context"))
        return;
    ImGuiContext& g = *GImGui;
    if (allow_opt_picker)
    {
        ImVec2 picker_size(g.FontSize * 8, ImMax(g.FontSize * 8 - (GetFrameHeight() + g.Style.ItemInnerSpacing.x), 1.0f)); // FIXME: Picker size copied from main picker function
        PushItemWidth(picker_size.x);
        for (int picker_type = 0; picker_type < 2; picker_type++)
        {
            // Draw small/thumbnail version of each picker type (over an invisible button for selection)
            if (picker_type > 0) Separator();
            PushID(picker_type);
            ImGuiColorEditFlags picker_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | (flags & ImGuiColorEditFlags_NoAlpha);
            if (picker_type == 0) picker_flags |= ImGuiColorEditFlags_PickerHueBar;
            if (picker_type == 1) picker_flags |= ImGuiColorEditFlags_PickerHueWheel;
            ImVec2 backup_pos = GetCursorScreenPos();
            if (Selectable("##selectable", false, 0, picker_size)) // By default, Selectable() is closing popup
                g.ColorEditOptions = (g.ColorEditOptions & ~ImGuiColorEditFlags__PickerMask) | (picker_flags & ImGuiColorEditFlags__PickerMask);
            SetCursorScreenPos(backup_pos);
            ImVec4 previewing_ref_col;
            memcpy(&previewing_ref_col, ref_col, sizeof(float) * ((picker_flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4));
            ColorPicker4("##previewing_picker", &previewing_ref_col.x, picker_flags);
            PopID();
        }
        PopItemWidth();
    }
    if (allow_opt_alpha_bar)
    {
        if (allow_opt_picker) Separator();
        CheckboxFlags("Alpha Bar", &g.ColorEditOptions, ImGuiColorEditFlags_AlphaBar);
    }
    EndPopup();
}

// //-------------------------------------------------------------------------
// // [SECTION] Widgets: TreeNode, CollapsingHeader, etc.
// //-------------------------------------------------------------------------
// // - TreeNode()
// // - TreeNodeV()
// // - TreeNodeEx()
// // - TreeNodeExV()
// // - TreeNodeBehavior() [Internal]
// // - TreePush()
// // - TreePop()
// // - GetTreeNodeToLabelSpacing()
// // - SetNextItemOpen()
// // - CollapsingHeader()
// //-------------------------------------------------------------------------

// bool ImGui::TreeNode(const char* str_id, const char* fmt, ...)
// {
//     va_list args;
//     va_start(args, fmt);
//     bool is_open = TreeNodeExV(str_id, 0, fmt, args);
//     va_end(args);
//     return is_open;
// }

// bool ImGui::TreeNode(const void* ptr_id, const char* fmt, ...)
// {
//     va_list args;
//     va_start(args, fmt);
//     bool is_open = TreeNodeExV(ptr_id, 0, fmt, args);
//     va_end(args);
//     return is_open;
// }

// bool ImGui::TreeNode(const char* label)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;
//     return TreeNodeBehavior(window->GetID(label), 0, label, NULL);
// }

// bool ImGui::TreeNodeV(const char* str_id, const char* fmt, va_list args)
// {
//     return TreeNodeExV(str_id, 0, fmt, args);
// }

// bool ImGui::TreeNodeV(const void* ptr_id, const char* fmt, va_list args)
// {
//     return TreeNodeExV(ptr_id, 0, fmt, args);
// }

// bool ImGui::TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     return TreeNodeBehavior(window->GetID(label), flags, label, NULL);
// }

// bool ImGui::TreeNodeEx(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
// {
//     va_list args;
//     va_start(args, fmt);
//     bool is_open = TreeNodeExV(str_id, flags, fmt, args);
//     va_end(args);
//     return is_open;
// }

// bool ImGui::TreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
// {
//     va_list args;
//     va_start(args, fmt);
//     bool is_open = TreeNodeExV(ptr_id, flags, fmt, args);
//     va_end(args);
//     return is_open;
// }

// bool ImGui::TreeNodeExV(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     ImGuiContext& g = *GImGui;
//     const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
//     return TreeNodeBehavior(window->GetID(str_id), flags, g.TempBuffer, label_end);
// }

// bool ImGui::TreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     ImGuiContext& g = *GImGui;
//     const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
//     return TreeNodeBehavior(window->GetID(ptr_id), flags, g.TempBuffer, label_end);
// }

// bool ImGui::TreeNodeBehaviorIsOpen(ImGuiID id, ImGuiTreeNodeFlags flags)
// {
//     if (flags & ImGuiTreeNodeFlags_Leaf)
//         return true;

//     // We only write to the tree storage if the user clicks (or explicitly use the SetNextItemOpen function)
//     ImGuiContext& g = *GImGui;
//     ImGuiWindow* window = g.CurrentWindow;
//     ImGuiStorage* storage = window->DC.StateStorage;

//     bool is_open;
//     if (g.NextItemData.Flags & ImGuiNextItemDataFlags_HasOpen)
//     {
//         if (g.NextItemData.OpenCond & ImGuiCond_Always)
//         {
//             is_open = g.NextItemData.OpenVal;
//             storage->SetInt(id, is_open);
//         }
//         else
//         {
//             // We treat ImGuiCond_Once and ImGuiCond_FirstUseEver the same because tree node state are not saved persistently.
//             const int stored_value = storage->GetInt(id, -1);
//             if (stored_value == -1)
//             {
//                 is_open = g.NextItemData.OpenVal;
//                 storage->SetInt(id, is_open);
//             }
//             else
//             {
//                 is_open = stored_value != 0;
//             }
//         }
//     }
//     else
//     {
//         is_open = storage->GetInt(id, (flags & ImGuiTreeNodeFlags_DefaultOpen) ? 1 : 0) != 0;
//     }

//     // When logging is enabled, we automatically expand tree nodes (but *NOT* collapsing headers.. seems like sensible behavior).
//     // NB- If we are above max depth we still allow manually opened nodes to be logged.
//     if (g.LogEnabled && !(flags & ImGuiTreeNodeFlags_NoAutoOpenOnLog) && (window->DC.TreeDepth - g.LogDepthRef) < g.LogDepthToExpand)
//         is_open = true;

//     return is_open;
// }

// bool ImGui::TreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     ImGuiContext& g = *GImGui;
//     const ImGuiStyle& style = g.Style;
//     const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
//     const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

//     if (!label_end)
//         label_end = FindRenderedTextEnd(label);
//     const ImVec2 label_size = CalcTextSize(label, label_end, false);

//     // We vertically grow up to current line height up the typical widget height.
//     const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);
//     ImRect frame_bb;
//     frame_bb.Min.x = (flags & ImGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x : window->DC.CursorPos.x;
//     frame_bb.Min.y = window->DC.CursorPos.y;
//     frame_bb.Max.x = window->WorkRect.Max.x;
//     frame_bb.Max.y = window->DC.CursorPos.y + frame_height;
//     if (display_frame)
//     {
//         // Framed header expand a little outside the default padding, to the edge of InnerClipRect
//         // (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)
//         frame_bb.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
//         frame_bb.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);
//     }

//     const float text_offset_x = g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2);           // Collapser arrow width + Spacing
//     const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);                    // Latch before ItemSize changes it
//     const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);  // Include collapser
//     ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
//     ItemSize(ImVec2(text_width, frame_height), padding.y);

//     // For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
//     ImRect interact_bb = frame_bb;
//     if (!display_frame && (flags & (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth)) == 0)
//         interact_bb.Max.x = frame_bb.Min.x + text_width + style.ItemSpacing.x * 2.0f;

//     // Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
//     // For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
//     // This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
//     const bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;
//     bool is_open = TreeNodeBehaviorIsOpen(id, flags);
//     if (is_open && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
//         window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);

//     bool item_add = ItemAdd(interact_bb, id);
//     window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
//     window->DC.LastItemDisplayRect = frame_bb;

//     if (!item_add)
//     {
//         if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
//             TreePushOverrideID(id);
//         IMGUI_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId, label, window->DC.LastItemStatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
//         return is_open;
//     }

//     ImGuiButtonFlags button_flags = ImGuiTreeNodeFlags_None;
//     if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
//         button_flags |= ImGuiButtonFlags_AllowItemOverlap;
//     if (!is_leaf)
//         button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

//     // We allow clicking on the arrow section with keyboard modifiers held, in order to easily
//     // allow browsing a tree while preserving selection with code implementing multi-selection patterns.
//     // When clicking on the rest of the tree node we always disallow keyboard modifiers.
//     const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
//     const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
//     const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
//     if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
//         button_flags |= ImGuiButtonFlags_NoKeyModifiers;

//     // Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick flags.
//     // Some alteration have subtle effects (e.g. toggle on MouseUp vs MouseDown events) due to requirements for multi-selection and drag and drop support.
//     // - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
//     // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
//     // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
//     // - Double-click on label = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1)
//     // - Double-click on arrow = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1 and _OpenOnArrow=0)
//     // It is rather standard that arrow click react on Down rather than Up.
//     // We set ImGuiButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want the item to be active on the initial MouseDown in order for drag and drop to work.
//     if (is_mouse_x_over_arrow)
//         button_flags |= ImGuiButtonFlags_PressedOnClick;
//     else if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
//         button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
//     else
//         button_flags |= ImGuiButtonFlags_PressedOnClickRelease;

//     bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
//     const bool was_selected = selected;

//     bool hovered, held;
//     bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
//     bool toggled = false;
//     if (!is_leaf)
//     {
//         if (pressed && g.DragDropHoldJustPressedId != id)
//         {
//             if ((flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) == 0 || (g.NavActivateId == id))
//                 toggled = true;
//             if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
//                 toggled |= is_mouse_x_over_arrow && !g.NavDisableMouseHover; // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
//             if ((flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseDoubleClicked[0])
//                 toggled = true;
//         }
//         else if (pressed && g.DragDropHoldJustPressedId == id)
//         {
//             IM_ASSERT(button_flags & ImGuiButtonFlags_PressedOnDragDropHold);
//             if (!is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
//                 toggled = true;
//         }

//         if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Left && is_open)
//         {
//             toggled = true;
//             NavMoveRequestCancel();
//         }
//         if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
//         {
//             toggled = true;
//             NavMoveRequestCancel();
//         }

//         if (toggled)
//         {
//             is_open = !is_open;
//             window->DC.StateStorage->SetInt(id, is_open);
//             window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
//         }
//     }
//     if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
//         SetItemAllowOverlap();

//     // In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
//     if (selected != was_selected) //-V547
//         window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

//     // Render
//     const ImU32 text_col = GetColorU32(ImGuiCol_Text);
//     ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
//     if (display_frame)
//     {
//         // Framed type
//         const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
//         RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
//         RenderNavHighlight(frame_bb, id, nav_highlight_flags);
//         if (flags & ImGuiTreeNodeFlags_Bullet)
//             RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f), text_col);
//         else if (!is_leaf)
//             RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
//         else // Leaf without bullet, left-adjusted text
//             text_pos.x -= text_offset_x;
//         if (flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton)
//             frame_bb.Max.x -= g.FontSize + style.FramePadding.x;

//         if (g.LogEnabled)
//             LogSetNextTextDecoration("###", "###");
//         RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
//     }
//     else
//     {
//         // Unframed typed for tree nodes
//         if (hovered || selected)
//         {
//             const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
//             RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
//             RenderNavHighlight(frame_bb, id, nav_highlight_flags);
//         }
//         if (flags & ImGuiTreeNodeFlags_Bullet)
//             RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f), text_col);
//         else if (!is_leaf)
//             RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
//         if (g.LogEnabled)
//             LogSetNextTextDecoration(">", NULL);
//         RenderText(text_pos, label, label_end, false);
//     }

//     if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
//         TreePushOverrideID(id);
//     IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
//     return is_open;
// }

// void ImGui::TreePush(const char* str_id)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     Indent();
//     window->DC.TreeDepth++;
//     PushID(str_id ? str_id : "#TreePush");
// }

// void ImGui::TreePush(const void* ptr_id)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     Indent();
//     window->DC.TreeDepth++;
//     PushID(ptr_id ? ptr_id : (const void*)"#TreePush");
// }

// void ImGui::TreePushOverrideID(ImGuiID id)
// {
//     ImGuiContext& g = *GImGui;
//     ImGuiWindow* window = g.CurrentWindow;
//     Indent();
//     window->DC.TreeDepth++;
//     window->IDStack.push_back(id);
// }

// void ImGui::TreePop()
// {
//     ImGuiContext& g = *GImGui;
//     ImGuiWindow* window = g.CurrentWindow;
//     Unindent();

//     window->DC.TreeDepth--;
//     ImU32 tree_depth_mask = (1 << window->DC.TreeDepth);

//     // Handle Left arrow to move to parent tree node (when ImGuiTreeNodeFlags_NavLeftJumpsBackHere is enabled)
//     if (g.NavMoveDir == ImGuiDir_Left && g.NavWindow == window && NavMoveRequestButNoResultYet())
//         if (g.NavIdIsAlive && (window->DC.TreeJumpToParentOnPopMask & tree_depth_mask))
//         {
//             SetNavID(window->IDStack.back(), g.NavLayer, 0, ImRect());
//             NavMoveRequestCancel();
//         }
//     window->DC.TreeJumpToParentOnPopMask &= tree_depth_mask - 1;

//     IM_ASSERT(window->IDStack.Size > 1); // There should always be 1 element in the IDStack (pushed during window creation). If this triggers you called TreePop/PopID too much.
//     PopID();
// }

// // Horizontal distance preceding label when using TreeNode() or Bullet()
// float ImGui::GetTreeNodeToLabelSpacing()
// {
//     ImGuiContext& g = *GImGui;
//     return g.FontSize + (g.Style.FramePadding.x * 2.0f);
// }

// // Set next TreeNode/CollapsingHeader open state.
// void ImGui::SetNextItemOpen(bool is_open, ImGuiCond cond)
// {
//     ImGuiContext& g = *GImGui;
//     if (g.CurrentWindow->SkipItems)
//         return;
//     g.NextItemData.Flags |= ImGuiNextItemDataFlags_HasOpen;
//     g.NextItemData.OpenVal = is_open;
//     g.NextItemData.OpenCond = cond ? cond : ImGuiCond_Always;
// }

// // CollapsingHeader returns true when opened but do not indent nor push into the ID stack (because of the ImGuiTreeNodeFlags_NoTreePushOnOpen flag).
// // This is basically the same as calling TreeNodeEx(label, ImGuiTreeNodeFlags_CollapsingHeader). You can remove the _NoTreePushOnOpen flag if you want behavior closer to normal TreeNode().
// bool ImGui::CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     return TreeNodeBehavior(window->GetID(label), flags | ImGuiTreeNodeFlags_CollapsingHeader, label);
// }

// // p_visible == NULL                        : regular collapsing header
// // p_visible != NULL && *p_visible == true  : show a small close button on the corner of the header, clicking the button will set *p_visible = false
// // p_visible != NULL && *p_visible == false : do not show the header at all
// // Do not mistake this with the Open state of the header itself, which you can adjust with SetNextItemOpen() or ImGuiTreeNodeFlags_DefaultOpen.
// bool ImGui::CollapsingHeader(const char* label, bool* p_visible, ImGuiTreeNodeFlags flags)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     if (p_visible && !*p_visible)
//         return false;

//     ImGuiID id = window->GetID(label);
//     flags |= ImGuiTreeNodeFlags_CollapsingHeader;
//     if (p_visible)
//         flags |= ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_ClipLabelForTrailingButton;
//     bool is_open = TreeNodeBehavior(id, flags, label);
//     if (p_visible != NULL)
//     {
//         // Create a small overlapping close button
//         // FIXME: We can evolve this into user accessible helpers to add extra buttons on title bars, headers, etc.
//         // FIXME: CloseButton can overlap into text, need find a way to clip the text somehow.
//         ImGuiContext& g = *GImGui;
//         ImGuiLastItemDataBackup last_item_backup;
//         float button_size = g.FontSize;
//         float button_x = ImMax(window->DC.LastItemRect.Min.x, window->DC.LastItemRect.Max.x - g.Style.FramePadding.x * 2.0f - button_size);
//         float button_y = window->DC.LastItemRect.Min.y;
//         ImGuiID close_button_id = GetIDWithSeed("#CLOSE", NULL, id);
//         if (CloseButton(close_button_id, ImVec2(button_x, button_y)))
//             *p_visible = false;
//         last_item_backup.Restore();
//     }

//     return is_open;
// }

// //-------------------------------------------------------------------------
// // [SECTION] Widgets: Selectable
// //-------------------------------------------------------------------------
// // - Selectable()
// //-------------------------------------------------------------------------

// // Tip: pass a non-visible label (e.g. "##hello") then you can use the space to draw other text or image.
// // But you need to make sure the ID is unique, e.g. enclose calls in PushID/PopID or use ##unique_id.
// // With this scheme, ImGuiSelectableFlags_SpanAllColumns and ImGuiSelectableFlags_AllowItemOverlap are also frequently used flags.
// // FIXME: Selectable() with (size.x == 0.0f) and (SelectableTextAlign.x > 0.0f) followed by SameLine() is currently not supported.
// bool ImGui::Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
// {
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems)
//         return false;

//     ImGuiContext& g = *GImGui;
//     const ImGuiStyle& style = g.Style;

//     // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
//     ImGuiID id = window->GetID(label);
//     ImVec2 label_size = CalcTextSize(label, NULL, true);
//     ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
//     ImVec2 pos = window->DC.CursorPos;
//     pos.y += window->DC.CurrLineTextBaseOffset;
//     ItemSize(size, 0.0f);

//     // Fill horizontal space
//     // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
//     const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
//     const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
//     const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
//     if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
//         size.x = ImMax(label_size.x, max_x - min_x);

//     // Text stays at the submission position, but bounding box may be extended on both sides
//     const ImVec2 text_min = pos;
//     const ImVec2 text_max(min_x + size.x, pos.y + size.y);

//     // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
//     ImRect bb(min_x, pos.y, text_max.x, text_max.y);
//     if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
//     {
//         const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
//         const float spacing_y = style.ItemSpacing.y;
//         const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
//         const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
//         bb.Min.x -= spacing_L;
//         bb.Min.y -= spacing_U;
//         bb.Max.x += (spacing_x - spacing_L);
//         bb.Max.y += (spacing_y - spacing_U);
//     }
//     //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

//     // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
//     const float backup_clip_rect_min_x = window->ClipRect.Min.x;
//     const float backup_clip_rect_max_x = window->ClipRect.Max.x;
//     if (span_all_columns)
//     {
//         window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
//         window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
//     }

//     bool item_add;
//     if (flags & ImGuiSelectableFlags_Disabled)
//     {
//         ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
//         g.CurrentItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
//         item_add = ItemAdd(bb, id);
//         g.CurrentItemFlags = backup_item_flags;
//     }
//     else
//     {
//         item_add = ItemAdd(bb, id);
//     }

//     if (span_all_columns)
//     {
//         window->ClipRect.Min.x = backup_clip_rect_min_x;
//         window->ClipRect.Max.x = backup_clip_rect_max_x;
//     }

//     if (!item_add)
//         return false;

//     // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
//     // which would be advantageous since most selectable are not selected.
//     if (span_all_columns && window->DC.CurrentColumns)
//         PushColumnsBackground();
//     else if (span_all_columns && g.CurrentTable)
//         TablePushBackgroundChannel();

//     // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
//     ImGuiButtonFlags button_flags = 0;
//     if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
//     if (flags & ImGuiSelectableFlags_SelectOnClick)     { button_flags |= ImGuiButtonFlags_PressedOnClick; }
//     if (flags & ImGuiSelectableFlags_SelectOnRelease)   { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
//     if (flags & ImGuiSelectableFlags_Disabled)          { button_flags |= ImGuiButtonFlags_Disabled; }
//     if (flags & ImGuiSelectableFlags_AllowDoubleClick)  { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
//     if (flags & ImGuiSelectableFlags_AllowItemOverlap)  { button_flags |= ImGuiButtonFlags_AllowItemOverlap; }

//     if (flags & ImGuiSelectableFlags_Disabled)
//         selected = false;

//     const bool was_selected = selected;
//     bool hovered, held;
//     bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

//     // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
//     if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
//     {
//         if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
//         {
//             SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent, ImRect(bb.Min - window->Pos, bb.Max - window->Pos));
//             g.NavDisableHighlight = true;
//         }
//     }
//     if (pressed)
//         MarkItemEdited(id);

//     if (flags & ImGuiSelectableFlags_AllowItemOverlap)
//         SetItemAllowOverlap();

//     // In this branch, Selectable() cannot toggle the selection so this will never trigger.
//     if (selected != was_selected) //-V547
//         window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

//     // Render
//     if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
//         hovered = true;
//     if (hovered || selected)
//     {
//         const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
//         RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
//         RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
//     }

//     if (span_all_columns && window->DC.CurrentColumns)
//         PopColumnsBackground();
//     else if (span_all_columns && g.CurrentTable)
//         TablePopBackgroundChannel();

//     if (flags & ImGuiSelectableFlags_Disabled) PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
//     RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
//     if (flags & ImGuiSelectableFlags_Disabled) PopStyleColor();

//     // Automatically close popups
//     if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.CurrentItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
//         CloseCurrentPopup();

//     IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
//     return pressed;
// }

// bool ImGui::Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
// {
//     if (Selectable(label, *p_selected, flags, size_arg))
//     {
//         *p_selected = !*p_selected;
//         return true;
//     }
//     return false;
// }


//-------------------------------------------------------------------------
// [SECTION] Widgets: Value helpers
// Those is not very useful, legacy API.
//-------------------------------------------------------------------------
// - Value()
//-------------------------------------------------------------------------

void ImGui::Value(const char* prefix, bool b)
{
    Text("%s: %s", prefix, (b ? "true" : "false"));
}

void ImGui::Value(const char* prefix, int v)
{
    Text("%s: %d", prefix, v);
}

void ImGui::Value(const char* prefix, unsigned int v)
{
    Text("%s: %d", prefix, v);
}

void ImGui::Value(const char* prefix, float v, const char* float_format)
{
    if (float_format)
    {
        char fmt[64];
        ImFormatString(fmt, IM_ARRAYSIZE(fmt), "%%s: %s", float_format);
        Text(fmt, prefix, v);
    }
    else
    {
        Text("%s: %.3f", prefix, v);
    }
}

//-------------------------------------------------------------------------
// [SECTION] MenuItem, BeginMenu, EndMenu, etc.
//-------------------------------------------------------------------------
// - ImGuiMenuColumns [Internal]
// - BeginMenuBar()
// - EndMenuBar()
// - BeginMainMenuBar()
// - EndMainMenuBar()
// - BeginMenu()
// - EndMenu()
// - MenuItem()
//-------------------------------------------------------------------------

// Helpers for internal use
void ImGuiMenuColumns::Update(int count, float spacing, bool clear)
{
    IM_ASSERT(count == IM_ARRAYSIZE(Pos));
    IM_UNUSED(count);
    Width = NextWidth = 0.0f;
    Spacing = spacing;
    if (clear)
        memset(NextWidths, 0, sizeof(NextWidths));
    for (int i = 0; i < IM_ARRAYSIZE(Pos); i++)
    {
        if (i > 0 && NextWidths[i] > 0.0f)
            Width += Spacing;
        Pos[i] = IM_FLOOR(Width);
        Width += NextWidths[i];
        NextWidths[i] = 0.0f;
    }
}

float ImGuiMenuColumns::DeclColumns(float w0, float w1, float w2) // not using va_arg because they promote float to double
{
    NextWidth = 0.0f;
    NextWidths[0] = ImMax(NextWidths[0], w0);
    NextWidths[1] = ImMax(NextWidths[1], w1);
    NextWidths[2] = ImMax(NextWidths[2], w2);
    for (int i = 0; i < IM_ARRAYSIZE(Pos); i++)
        NextWidth += NextWidths[i] + ((i > 0 && NextWidths[i] > 0.0f) ? Spacing : 0.0f);
    return ImMax(Width, NextWidth);
}

float ImGuiMenuColumns::CalcExtraSpace(float avail_w) const
{
    return ImMax(0.0f, avail_w - Width);
}



// Important: calling order matters!
// FIXME: Somehow overlapping with docking tech.
// FIXME: The "rect-cut" aspect of this could be formalized into a lower-level helper (rect-cut: https://halt.software/dead-simple-layouts)
bool ImGui::BeginViewportSideBar(const char* name, ImGuiViewport* viewport_p, ImGuiDir dir, float axis_size, ImGuiWindowFlags window_flags)
{
    IM_ASSERT(dir != ImGuiDir_None);

    ImGuiWindow* bar_window = FindWindowByName(name);
    if (bar_window == NULL || bar_window->BeginCount == 0)
    {
        // Calculate and set window size/position
        ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)(viewport_p ? viewport_p : GetMainViewport());
        ImRect avail_rect = viewport->GetBuildWorkRect();
        ImGuiAxis axis = (dir == ImGuiDir_Up || dir == ImGuiDir_Down) ? ImGuiAxis_Y : ImGuiAxis_X;
        ImVec2 pos = avail_rect.Min;
        if (dir == ImGuiDir_Right || dir == ImGuiDir_Down)
            pos[axis] = avail_rect.Max[axis] - axis_size;
        ImVec2 size = avail_rect.GetSize();
        size[axis] = axis_size;
        SetNextWindowPos(pos);
        SetNextWindowSize(size);

        // Report our size into work area (for next frame) using actual window size
        if (dir == ImGuiDir_Up || dir == ImGuiDir_Left)
            viewport->BuildWorkOffsetMin[axis] += axis_size;
        else if (dir == ImGuiDir_Down || dir == ImGuiDir_Right)
            viewport->BuildWorkOffsetMax[axis] -= axis_size;
    }

    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0)); // Lift normal size constraint
    bool is_open = Begin(name, NULL, window_flags);
    PopStyleVar(2);

    return is_open;
}

