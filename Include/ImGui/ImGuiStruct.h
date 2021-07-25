#pragma once


#include <float.h>                  // FLT_MIN, FLT_MAX
#include <stdarg.h>                 // va_list, va_start, va_end
#include <stddef.h>                 // ptrdiff_t, NULL
#include <string.h>                 // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp


#include <ImGui/ImGuiType.h>
//-----------------------------------------------------------------------------
// [SECTION] Misc data structures
//-----------------------------------------------------------------------------

// // Shared state of InputText(), passed as an argument to your callback when a ImGuiInputTextFlags_Callback* flag is used.
// // The callback function should return 0 by default.
// // Callbacks (follow a flag name and see comments in ImGuiInputTextFlags_ declarations for more details)
// // - ImGuiInputTextFlags_CallbackEdit:        Callback on buffer edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active)
// // - ImGuiInputTextFlags_CallbackAlways:      Callback on each iteration
// // - ImGuiInputTextFlags_CallbackCompletion:  Callback on pressing TAB
// // - ImGuiInputTextFlags_CallbackHistory:     Callback on pressing Up/Down arrows
// // - ImGuiInputTextFlags_CallbackCharFilter:  Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
// // - ImGuiInputTextFlags_CallbackResize:      Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow.
// struct ImGuiInputTextCallbackData
// {
//     ImGuiInputTextFlags EventFlag;      // One ImGuiInputTextFlags_Callback*    // Read-only
//     ImGuiInputTextFlags Flags;          // What user passed to InputText()      // Read-only
//     void*               UserData;       // What user passed to InputText()      // Read-only

//     // Arguments for the different callback events
//     // - To modify the text buffer in a callback, prefer using the InsertChars() / DeleteChars() function. InsertChars() will take care of calling the resize callback if necessary.
//     // - If you know your edits are not going to resize the underlying buffer allocation, you may modify the contents of 'Buf[]' directly. You need to update 'BufTextLen' accordingly (0 <= BufTextLen < BufSize) and set 'BufDirty'' to true so InputText can update its internal state.
//     ImWchar             EventChar;      // Character input                      // Read-write   // [CharFilter] Replace character with another one, or set to zero to drop. return 1 is equivalent to setting EventChar=0;
//     ImGuiKey            EventKey;       // Key pressed (Up/Down/TAB)            // Read-only    // [Completion,History]
//     char*               Buf;            // Text buffer                          // Read-write   // [Resize] Can replace pointer / [Completion,History,Always] Only write to pointed data, don't replace the actual pointer!
//     int                 BufTextLen;     // Text length (in bytes)               // Read-write   // [Resize,Completion,History,Always] Exclude zero-terminator storage. In C land: == strlen(some_text), in C++ land: string.length()
//     int                 BufSize;        // Buffer size (in bytes) = capacity+1  // Read-only    // [Resize,Completion,History,Always] Include zero-terminator storage. In C land == ARRAYSIZE(my_char_array), in C++ land: string.capacity()+1
//     bool                BufDirty;       // Set if you modify Buf/BufTextLen!    // Write        // [Completion,History,Always]
//     int                 CursorPos;      //                                      // Read-write   // [Completion,History,Always]
//     int                 SelectionStart; //                                      // Read-write   // [Completion,History,Always] == to SelectionEnd when no selection)
//     int                 SelectionEnd;   //                                      // Read-write   // [Completion,History,Always]

//     // Helper functions for text manipulation.
//     // Use those function to benefit from the CallbackResize behaviors. Calling those function reset the selection.
//     IMGUI_API ImGuiInputTextCallbackData();
//     IMGUI_API void      DeleteChars(int pos, int bytes_count);
//     IMGUI_API void      InsertChars(int pos, const char* text, const char* text_end = NULL);
//     void                SelectAll()             { SelectionStart = 0; SelectionEnd = BufTextLen; }
//     void                ClearSelection()        { SelectionStart = SelectionEnd = BufTextLen; }
//     bool                HasSelection() const    { return SelectionStart != SelectionEnd; }
// };

// Resizing callback data to apply custom constraint. As enabled by SetNextWindowSizeConstraints(). Callback is called during the next Begin().
// NB: For basic min/max size constraint on each axis you don't need to use the callback! The SetNextWindowSizeConstraints() parameters are enough.
struct ImGuiSizeCallbackData
{
    void*   UserData;       // Read-only.   What user passed to SetNextWindowSizeConstraints()
    ImVec2  Pos;            // Read-only.   Window position, for reference.
    ImVec2  CurrentSize;    // Read-only.   Current window size.
    ImVec2  DesiredSize;    // Read-write.  Desired size, based on user's mouse position. Write to this field to restrain resizing.
};

// Data payload for Drag and Drop operations: AcceptDragDropPayload(), GetDragDropPayload()
struct ImGuiPayload
{
    // Members
    void*           Data;               // Data (copied and owned by dear imgui)
    int             DataSize;           // Data size

    // [Internal]
    ImGuiID         SourceId;           // Source item id
    ImGuiID         SourceParentId;     // Source parent id (if available)
    int             DataFrameCount;     // Data timestamp
    char            DataType[32 + 1];   // Data type tag (short user-supplied string, 32 characters max)
    bool            Preview;            // Set when AcceptDragDropPayload() was called and mouse has been hovering the target item (nb: handle overlapping drag targets)
    bool            Delivery;           // Set when AcceptDragDropPayload() was called and mouse button is released over the target item.

    ImGuiPayload()  { Clear(); }
    void Clear()    { SourceId = SourceParentId = 0; Data = NULL; DataSize = 0; memset(DataType, 0, sizeof(DataType)); DataFrameCount = -1; Preview = Delivery = false; }
    bool IsDataType(const char* type) const { return DataFrameCount != -1 && strcmp(type, DataType) == 0; }
    bool IsPreview() const                  { return Preview; }
    bool IsDelivery() const                 { return Delivery; }
};

// Sorting specification for one column of a table (sizeof == 12 bytes)
struct ImGuiTableColumnSortSpecs
{
    ImGuiID                     ColumnUserID;       // User id of the column (if specified by a TableSetupColumn() call)
    ImS16                       ColumnIndex;        // Index of the column
    ImS16                       SortOrder;          // Index within parent ImGuiTableSortSpecs (always stored in order starting from 0, tables sorted on a single criteria will always have a 0 here)
    ImGuiSortDirection          SortDirection : 8;  // ImGuiSortDirection_Ascending or ImGuiSortDirection_Descending (you can use this or SortSign, whichever is more convenient for your sort function)

    ImGuiTableColumnSortSpecs() { memset(this, 0, sizeof(*this)); }
};

// Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
// Obtained by calling TableGetSortSpecs().
// When 'SpecsDirty == true' you can sort your data. It will be true with sorting specs have changed since last call, or the first time.
// Make sure to set 'SpecsDirty = false' after sorting, else you may wastefully sort your data every frame!
struct ImGuiTableSortSpecs
{
    const ImGuiTableColumnSortSpecs* Specs;     // Pointer to sort spec array.
    int                         SpecsCount;     // Sort spec count. Most often 1. May be > 1 when ImGuiTableFlags_SortMulti is enabled. May be == 0 when ImGuiTableFlags_SortTristate is enabled.
    bool                        SpecsDirty;     // Set to true when specs have changed since last time! Use this to sort again, then clear the flag.

    ImGuiTableSortSpecs()       { memset(this, 0, sizeof(*this)); }
};


//-----------------------------------------------------------------------------
// [SECTION] Metrics, Debug
//-----------------------------------------------------------------------------

struct ImGuiMetricsConfig
{
    bool        ShowWindowsRects;
    bool        ShowWindowsBeginOrder;
    bool        ShowTablesRects;
    bool        ShowDrawCmdMesh;
    bool        ShowDrawCmdBoundingBoxes;
    int         ShowWindowsRectsType;
    int         ShowTablesRectsType;

    ImGuiMetricsConfig()
    {
        ShowWindowsRects = false;
        ShowWindowsBeginOrder = false;
        ShowTablesRects = false;
        ShowDrawCmdMesh = true;
        ShowDrawCmdBoundingBoxes = true;
        ShowWindowsRectsType = -1;
        ShowTablesRectsType = -1;
    }
};


//-----------------------------------------------------------------------------
// [SECTION] Settings support
//-----------------------------------------------------------------------------

// Windows data saved in imgui.ini file
// Because we never destroy or rename ImGuiWindowSettings, we can store the names in a separate buffer easily.
// (this is designed to be stored in a ImChunkStream buffer, with the variable-length Name following our structure)
struct ImGuiWindowSettings
{
    ImGuiID     ID;
    ImVec2ih    Pos;
    ImVec2ih    Size;
    bool        Collapsed;
    bool        WantApply;      // Set when loaded from .ini data (to enable merging/loading .ini data into an already running context)

    ImGuiWindowSettings()       { memset(this, 0, sizeof(*this)); }
    char* GetName()             { return (char*)(this + 1); }
};

struct ImGuiSettingsHandler
{
    const char* TypeName;       // Short description stored in .ini file. Disallowed characters: '[' ']'
    ImGuiID     TypeHash;       // == ImHashStr(TypeName)
    void        (*ClearAllFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler);                                // Clear all settings data
    void        (*ReadInitFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler);                                // Read: Called before reading (in registration order)
    void*       (*ReadOpenFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name);              // Read: Called when entering into a new ini entry e.g. "[Window][Name]"
    void        (*ReadLineFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line); // Read: Called for every line of text within an ini entry
    void        (*ApplyAllFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler);                                // Read: Called after reading (in registration order)
    void        (*WriteAllFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf);      // Write: Output every entries into 'out_buf'
    void*       UserData;

    ImGuiSettingsHandler() { memset(this, 0, sizeof(*this)); }
};


struct IMGUI_API ImGuiStackSizes
{
    short   SizeOfIDStack;
    short   SizeOfColorStack;
    short   SizeOfStyleVarStack;
    short   SizeOfFontStack;
    short   SizeOfFocusScopeStack;
    short   SizeOfGroupStack;
    short   SizeOfBeginPopupStack;

    ImGuiStackSizes() { memset(this, 0, sizeof(*this)); }
    void SetToCurrentState();
    void CompareWithCurrentState();
};

