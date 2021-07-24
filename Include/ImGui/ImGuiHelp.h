#pragma once

#include <ImGui/ImGuiVector.h>
#include <ImGui/ImGuiType.h>




//-----------------------------------------------------------------------------
// [SECTION] Helpers (ImGuiOnceUponAFrame, ImGuiTextFilter, ImGuiTextBuffer, ImGuiStorage, ImGuiListClipper, ImColor)
//-----------------------------------------------------------------------------

// Helper: Unicode defines
#define IM_UNICODE_CODEPOINT_INVALID 0xFFFD     // Invalid Unicode code point (standard value).
#ifdef IMGUI_USE_WCHAR32
#define IM_UNICODE_CODEPOINT_MAX     0x10FFFF   // Maximum Unicode code point supported by this build.
#else
#define IM_UNICODE_CODEPOINT_MAX     0xFFFF     // Maximum Unicode code point supported by this build.
#endif

// Helper: Execute a block of code at maximum once a frame. Convenient if you want to quickly create an UI within deep-nested code that runs multiple times every frame.
// Usage: static ImGuiOnceUponAFrame oaf; if (oaf) ImGui::Text("This will be called only once per frame");
struct ImGuiOnceUponAFrame
{
    ImGuiOnceUponAFrame() { RefFrame = -1; }
    mutable int RefFrame;
    operator bool() const { int current_frame = ImGui::GetFrameCount(); if (RefFrame == current_frame) return false; RefFrame = current_frame; return true; }
};

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
struct ImGuiTextFilter
{
    IMGUI_API           ImGuiTextFilter(const char* default_filter = "");
    IMGUI_API bool      Draw(const char* label = "Filter (inc,-exc)", float width = 0.0f);  // Helper calling InputText+Build
    IMGUI_API bool      PassFilter(const char* text, const char* text_end = NULL) const;
    IMGUI_API void      Build();
    void                Clear()          { InputBuf[0] = 0; Build(); }
    bool                IsActive() const { return !Filters.empty(); }

    // [Internal]
    struct ImGuiTextRange
    {
        const char*     b;
        const char*     e;

        ImGuiTextRange()                                { b = e = NULL; }
        ImGuiTextRange(const char* _b, const char* _e)  { b = _b; e = _e; }
        bool            empty() const                   { return b == e; }
        IMGUI_API void  split(char separator, ImVector<ImGuiTextRange>* out) const;
    };
    char                    InputBuf[256];
    ImVector<ImGuiTextRange>Filters;
    int                     CountGrep;
};

// Helper: Growable text buffer for logging/accumulating text
// (this could be called 'ImGuiTextBuilder' / 'ImGuiStringBuilder')
struct ImGuiTextBuffer
{
    ImVector<char>      Buf;
    IMGUI_API static char EmptyString[1];

    ImGuiTextBuffer()   { }
    inline char         operator[](int i) const { IM_ASSERT(Buf.Data != NULL); return Buf.Data[i]; }
    const char*         begin() const           { return Buf.Data ? &Buf.front() : EmptyString; }
    const char*         end() const             { return Buf.Data ? &Buf.back() : EmptyString; }   // Buf is zero-terminated, so end() will point on the zero-terminator
    int                 size() const            { return Buf.Size ? Buf.Size - 1 : 0; }
    bool                empty() const           { return Buf.Size <= 1; }
    void                clear()                 { Buf.clear(); }
    void                reserve(int capacity)   { Buf.reserve(capacity); }
    const char*         c_str() const           { return Buf.Data ? Buf.Data : EmptyString; }
    IMGUI_API void      append(const char* str, const char* str_end = NULL);
    IMGUI_API void      appendf(const char* fmt, ...) IM_FMTARGS(2);
    IMGUI_API void      appendfv(const char* fmt, va_list args) IM_FMTLIST(2);
};

// Helper: Key->Value storage
// Typically you don't have to worry about this since a storage is held within each Window.
// We use it to e.g. store collapse state for a tree (Int 0/1)
// This is optimized for efficient lookup (dichotomy into a contiguous buffer) and rare insertion (typically tied to user interactions aka max once a frame)
// You can use it as custom user storage for temporary values. Declare your own storage if, for example:
// - You want to manipulate the open/close state of a particular sub-tree in your interface (tree node uses Int 0/1 to store their state).
// - You want to store custom debug data easily without adding or editing structures in your code (probably not efficient, but convenient)
// Types are NOT stored, so it is up to you to make sure your Key don't collide with different types.
struct ImGuiStorage
{
    // [Internal]
    struct ImGuiStoragePair
    {
        ImGuiID key;
        union { int val_i; float val_f; void* val_p; };
        ImGuiStoragePair(ImGuiID _key, int _val_i)      { key = _key; val_i = _val_i; }
        ImGuiStoragePair(ImGuiID _key, float _val_f)    { key = _key; val_f = _val_f; }
        ImGuiStoragePair(ImGuiID _key, void* _val_p)    { key = _key; val_p = _val_p; }
    };

    ImVector<ImGuiStoragePair>      Data;

    // - Get***() functions find pair, never add/allocate. Pairs are sorted so a query is O(log N)
    // - Set***() functions find pair, insertion on demand if missing.
    // - Sorted insertion is costly, paid once. A typical frame shouldn't need to insert any new pair.
    void                Clear() { Data.clear(); }
    IMGUI_API int       GetInt(ImGuiID key, int default_val = 0) const;
    IMGUI_API void      SetInt(ImGuiID key, int val);
    IMGUI_API bool      GetBool(ImGuiID key, bool default_val = false) const;
    IMGUI_API void      SetBool(ImGuiID key, bool val);
    IMGUI_API float     GetFloat(ImGuiID key, float default_val = 0.0f) const;
    IMGUI_API void      SetFloat(ImGuiID key, float val);
    IMGUI_API void*     GetVoidPtr(ImGuiID key) const; // default_val is NULL
    IMGUI_API void      SetVoidPtr(ImGuiID key, void* val);

    // - Get***Ref() functions finds pair, insert on demand if missing, return pointer. Useful if you intend to do Get+Set.
    // - References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
    // - A typical use case where this is convenient for quick hacking (e.g. add storage during a live Edit&Continue session if you can't modify existing struct)
    //      float* pvar = ImGui::GetFloatRef(key); ImGui::SliderFloat("var", pvar, 0, 100.0f); some_var += *pvar;
    IMGUI_API int*      GetIntRef(ImGuiID key, int default_val = 0);
    IMGUI_API bool*     GetBoolRef(ImGuiID key, bool default_val = false);
    IMGUI_API float*    GetFloatRef(ImGuiID key, float default_val = 0.0f);
    IMGUI_API void**    GetVoidPtrRef(ImGuiID key, void* default_val = NULL);

    // Use on your own storage if you know only integer are being stored (open/close all tree nodes)
    IMGUI_API void      SetAllInt(int val);

    // For quicker full rebuild of a storage (instead of an incremental one), you may add all your contents and then sort once.
    IMGUI_API void      BuildSortByKey();
};

// Helper: Manually clip large list of items.
// If you are submitting lots of evenly spaced items and you have a random access to the list, you can perform coarse
// clipping based on visibility to save yourself from processing those items at all.
// The clipper calculates the range of visible items and advance the cursor to compensate for the non-visible items we have skipped.
// (Dear ImGui already clip items based on their bounds but it needs to measure text size to do so, whereas manual coarse clipping before submission makes this cost and your own data fetching/submission cost almost null)
// Usage:
//   ImGuiListClipper clipper;
//   clipper.Begin(1000);         // We have 1000 elements, evenly spaced.
//   while (clipper.Step())
//       for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
//           ImGui::Text("line number %d", i);
// Generally what happens is:
// - Clipper lets you process the first element (DisplayStart = 0, DisplayEnd = 1) regardless of it being visible or not.
// - User code submit one element.
// - Clipper can measure the height of the first element
// - Clipper calculate the actual range of elements to display based on the current clipping rectangle, position the cursor before the first visible element.
// - User code submit visible elements.
struct ImGuiListClipper
{
    int     DisplayStart;
    int     DisplayEnd;

    // [Internal]
    int     ItemsCount;
    int     StepNo;
    int     ItemsFrozen;
    float   ItemsHeight;
    float   StartPosY;

    IMGUI_API ImGuiListClipper();
    IMGUI_API ~ImGuiListClipper();

    // items_count: Use INT_MAX if you don't know how many items you have (in which case the cursor won't be advanced in the final step)
    // items_height: Use -1.0f to be calculated automatically on first step. Otherwise pass in the distance between your items, typically GetTextLineHeightWithSpacing() or GetFrameHeightWithSpacing().
    IMGUI_API void Begin(int items_count, float items_height = -1.0f);  // Automatically called by constructor if you passed 'items_count' or by Step() in Step 1.
    IMGUI_API void End();                                               // Automatically called on the last call of Step() that returns false.
    IMGUI_API bool Step();                                              // Call until it returns false. The DisplayStart/DisplayEnd fields will be set and you can process/draw those items.

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    inline ImGuiListClipper(int items_count, float items_height = -1.0f) { memset(this, 0, sizeof(*this)); ItemsCount = -1; Begin(items_count, items_height); } // [removed in 1.79]
#endif
};

// Helpers macros to generate 32-bit encoded colors
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IM_COL32_R_SHIFT    16
#define IM_COL32_G_SHIFT    8
#define IM_COL32_B_SHIFT    0
#define IM_COL32_A_SHIFT    24
#define IM_COL32_A_MASK     0xFF000000
#else
#define IM_COL32_R_SHIFT    0
#define IM_COL32_G_SHIFT    8
#define IM_COL32_B_SHIFT    16
#define IM_COL32_A_SHIFT    24
#define IM_COL32_A_MASK     0xFF000000
#endif
#define IM_COL32(R,G,B,A)    (((ImU32)(A)<<IM_COL32_A_SHIFT) | ((ImU32)(B)<<IM_COL32_B_SHIFT) | ((ImU32)(G)<<IM_COL32_G_SHIFT) | ((ImU32)(R)<<IM_COL32_R_SHIFT))
#define IM_COL32_WHITE       IM_COL32(255,255,255,255)  // Opaque white = 0xFFFFFFFF
#define IM_COL32_BLACK       IM_COL32(0,0,0,255)        // Opaque black
#define IM_COL32_BLACK_TRANS IM_COL32(0,0,0,0)          // Transparent black = 0x00000000

// Helper: ImColor() implicitly converts colors to either ImU32 (packed 4x1 byte) or ImVec4 (4x1 float)
// Prefer using IM_COL32() macros if you want a guaranteed compile-time ImU32 for usage with ImDrawList API.
// **Avoid storing ImColor! Store either u32 of ImVec4. This is not a full-featured color class. MAY OBSOLETE.
// **None of the ImGui API are using ImColor directly but you can use it as a convenience to pass colors in either ImU32 or ImVec4 formats. Explicitly cast to ImU32 or ImVec4 if needed.
struct ImColor
{
    ImVec4              Value;

    ImColor()                                                       { Value.x = Value.y = Value.z = Value.w = 0.0f; }
    ImColor(int r, int g, int b, int a = 255)                       { float sc = 1.0f / 255.0f; Value.x = (float)r * sc; Value.y = (float)g * sc; Value.z = (float)b * sc; Value.w = (float)a * sc; }
    ImColor(ImU32 rgba)                                             { float sc = 1.0f / 255.0f; Value.x = (float)((rgba >> IM_COL32_R_SHIFT) & 0xFF) * sc; Value.y = (float)((rgba >> IM_COL32_G_SHIFT) & 0xFF) * sc; Value.z = (float)((rgba >> IM_COL32_B_SHIFT) & 0xFF) * sc; Value.w = (float)((rgba >> IM_COL32_A_SHIFT) & 0xFF) * sc; }
    ImColor(float r, float g, float b, float a = 1.0f)              { Value.x = r; Value.y = g; Value.z = b; Value.w = a; }
    ImColor(const ImVec4& col)                                      { Value = col; }
    inline operator ImU32() const                                   { return ImGui::ColorConvertFloat4ToU32(Value); }
    inline operator ImVec4() const                                  { return Value; }

    // FIXME-OBSOLETE: May need to obsolete/cleanup those helpers.
    inline void    SetHSV(float h, float s, float v, float a = 1.0f){ ImGui::ColorConvertHSVtoRGB(h, s, v, Value.x, Value.y, Value.z); Value.w = a; }
    static ImColor HSV(float h, float s, float v, float a = 1.0f)   { float r, g, b; ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b); return ImColor(r, g, b, a); }
};


//-----------------------------------------------------------------------------
// [SECTION] Generic helpers
// Note that the ImXXX helpers functions are lower-level than ImGui functions.
// ImGui functions or the ImGui context are never called/used from other ImXXX functions.
//-----------------------------------------------------------------------------
// - Helpers: Hashing
// - Helpers: Sorting
// - Helpers: Bit manipulation
// - Helpers: String, Formatting
// - Helpers: UTF-8 <> wchar conversions
// - Helpers: ImVec2/ImVec4 operators
// - Helpers: Maths
// - Helpers: Geometry
// - Helper: ImVec1
// - Helper: ImVec2ih
// - Helper: ImRect
// - Helper: ImBitArray
// - Helper: ImBitVector
// - Helper: ImSpan<>, ImSpanAllocator<>
// - Helper: ImPool<>
// - Helper: ImChunkStream<>
//-----------------------------------------------------------------------------


// Helpers: Hashing
IMGUI_API ImGuiID       ImHashData(const void* data, size_t data_size, ImU32 seed = 0);
IMGUI_API ImGuiID       ImHashStr(const char* data, size_t data_size = 0, ImU32 seed = 0);
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
static inline ImGuiID   ImHash(const void* data, int size, ImU32 seed = 0) { return size ? ImHashData(data, (size_t)size, seed) : ImHashStr((const char*)data, 0, seed); } // [moved to ImHashStr/ImHashData in 1.68]
#endif

// Helpers: Sorting
#define ImQsort         qsort

// Helpers: Color Blending
IMGUI_API ImU32         ImAlphaBlendColors(ImU32 col_a, ImU32 col_b);

// Helpers: Bit manipulation
static inline bool      ImIsPowerOfTwo(int v)           { return v != 0 && (v & (v - 1)) == 0; }
static inline bool      ImIsPowerOfTwo(ImU64 v)         { return v != 0 && (v & (v - 1)) == 0; }
static inline int       ImUpperPowerOfTwo(int v)        { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }

// Helpers: String, Formatting
IMGUI_API int           ImStricmp(const char* str1, const char* str2);
IMGUI_API int           ImStrnicmp(const char* str1, const char* str2, size_t count);
IMGUI_API void          ImStrncpy(char* dst, const char* src, size_t count);
IMGUI_API char*         ImStrdup(const char* str);
IMGUI_API char*         ImStrdupcpy(char* dst, size_t* p_dst_size, const char* str);
IMGUI_API const char*   ImStrchrRange(const char* str_begin, const char* str_end, char c);
IMGUI_API int           ImStrlenW(const ImWchar* str);
IMGUI_API const char*   ImStreolRange(const char* str, const char* str_end);                // End end-of-line
IMGUI_API const ImWchar*ImStrbolW(const ImWchar* buf_mid_line, const ImWchar* buf_begin);   // Find beginning-of-line
IMGUI_API const char*   ImStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end);
IMGUI_API void          ImStrTrimBlanks(char* str);
IMGUI_API const char*   ImStrSkipBlank(const char* str);
IMGUI_API int           ImFormatString(char* buf, size_t buf_size, const char* fmt, ...) IM_FMTARGS(3);
IMGUI_API int           ImFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args) IM_FMTLIST(3);
IMGUI_API const char*   ImParseFormatFindStart(const char* format);
IMGUI_API const char*   ImParseFormatFindEnd(const char* format);
IMGUI_API const char*   ImParseFormatTrimDecorations(const char* format, char* buf, size_t buf_size);
IMGUI_API int           ImParseFormatPrecision(const char* format, int default_value);
static inline bool      ImCharIsBlankA(char c)          { return c == ' ' || c == '\t'; }
static inline bool      ImCharIsBlankW(unsigned int c)  { return c == ' ' || c == '\t' || c == 0x3000; }

// Helpers: UTF-8 <> wchar conversions
IMGUI_API int           ImTextStrToUtf8(char* buf, int buf_size, const ImWchar* in_text, const ImWchar* in_text_end);      // return output UTF-8 bytes count
IMGUI_API int           ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);          // read one character. return input UTF-8 bytes count
IMGUI_API int           ImTextStrFromUtf8(ImWchar* buf, int buf_size, const char* in_text, const char* in_text_end, const char** in_remaining = NULL);   // return input UTF-8 bytes count
IMGUI_API int           ImTextCountCharsFromUtf8(const char* in_text, const char* in_text_end);                            // return number of UTF-8 code-points (NOT bytes count)
IMGUI_API int           ImTextCountUtf8BytesFromChar(const char* in_text, const char* in_text_end);                        // return number of bytes to express one char in UTF-8
IMGUI_API int           ImTextCountUtf8BytesFromStr(const ImWchar* in_text, const ImWchar* in_text_end);                   // return number of bytes to express string in UTF-8

// Helpers: ImVec2/ImVec4 operators
// We are keeping those disabled by default so they don't leak in user space, to allow user enabling implicit cast operators between ImVec2 and their own types (using IM_VEC2_CLASS_EXTRA etc.)
// We unfortunately don't have a unary- operator for ImVec2 because this would needs to be defined inside the class itself.
#ifdef IMGUI_DEFINE_MATH_OPERATORS
IM_MSVC_RUNTIME_CHECKS_OFF
static inline ImVec2 operator*(const ImVec2& lhs, const float rhs)              { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
static inline ImVec2 operator/(const ImVec2& lhs, const float rhs)              { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
static inline ImVec2& operator*=(ImVec2& lhs, const float rhs)                  { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const float rhs)                  { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
static inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
static inline ImVec4 operator+(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
static inline ImVec4 operator-(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
static inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
IM_MSVC_RUNTIME_CHECKS_RESTORE
#endif

// Helpers: File System
#ifdef IMGUI_DISABLE_FILE_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef void* ImFileHandle;
static inline ImFileHandle  ImFileOpen(const char*, const char*)                    { return NULL; }
static inline bool          ImFileClose(ImFileHandle)                               { return false; }
static inline ImU64         ImFileGetSize(ImFileHandle)                             { return (ImU64)-1; }
static inline ImU64         ImFileRead(void*, ImU64, ImU64, ImFileHandle)           { return 0; }
static inline ImU64         ImFileWrite(const void*, ImU64, ImU64, ImFileHandle)    { return 0; }
#endif
#ifndef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef FILE* ImFileHandle;
IMGUI_API ImFileHandle      ImFileOpen(const char* filename, const char* mode);
IMGUI_API bool              ImFileClose(ImFileHandle file);
IMGUI_API ImU64             ImFileGetSize(ImFileHandle file);
IMGUI_API ImU64             ImFileRead(void* data, ImU64 size, ImU64 count, ImFileHandle file);
IMGUI_API ImU64             ImFileWrite(const void* data, ImU64 size, ImU64 count, ImFileHandle file);
#else
#define IMGUI_DISABLE_TTY_FUNCTIONS // Can't use stdout, fflush if we are not using default file functions
#endif
IMGUI_API void*             ImFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size = NULL, int padding_bytes = 0);


// Helpers: Maths
IM_MSVC_RUNTIME_CHECKS_OFF
// - Wrapper for standard libs functions. (Note that imgui_demo.cpp does _not_ use them to keep the code easy to copy)
#ifndef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define ImFabs(X)           fabsf(X)
#define ImSqrt(X)           sqrtf(X)
#define ImFmod(X, Y)        fmodf((X), (Y))
#define ImCos(X)            cosf(X)
#define ImSin(X)            sinf(X)
#define ImAcos(X)           acosf(X)
#define ImAtan2(Y, X)       atan2f((Y), (X))
#define ImAtof(STR)         atof(STR)
//#define ImFloorStd(X)     floorf(X)           // We use our own, see ImFloor() and ImFloorSigned()
#define ImCeil(X)           ceilf(X)
static inline float  ImPow(float x, float y)    { return powf(x, y); }          // DragBehaviorT/SliderBehaviorT uses ImPow with either float/double and need the precision
static inline double ImPow(double x, double y)  { return pow(x, y); }
static inline float  ImLog(float x)             { return logf(x); }             // DragBehaviorT/SliderBehaviorT uses ImLog with either float/double and need the precision
static inline double ImLog(double x)            { return log(x); }
static inline int    ImAbs(int x)               { return x < 0 ? -x : x; }
static inline float  ImAbs(float x)             { return fabsf(x); }
static inline double ImAbs(double x)            { return fabs(x); }
static inline float  ImSign(float x)            { return (x < 0.0f) ? -1.0f : ((x > 0.0f) ? 1.0f : 0.0f); } // Sign operator - returns -1, 0 or 1 based on sign of argument
static inline double ImSign(double x)           { return (x < 0.0) ? -1.0 : ((x > 0.0) ? 1.0 : 0.0); }
#ifdef IMGUI_ENABLE_SSE
static inline float  ImRsqrt(float x)           { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }
#else
static inline float  ImRsqrt(float x)           { return 1.0f / sqrtf(x); }
#endif
static inline double ImRsqrt(double x)          { return 1.0 / sqrt(x); }
#endif
// - ImMin/ImMax/ImClamp/ImLerp/ImSwap are used by widgets which support variety of types: signed/unsigned int/long long float/double
// (Exceptionally using templates here but we could also redefine them for those types)
template<typename T> static inline T ImMin(T lhs, T rhs)                        { return lhs < rhs ? lhs : rhs; }
template<typename T> static inline T ImMax(T lhs, T rhs)                        { return lhs >= rhs ? lhs : rhs; }
template<typename T> static inline T ImClamp(T v, T mn, T mx)                   { return (v < mn) ? mn : (v > mx) ? mx : v; }
template<typename T> static inline T ImLerp(T a, T b, float t)                  { return (T)(a + (b - a) * t); }
template<typename T> static inline void ImSwap(T& a, T& b)                      { T tmp = a; a = b; b = tmp; }
template<typename T> static inline T ImAddClampOverflow(T a, T b, T mn, T mx)   { if (b < 0 && (a < mn - b)) return mn; if (b > 0 && (a > mx - b)) return mx; return a + b; }
template<typename T> static inline T ImSubClampOverflow(T a, T b, T mn, T mx)   { if (b > 0 && (a < mn + b)) return mn; if (b < 0 && (a > mx + b)) return mx; return a - b; }
// - Misc maths helpers
static inline ImVec2 ImMin(const ImVec2& lhs, const ImVec2& rhs)                { return ImVec2(lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y); }
static inline ImVec2 ImMax(const ImVec2& lhs, const ImVec2& rhs)                { return ImVec2(lhs.x >= rhs.x ? lhs.x : rhs.x, lhs.y >= rhs.y ? lhs.y : rhs.y); }
static inline ImVec2 ImClamp(const ImVec2& v, const ImVec2& mn, ImVec2 mx)      { return ImVec2((v.x < mn.x) ? mn.x : (v.x > mx.x) ? mx.x : v.x, (v.y < mn.y) ? mn.y : (v.y > mx.y) ? mx.y : v.y); }
static inline ImVec2 ImLerp(const ImVec2& a, const ImVec2& b, float t)          { return ImVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
static inline ImVec2 ImLerp(const ImVec2& a, const ImVec2& b, const ImVec2& t)  { return ImVec2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
static inline ImVec4 ImLerp(const ImVec4& a, const ImVec4& b, float t)          { return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t); }
static inline float  ImSaturate(float f)                                        { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
static inline float  ImLengthSqr(const ImVec2& lhs)                             { return (lhs.x * lhs.x) + (lhs.y * lhs.y); }
static inline float  ImLengthSqr(const ImVec4& lhs)                             { return (lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z) + (lhs.w * lhs.w); }
static inline float  ImInvLength(const ImVec2& lhs, float fail_value)           { float d = (lhs.x * lhs.x) + (lhs.y * lhs.y); if (d > 0.0f) return ImRsqrt(d); return fail_value; }
static inline float  ImFloor(float f)                                           { return (float)(int)(f); }
static inline float  ImFloorSigned(float f)                                     { return (float)((f >= 0 || (int)f == f) ? (int)f : (int)f - 1); } // Decent replacement for floorf()
static inline ImVec2 ImFloor(const ImVec2& v)                                   { return ImVec2((float)(int)(v.x), (float)(int)(v.y)); }
static inline int    ImModPositive(int a, int b)                                { return (a + b) % b; }
static inline float  ImDot(const ImVec2& a, const ImVec2& b)                    { return a.x * b.x + a.y * b.y; }
static inline ImVec2 ImRotate(const ImVec2& v, float cos_a, float sin_a)        { return ImVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a); }
static inline float  ImLinearSweep(float current, float target, float speed)    { if (current < target) return ImMin(current + speed, target); if (current > target) return ImMax(current - speed, target); return current; }
static inline ImVec2 ImMul(const ImVec2& lhs, const ImVec2& rhs)                { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
IM_MSVC_RUNTIME_CHECKS_RESTORE

// Helpers: Geometry
IMGUI_API ImVec2     ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t);
IMGUI_API ImVec2     ImBezierCubicClosestPoint(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& p, int num_segments);       // For curves with explicit number of segments
IMGUI_API ImVec2     ImBezierCubicClosestPointCasteljau(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& p, float tess_tol);// For auto-tessellated curves you can use tess_tol = style.CurveTessellationTol
IMGUI_API ImVec2     ImBezierQuadraticCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t);
IMGUI_API ImVec2     ImLineClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& p);
IMGUI_API bool       ImTriangleContainsPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p);
IMGUI_API ImVec2     ImTriangleClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p);
IMGUI_API void       ImTriangleBarycentricCoords(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p, float& out_u, float& out_v, float& out_w);
inline float         ImTriangleArea(const ImVec2& a, const ImVec2& b, const ImVec2& c) { return ImFabs((a.x * (b.y - c.y)) + (b.x * (c.y - a.y)) + (c.x * (a.y - b.y))) * 0.5f; }
IMGUI_API ImGuiDir   ImGetDirQuadrantFromDelta(float dx, float dy);

// Helper: ImVec1 (1D vector)
// (this odd construct is used to facilitate the transition between 1D and 2D, and the maintenance of some branches/patches)
IM_MSVC_RUNTIME_CHECKS_OFF
struct ImVec1
{
    float   x;
    ImVec1()         { x = 0.0f; }
    ImVec1(float _x) { x = _x; }
};

// Helper: ImVec2ih (2D vector, half-size integer, for long-term packed storage)
struct ImVec2ih
{
    short   x, y;
    ImVec2ih()                           { x = y = 0; }
    ImVec2ih(short _x, short _y)         { x = _x; y = _y; }
    explicit ImVec2ih(const ImVec2& rhs) { x = (short)rhs.x; y = (short)rhs.y; }
};

// Helper: ImRect (2D axis aligned bounding-box)
// NB: we can't rely on ImVec2 math operators being available here!
struct IMGUI_API ImRect
{
    ImVec2      Min;    // Upper-left
    ImVec2      Max;    // Lower-right

    ImRect()                                        : Min(0.0f, 0.0f), Max(0.0f, 0.0f)  {}
    ImRect(const ImVec2& min, const ImVec2& max)    : Min(min), Max(max)                {}
    ImRect(const ImVec4& v)                         : Min(v.x, v.y), Max(v.z, v.w)      {}
    ImRect(float x1, float y1, float x2, float y2)  : Min(x1, y1), Max(x2, y2)          {}

    ImVec2      GetCenter() const                   { return ImVec2((Min.x + Max.x) * 0.5f, (Min.y + Max.y) * 0.5f); }
    ImVec2      GetSize() const                     { return ImVec2(Max.x - Min.x, Max.y - Min.y); }
    float       GetWidth() const                    { return Max.x - Min.x; }
    float       GetHeight() const                   { return Max.y - Min.y; }
    float       GetArea() const                     { return (Max.x - Min.x) * (Max.y - Min.y); }
    ImVec2      GetTL() const                       { return Min; }                   // Top-left
    ImVec2      GetTR() const                       { return ImVec2(Max.x, Min.y); }  // Top-right
    ImVec2      GetBL() const                       { return ImVec2(Min.x, Max.y); }  // Bottom-left
    ImVec2      GetBR() const                       { return Max; }                   // Bottom-right
    bool        Contains(const ImVec2& p) const     { return p.x     >= Min.x && p.y     >= Min.y && p.x     <  Max.x && p.y     <  Max.y; }
    bool        Contains(const ImRect& r) const     { return r.Min.x >= Min.x && r.Min.y >= Min.y && r.Max.x <= Max.x && r.Max.y <= Max.y; }
    bool        Overlaps(const ImRect& r) const     { return r.Min.y <  Max.y && r.Max.y >  Min.y && r.Min.x <  Max.x && r.Max.x >  Min.x; }
    void        Add(const ImVec2& p)                { if (Min.x > p.x)     Min.x = p.x;     if (Min.y > p.y)     Min.y = p.y;     if (Max.x < p.x)     Max.x = p.x;     if (Max.y < p.y)     Max.y = p.y; }
    void        Add(const ImRect& r)                { if (Min.x > r.Min.x) Min.x = r.Min.x; if (Min.y > r.Min.y) Min.y = r.Min.y; if (Max.x < r.Max.x) Max.x = r.Max.x; if (Max.y < r.Max.y) Max.y = r.Max.y; }
    void        Expand(const float amount)          { Min.x -= amount;   Min.y -= amount;   Max.x += amount;   Max.y += amount; }
    void        Expand(const ImVec2& amount)        { Min.x -= amount.x; Min.y -= amount.y; Max.x += amount.x; Max.y += amount.y; }
    void        Translate(const ImVec2& d)          { Min.x += d.x; Min.y += d.y; Max.x += d.x; Max.y += d.y; }
    void        TranslateX(float dx)                { Min.x += dx; Max.x += dx; }
    void        TranslateY(float dy)                { Min.y += dy; Max.y += dy; }
    void        ClipWith(const ImRect& r)           { Min = ImMax(Min, r.Min); Max = ImMin(Max, r.Max); }                   // Simple version, may lead to an inverted rectangle, which is fine for Contains/Overlaps test but not for display.
    void        ClipWithFull(const ImRect& r)       { Min = ImClamp(Min, r.Min, r.Max); Max = ImClamp(Max, r.Min, r.Max); } // Full version, ensure both points are fully clipped.
    void        Floor()                             { Min.x = IM_FLOOR(Min.x); Min.y = IM_FLOOR(Min.y); Max.x = IM_FLOOR(Max.x); Max.y = IM_FLOOR(Max.y); }
    bool        IsInverted() const                  { return Min.x > Max.x || Min.y > Max.y; }
    ImVec4      ToVec4() const                      { return ImVec4(Min.x, Min.y, Max.x, Max.y); }
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

// Helper: ImBitArray
inline bool     ImBitArrayTestBit(const ImU32* arr, int n)      { ImU32 mask = (ImU32)1 << (n & 31); return (arr[n >> 5] & mask) != 0; }
inline void     ImBitArrayClearBit(ImU32* arr, int n)           { ImU32 mask = (ImU32)1 << (n & 31); arr[n >> 5] &= ~mask; }
inline void     ImBitArraySetBit(ImU32* arr, int n)             { ImU32 mask = (ImU32)1 << (n & 31); arr[n >> 5] |= mask; }
inline void     ImBitArraySetBitRange(ImU32* arr, int n, int n2) // Works on range [n..n2)
{
    n2--;
    while (n <= n2)
    {
        int a_mod = (n & 31);
        int b_mod = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
        ImU32 mask = (ImU32)(((ImU64)1 << b_mod) - 1) & ~(ImU32)(((ImU64)1 << a_mod) - 1);
        arr[n >> 5] |= mask;
        n = (n + 32) & ~31;
    }
}

// Helper: ImBitArray class (wrapper over ImBitArray functions)
// Store 1-bit per value.
template<int BITCOUNT>
struct IMGUI_API ImBitArray
{
    ImU32           Storage[(BITCOUNT + 31) >> 5];
    ImBitArray()                                { ClearAllBits(); }
    void            ClearAllBits()              { memset(Storage, 0, sizeof(Storage)); }
    void            SetAllBits()                { memset(Storage, 255, sizeof(Storage)); }
    bool            TestBit(int n) const        { IM_ASSERT(n < BITCOUNT); return ImBitArrayTestBit(Storage, n); }
    void            SetBit(int n)               { IM_ASSERT(n < BITCOUNT); ImBitArraySetBit(Storage, n); }
    void            ClearBit(int n)             { IM_ASSERT(n < BITCOUNT); ImBitArrayClearBit(Storage, n); }
    void            SetBitRange(int n, int n2)  { ImBitArraySetBitRange(Storage, n, n2); } // Works on range [n..n2)
};

// Helper: ImBitVector
// Store 1-bit per value.
struct IMGUI_API ImBitVector
{
    ImVector<ImU32> Storage;
    void            Create(int sz)              { Storage.resize((sz + 31) >> 5); memset(Storage.Data, 0, (size_t)Storage.Size * sizeof(Storage.Data[0])); }
    void            Clear()                     { Storage.clear(); }
    bool            TestBit(int n) const        { IM_ASSERT(n < (Storage.Size << 5)); return ImBitArrayTestBit(Storage.Data, n); }
    void            SetBit(int n)               { IM_ASSERT(n < (Storage.Size << 5)); ImBitArraySetBit(Storage.Data, n); }
    void            ClearBit(int n)             { IM_ASSERT(n < (Storage.Size << 5)); ImBitArrayClearBit(Storage.Data, n); }
};

// Helper: ImSpan<>
// Pointing to a span of data we don't own.
template<typename T>
struct ImSpan
{
    T*                  Data;
    T*                  DataEnd;

    // Constructors, destructor
    inline ImSpan()                                 { Data = DataEnd = NULL; }
    inline ImSpan(T* data, int size)                { Data = data; DataEnd = data + size; }
    inline ImSpan(T* data, T* data_end)             { Data = data; DataEnd = data_end; }

    inline void         set(T* data, int size)      { Data = data; DataEnd = data + size; }
    inline void         set(T* data, T* data_end)   { Data = data; DataEnd = data_end; }
    inline int          size() const                { return (int)(ptrdiff_t)(DataEnd - Data); }
    inline int          size_in_bytes() const       { return (int)(ptrdiff_t)(DataEnd - Data) * (int)sizeof(T); }
    inline T&           operator[](int i)           { T* p = Data + i; IM_ASSERT(p >= Data && p < DataEnd); return *p; }
    inline const T&     operator[](int i) const     { const T* p = Data + i; IM_ASSERT(p >= Data && p < DataEnd); return *p; }

    inline T*           begin()                     { return Data; }
    inline const T*     begin() const               { return Data; }
    inline T*           end()                       { return DataEnd; }
    inline const T*     end() const                 { return DataEnd; }

    // Utilities
    inline int  index_from_ptr(const T* it) const   { IM_ASSERT(it >= Data && it < DataEnd); const ptrdiff_t off = it - Data; return (int)off; }
};

// Helper: ImSpanAllocator<>
// Facilitate storing multiple chunks into a single large block (the "arena")
// - Usage: call Reserve() N times, allocate GetArenaSizeInBytes() worth, pass it to SetArenaBasePtr(), call GetSpan() N times to retrieve the aligned ranges.
template<int CHUNKS>
struct ImSpanAllocator
{
    char*   BasePtr;
    int     CurrOff;
    int     CurrIdx;
    int     Offsets[CHUNKS];
    int     Sizes[CHUNKS];

    ImSpanAllocator()                               { memset(this, 0, sizeof(*this)); }
    inline void  Reserve(int n, size_t sz, int a=4) { IM_ASSERT(n == CurrIdx && n < CHUNKS); CurrOff = IM_MEMALIGN(CurrOff, a); Offsets[n] = CurrOff; Sizes[n] = (int)sz; CurrIdx++; CurrOff += (int)sz; }
    inline int   GetArenaSizeInBytes()              { return CurrOff; }
    inline void  SetArenaBasePtr(void* base_ptr)    { BasePtr = (char*)base_ptr; }
    inline void* GetSpanPtrBegin(int n)             { IM_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS); return (void*)(BasePtr + Offsets[n]); }
    inline void* GetSpanPtrEnd(int n)               { IM_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS); return (void*)(BasePtr + Offsets[n] + Sizes[n]); }
    template<typename T>
    inline void  GetSpan(int n, ImSpan<T>* span)    { span->set((T*)GetSpanPtrBegin(n), (T*)GetSpanPtrEnd(n)); }
};

// Helper: ImPool<>
// Basic keyed storage for contiguous instances, slow/amortized insertion, O(1) indexable, O(Log N) queries by ID over a dense/hot buffer,
// Honor constructor/destructor. Add/remove invalidate all pointers. Indexes have the same lifetime as the associated object.
typedef int ImPoolIdx;
template<typename T>
struct IMGUI_API ImPool
{
    ImVector<T>     Buf;        // Contiguous data
    ImGuiStorage    Map;        // ID->Index
    ImPoolIdx       FreeIdx;    // Next free idx to use

    ImPool()    { FreeIdx = 0; }
    ~ImPool()   { Clear(); }
    T*          GetByKey(ImGuiID key)               { int idx = Map.GetInt(key, -1); return (idx != -1) ? &Buf[idx] : NULL; }
    T*          GetByIndex(ImPoolIdx n)             { return &Buf[n]; }
    ImPoolIdx   GetIndex(const T* p) const          { IM_ASSERT(p >= Buf.Data && p < Buf.Data + Buf.Size); return (ImPoolIdx)(p - Buf.Data); }
    T*          GetOrAddByKey(ImGuiID key)          { int* p_idx = Map.GetIntRef(key, -1); if (*p_idx != -1) return &Buf[*p_idx]; *p_idx = FreeIdx; return Add(); }
    bool        Contains(const T* p) const          { return (p >= Buf.Data && p < Buf.Data + Buf.Size); }
    void        Clear()                             { for (int n = 0; n < Map.Data.Size; n++) { int idx = Map.Data[n].val_i; if (idx != -1) Buf[idx].~T(); } Map.Clear(); Buf.clear(); FreeIdx = 0; }
    T*          Add()                               { int idx = FreeIdx; if (idx == Buf.Size) { Buf.resize(Buf.Size + 1); FreeIdx++; } else { FreeIdx = *(int*)&Buf[idx]; } IM_PLACEMENT_NEW(&Buf[idx]) T(); return &Buf[idx]; }
    void        Remove(ImGuiID key, const T* p)     { Remove(key, GetIndex(p)); }
    void        Remove(ImGuiID key, ImPoolIdx idx)  { Buf[idx].~T(); *(int*)&Buf[idx] = FreeIdx; FreeIdx = idx; Map.SetInt(key, -1); }
    void        Reserve(int capacity)               { Buf.reserve(capacity); Map.Data.reserve(capacity); }
    int         GetSize() const                     { return Buf.Size; }
};

// Helper: ImChunkStream<>
// Build and iterate a contiguous stream of variable-sized structures.
// This is used by Settings to store persistent data while reducing allocation count.
// We store the chunk size first, and align the final size on 4 bytes boundaries.
// The tedious/zealous amount of casting is to avoid -Wcast-align warnings.
template<typename T>
struct IMGUI_API ImChunkStream
{
    ImVector<char>  Buf;

    void    clear()                     { Buf.clear(); }
    bool    empty() const               { return Buf.Size == 0; }
    int     size() const                { return Buf.Size; }
    T*      alloc_chunk(size_t sz)      { size_t HDR_SZ = 4; sz = IM_MEMALIGN(HDR_SZ + sz, 4u); int off = Buf.Size; Buf.resize(off + (int)sz); ((int*)(void*)(Buf.Data + off))[0] = (int)sz; return (T*)(void*)(Buf.Data + off + (int)HDR_SZ); }
    T*      begin()                     { size_t HDR_SZ = 4; if (!Buf.Data) return NULL; return (T*)(void*)(Buf.Data + HDR_SZ); }
    T*      next_chunk(T* p)            { size_t HDR_SZ = 4; IM_ASSERT(p >= begin() && p < end()); p = (T*)(void*)((char*)(void*)p + chunk_size(p)); if (p == (T*)(void*)((char*)end() + HDR_SZ)) return (T*)0; IM_ASSERT(p < end()); return p; }
    int     chunk_size(const T* p)      { return ((const int*)p)[-1]; }
    T*      end()                       { return (T*)(void*)(Buf.Data + Buf.Size); }
    int     offset_from_ptr(const T* p) { IM_ASSERT(p >= begin() && p < end()); const ptrdiff_t off = (const char*)p - Buf.Data; return (int)off; }
    T*      ptr_from_offset(int off)    { IM_ASSERT(off >= 4 && off < Buf.Size); return (T*)(void*)(Buf.Data + off); }
    void    swap(ImChunkStream<T>& rhs) { rhs.Buf.swap(Buf); }

};


// Data for resizing from corner
struct ImGuiResizeGripDef
{
    ImVec2  CornerPosN;
    ImVec2  InnerDir;
    int     AngleMin12, AngleMax12;
};
// static const ImGuiResizeGripDef resize_grip_def[4] =
// {
//     { ImVec2(1, 1), ImVec2(-1, -1), 0, 3 },  // Lower-right
//     { ImVec2(0, 1), ImVec2(+1, -1), 3, 6 },  // Lower-left
//     { ImVec2(0, 0), ImVec2(+1, +1), 6, 9 },  // Upper-left (Unused)
//     { ImVec2(1, 0), ImVec2(-1, +1), 9, 12 }  // Upper-right (Unused)
// };

// Data for resizing from borders
struct ImGuiResizeBorderDef
{
    ImVec2 InnerDir;
    ImVec2 SegmentN1, SegmentN2;
    float  OuterAngle;
};

