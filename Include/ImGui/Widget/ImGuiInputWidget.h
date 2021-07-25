#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{


// Widgets: Input with Keyboard
// - If you want to use InputText() with std::string or any custom dynamic string type, see misc/cpp/imgui_stdlib.h and
// comments in imgui_demo.cpp.
// - Most of the ImGuiInputTextFlags flags are only useful for InputText() and not for InputFloatX, InputIntX,
// InputDouble etc.
IMGUI_API bool InputText(const char *label, char *buf, size_t buf_size, ImGuiInputTextFlags flags = 0,
                         ImGuiInputTextCallback callback = NULL, void *user_data = NULL);
IMGUI_API bool InputTextMultiline(const char *label, char *buf, size_t buf_size, const ImVec2 &size = ImVec2(0, 0),
                                  ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
                                  void *user_data = NULL);
IMGUI_API bool InputTextWithHint(const char *label, const char *hint, char *buf, size_t buf_size,
                                 ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
                                 void *user_data = NULL);
IMGUI_API bool InputFloat(const char *label, float *v, float step = 0.0f, float step_fast = 0.0f,
                          const char *format = "%.3f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputFloat2(const char *label, float v[2], const char *format = "%.3f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputFloat3(const char *label, float v[3], const char *format = "%.3f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputFloat4(const char *label, float v[4], const char *format = "%.3f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputInt(const char *label, int *v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputInt2(const char *label, int v[2], ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputInt3(const char *label, int v[3], ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputInt4(const char *label, int v[4], ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputDouble(const char *label, double *v, double step = 0.0, double step_fast = 0.0,
                           const char *format = "%.6f", ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputScalar(const char *label, ImGuiDataType data_type, void *p_data, const void *p_step = NULL,
                           const void *p_step_fast = NULL, const char *format = NULL, ImGuiInputTextFlags flags = 0);
IMGUI_API bool InputScalarN(const char *label, ImGuiDataType data_type, void *p_data, int components,
                            const void *p_step = NULL, const void *p_step_fast = NULL, const char *format = NULL,
                            ImGuiInputTextFlags flags = 0);


// InputText
IMGUI_API bool InputTextEx(const char *label, const char *hint, char *buf, int buf_size, const ImVec2 &size_arg,
                           ImGuiInputTextFlags flags, ImGuiInputTextCallback callback = NULL, void *user_data = NULL);
IMGUI_API bool TempInputText(const ImRect &bb, ImGuiID id, const char *label, char *buf, int buf_size,
                             ImGuiInputTextFlags flags);
IMGUI_API bool TempInputScalar(const ImRect &bb, ImGuiID id, const char *label, ImGuiDataType data_type, void *p_data,
                               const char *format, const void *p_clamp_min = NULL, const void *p_clamp_max = NULL);
inline bool TempInputIsActive(ImGuiID id)
{
    ImGuiContext &g = *GImGui;
    return (g.ActiveId == id && g.TempInputId == id);
}
inline ImGuiInputTextState *GetInputTextState(ImGuiID id)
{
    ImGuiContext &g = *GImGui;
    return (g.InputTextState.ID == id) ? &g.InputTextState : NULL;
} // Get input text state if active

} // namespace ImGui


// // Internal state of the currently focused/edited text input box
// // For a given item ID, access with ImGui::GetInputTextState()
// struct IMGUI_API ImGuiInputTextState
// {
//     ImGuiID                 ID;                     // widget id owning the text state
//     int                     CurLenW, CurLenA;       // we need to maintain our buffer length in both UTF-8 and wchar format. UTF-8 length is valid even if TextA is not.
//     ImVector<ImWchar>       TextW;                  // edit buffer, we need to persist but can't guarantee the persistence of the user-provided buffer. so we copy into own buffer.
//     ImVector<char>          TextA;                  // temporary UTF8 buffer for callbacks and other operations. this is not updated in every code-path! size=capacity.
//     ImVector<char>          InitialTextA;           // backup of end-user buffer at the time of focus (in UTF-8, unaltered)
//     bool                    TextAIsValid;           // temporary UTF8 buffer is not initially valid before we make the widget active (until then we pull the data from user argument)
//     int                     BufCapacityA;           // end-user buffer capacity
//     float                   ScrollX;                // horizontal scrolling/offset
//     ImStb::STB_TexteditState Stb;                   // state for stb_textedit.h
//     float                   CursorAnim;             // timer for cursor blink, reset on every user action so the cursor reappears immediately
//     bool                    CursorFollow;           // set when we want scrolling to follow the current cursor position (not always!)
//     bool                    SelectedAllMouseLock;   // after a double-click to select all, we ignore further mouse drags to update selection
//     bool                    Edited;                 // edited this frame
//     ImGuiInputTextFlags     Flags;                  // copy of InputText() flags
//     ImGuiInputTextCallback  UserCallback;           // "
//     void*                   UserCallbackData;       // "

//     ImGuiInputTextState()                   { memset(this, 0, sizeof(*this)); }
//     void        ClearText()                 { CurLenW = CurLenA = 0; TextW[0] = 0; TextA[0] = 0; CursorClamp(); }
//     void        ClearFreeMemory()           { TextW.clear(); TextA.clear(); InitialTextA.clear(); }
//     int         GetUndoAvailCount() const   { return Stb.undostate.undo_point; }
//     int         GetRedoAvailCount() const   { return STB_TEXTEDIT_UNDOSTATECOUNT - Stb.undostate.redo_point; }
//     void        OnKeyPressed(int key);      // Cannot be inline because we call in code in stb_textedit.h implementation

//     // Cursor & Selection
//     void        CursorAnimReset()           { CursorAnim = -0.30f; }                                   // After a user-input the cursor stays on for a while without blinking
//     void        CursorClamp()               { Stb.cursor = ImMin(Stb.cursor, CurLenW); Stb.select_start = ImMin(Stb.select_start, CurLenW); Stb.select_end = ImMin(Stb.select_end, CurLenW); }
//     bool        HasSelection() const        { return Stb.select_start != Stb.select_end; }
//     void        ClearSelection()            { Stb.select_start = Stb.select_end = Stb.cursor; }
//     void        SelectAll()                 { Stb.select_start = 0; Stb.cursor = Stb.select_end = CurLenW; Stb.has_preferred_x = 0; }
// };


// Shared state of InputText(), passed as an argument to your callback when a ImGuiInputTextFlags_Callback* flag is used.
// The callback function should return 0 by default.
// Callbacks (follow a flag name and see comments in ImGuiInputTextFlags_ declarations for more details)
// - ImGuiInputTextFlags_CallbackEdit:        Callback on buffer edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active)
// - ImGuiInputTextFlags_CallbackAlways:      Callback on each iteration
// - ImGuiInputTextFlags_CallbackCompletion:  Callback on pressing TAB
// - ImGuiInputTextFlags_CallbackHistory:     Callback on pressing Up/Down arrows
// - ImGuiInputTextFlags_CallbackCharFilter:  Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
// - ImGuiInputTextFlags_CallbackResize:      Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow.
struct ImGuiInputTextCallbackData
{
    ImGuiInputTextFlags EventFlag;      // One ImGuiInputTextFlags_Callback*    // Read-only
    ImGuiInputTextFlags Flags;          // What user passed to InputText()      // Read-only
    void*               UserData;       // What user passed to InputText()      // Read-only

    // Arguments for the different callback events
    // - To modify the text buffer in a callback, prefer using the InsertChars() / DeleteChars() function. InsertChars() will take care of calling the resize callback if necessary.
    // - If you know your edits are not going to resize the underlying buffer allocation, you may modify the contents of 'Buf[]' directly. You need to update 'BufTextLen' accordingly (0 <= BufTextLen < BufSize) and set 'BufDirty'' to true so InputText can update its internal state.
    ImWchar             EventChar;      // Character input                      // Read-write   // [CharFilter] Replace character with another one, or set to zero to drop. return 1 is equivalent to setting EventChar=0;
    ImGuiKey            EventKey;       // Key pressed (Up/Down/TAB)            // Read-only    // [Completion,History]
    char*               Buf;            // Text buffer                          // Read-write   // [Resize] Can replace pointer / [Completion,History,Always] Only write to pointed data, don't replace the actual pointer!
    int                 BufTextLen;     // Text length (in bytes)               // Read-write   // [Resize,Completion,History,Always] Exclude zero-terminator storage. In C land: == strlen(some_text), in C++ land: string.length()
    int                 BufSize;        // Buffer size (in bytes) = capacity+1  // Read-only    // [Resize,Completion,History,Always] Include zero-terminator storage. In C land == ARRAYSIZE(my_char_array), in C++ land: string.capacity()+1
    bool                BufDirty;       // Set if you modify Buf/BufTextLen!    // Write        // [Completion,History,Always]
    int                 CursorPos;      //                                      // Read-write   // [Completion,History,Always]
    int                 SelectionStart; //                                      // Read-write   // [Completion,History,Always] == to SelectionEnd when no selection)
    int                 SelectionEnd;   //                                      // Read-write   // [Completion,History,Always]

    // Helper functions for text manipulation.
    // Use those function to benefit from the CallbackResize behaviors. Calling those function reset the selection.
    IMGUI_API ImGuiInputTextCallbackData();
    IMGUI_API void      DeleteChars(int pos, int bytes_count);
    IMGUI_API void      InsertChars(int pos, const char* text, const char* text_end = NULL);
    void                SelectAll()             { SelectionStart = 0; SelectionEnd = BufTextLen; }
    void                ClearSelection()        { SelectionStart = SelectionEnd = BufTextLen; }
    bool                HasSelection() const    { return SelectionStart != SelectionEnd; }
};