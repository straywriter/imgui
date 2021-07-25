#pragma once


// Version
// (Integer encoded as XYYZZ for use in #if preprocessor conditionals. Work in progress versions typically starts at XYY99 then bounce up to XYY00, XYY01 etc. when release tagging happens)
#define IMGUI_VERSION               "1.83 WIP"
#define IMGUI_VERSION_NUM           18210
#define IMGUI_CHECKVERSION()        ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(ImDrawIdx))
#define IMGUI_HAS_TABLE

// Define attributes of all API symbols declarations (e.g. for DLL under Windows)
// IMGUI_API is used for core imgui functions, IMGUI_IMPL_API is used for the default backends files (imgui_impl_xxx.h)
// Using dear imgui via a shared library is not recommended, because we don't guarantee backward nor forward ABI compatibility (also function call overhead, as dear imgui is a call-heavy API)
#ifndef IMGUI_API
#define IMGUI_API
#endif
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API              IMGUI_API
#endif

// Helper Macros
#ifndef IM_ASSERT
#include <assert.h>
#define IM_ASSERT(_EXPR)            assert(_EXPR)                               // You can override the default assert handler by editing imconfig.h
#endif
#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*(_ARR))))     // Size of a static C-style array. Don't use on pointers!
#define IM_UNUSED(_VAR)             ((void)(_VAR))                              // Used to silence "unused variable warnings". Often useful as asserts may be stripped out from final builds.
#if (__cplusplus >= 201100) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201100)
#define IM_OFFSETOF(_TYPE,_MEMBER)  offsetof(_TYPE, _MEMBER)                    // Offset of _MEMBER within _TYPE. Standardized as offsetof() in C++11
#else
#define IM_OFFSETOF(_TYPE,_MEMBER)  ((size_t)&(((_TYPE*)0)->_MEMBER))           // Offset of _MEMBER within _TYPE. Old style macro.
#endif

// Helper Macros - IM_FMTARGS, IM_FMTLIST: Apply printf-style warnings to our formatting functions.
#if !defined(IMGUI_USE_STB_SPRINTF) && defined(__clang__)
#define IM_FMTARGS(FMT)             __attribute__((format(printf, FMT, FMT+1)))
#define IM_FMTLIST(FMT)             __attribute__((format(printf, FMT, 0)))
#elif !defined(IMGUI_USE_STB_SPRINTF) && defined(__GNUC__) && defined(__MINGW32__)
#define IM_FMTARGS(FMT)             __attribute__((format(gnu_printf, FMT, FMT+1)))
#define IM_FMTLIST(FMT)             __attribute__((format(gnu_printf, FMT, 0)))
#else
#define IM_FMTARGS(FMT)
#define IM_FMTLIST(FMT)
#endif

// Disable some of MSVC most aggressive Debug runtime checks in function header/footer (used in some simple/low-level functions)
#if defined(_MSC_VER) && !defined(__clang__) && !defined(IMGUI_DEBUG_PARANOID)
#define IM_MSVC_RUNTIME_CHECKS_OFF      __pragma(runtime_checks("",off))     __pragma(check_stack(off)) __pragma(strict_gs_check(push,off))
#define IM_MSVC_RUNTIME_CHECKS_RESTORE  __pragma(runtime_checks("",restore)) __pragma(check_stack())    __pragma(strict_gs_check(pop))
#else
#define IM_MSVC_RUNTIME_CHECKS_OFF
#define IM_MSVC_RUNTIME_CHECKS_RESTORE
#endif

// Warnings
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 26495)    // [Static Analyzer] Variable 'XXX' is uninitialized. Always initialize a member variable (type.6).
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wclass-memaccess"  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif


//-----------------------------------------------------------------------------
// [SECTION] Helpers: Memory allocations macros, ImVector<>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// IM_MALLOC(), IM_FREE(), IM_NEW(), IM_PLACEMENT_NEW(), IM_DELETE()
// We call C++ constructor on own allocated memory via the placement "new(ptr) Type()" syntax.
// Defining a custom placement new() with a custom parameter allows us to bypass including <new> which on some platforms complains when user has disabled exceptions.
//-----------------------------------------------------------------------------

struct ImNewWrapper {};
inline void* operator new(size_t, ImNewWrapper, void* ptr) { return ptr; }
inline void  operator delete(void*, ImNewWrapper, void*)   {} // This is only required so we can use the symmetrical new()
#define IM_ALLOC(_SIZE)                     ImGui::MemAlloc(_SIZE)
#define IM_FREE(_PTR)                       ImGui::MemFree(_PTR)
#define IM_PLACEMENT_NEW(_PTR)              new(ImNewWrapper(), _PTR)
#define IM_NEW(_TYPE)                       new(ImNewWrapper(), ImGui::MemAlloc(sizeof(_TYPE))) _TYPE
template<typename T> void IM_DELETE(T* p)   { if (p) { p->~T(); ImGui::MemFree(p); } }




// Enable SSE intrinsics if available
#if defined __SSE__ || defined __x86_64__ || defined _M_X64
#define IMGUI_ENABLE_SSE
#include <immintrin.h>
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4251)     // class 'xxx' needs to have dll-interface to be used by clients of struct 'xxx' // when IMGUI_API is set to__declspec(dllexport)
#pragma warning (disable: 26812)    // The enum type 'xxx' is unscoped. Prefer 'enum class' over 'enum' (Enum.3). [MSVC Static Analyzer)
#pragma warning (disable: 26495)    // [Static Analyzer] Variable 'XXX' is uninitialized. Always initialize a member variable (type.6).

#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants ok, for ImFloorSigned()
#pragma clang diagnostic ignored "-Wunused-function"                // for stb_textedit.h
#pragma clang diagnostic ignored "-Wmissing-prototypes"             // for stb_textedit.h
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"              // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wclass-memaccess"      // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

// Legacy defines
#ifdef IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS            // Renamed in 1.74
#error Use IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#endif
#ifdef IMGUI_DISABLE_MATH_FUNCTIONS                     // Renamed in 1.74
#error Use IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#endif

// Enable stb_truetype by default unless FreeType is enabled.
// You can compile with both by defining both IMGUI_ENABLE_FREETYPE and IMGUI_ENABLE_STB_TRUETYPE together.
#ifndef IMGUI_ENABLE_FREETYPE
#define IMGUI_ENABLE_STB_TRUETYPE
#endif



//-----------------------------------------------------------------------------
// [SECTION] Context pointer
// See implementation of this variable in imgui.cpp for comments and details.
//-----------------------------------------------------------------------------


// //-------------------------------------------------------------------------
// // [SECTION] STB libraries includes
// //-------------------------------------------------------------------------

// namespace ImStb
// {

// #undef STB_TEXTEDIT_STRING
// #undef STB_TEXTEDIT_CHARTYPE
// #define STB_TEXTEDIT_STRING             ImGuiInputTextState
// #define STB_TEXTEDIT_CHARTYPE           ImWchar
// #define STB_TEXTEDIT_GETWIDTH_NEWLINE   (-1.0f)
// #define STB_TEXTEDIT_UNDOSTATECOUNT     99
// #define STB_TEXTEDIT_UNDOCHARCOUNT      999
// #include "imstb_textedit.h"

// } // namespace ImStb

//-----------------------------------------------------------------------------
// [SECTION] Macros
//-----------------------------------------------------------------------------

// Debug Logging
#ifndef IMGUI_DEBUG_LOG
#define IMGUI_DEBUG_LOG(_FMT,...)       printf("[%05d] " _FMT, GImGui->FrameCount, __VA_ARGS__)
#endif

// Debug Logging for selected systems. Remove the '((void)0) //' to enable.
//#define IMGUI_DEBUG_LOG_POPUP         IMGUI_DEBUG_LOG // Enable log
//#define IMGUI_DEBUG_LOG_NAV           IMGUI_DEBUG_LOG // Enable log
#define IMGUI_DEBUG_LOG_POPUP(...)      ((void)0)       // Disable log
#define IMGUI_DEBUG_LOG_NAV(...)        ((void)0)       // Disable log

// Static Asserts
#if (__cplusplus >= 201100) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201100)
#define IM_STATIC_ASSERT(_COND)         static_assert(_COND, "")
#else
#define IM_STATIC_ASSERT(_COND)         typedef char static_assertion_##__line__[(_COND)?1:-1]
#endif

// "Paranoid" Debug Asserts are meant to only be enabled during specific debugging/work, otherwise would slow down the code too much.
// We currently don't have many of those so the effect is currently negligible, but onward intent to add more aggressive ones in the code.
//#define IMGUI_DEBUG_PARANOID
#ifdef IMGUI_DEBUG_PARANOID
#define IM_ASSERT_PARANOID(_EXPR)       IM_ASSERT(_EXPR)
#else
#define IM_ASSERT_PARANOID(_EXPR)
#endif

// Error handling
// Down the line in some frameworks/languages we would like to have a way to redirect those to the programmer and recover from more faults.
#ifndef IM_ASSERT_USER_ERROR
#define IM_ASSERT_USER_ERROR(_EXP,_MSG) IM_ASSERT((_EXP) && _MSG)   // Recoverable User Error
#endif

// Misc Macros
#define IM_PI                           3.14159265358979323846f
#ifdef _WIN32
#define IM_NEWLINE                      "\r\n"   // Play it nice with Windows users (Update: since 2018-05, Notepad finally appears to support Unix-style carriage returns!)
#else
#define IM_NEWLINE                      "\n"
#endif
#define IM_TABSIZE                      (4)
#define IM_MEMALIGN(_OFF,_ALIGN)        (((_OFF) + (_ALIGN - 1)) & ~(_ALIGN - 1))               // Memory align e.g. IM_ALIGN(0,4)=0, IM_ALIGN(1,4)=4, IM_ALIGN(4,4)=4, IM_ALIGN(5,4)=8
#define IM_F32_TO_INT8_UNBOUND(_VAL)    ((int)((_VAL) * 255.0f + ((_VAL)>=0 ? 0.5f : -0.5f)))   // Unsaturated, for display purpose
#define IM_F32_TO_INT8_SAT(_VAL)        ((int)(ImSaturate(_VAL) * 255.0f + 0.5f))               // Saturated, always output 0..255
#define IM_FLOOR(_VAL)                  ((float)(int)(_VAL))                                    // ImFloor() is not inlined in MSVC debug builds
#define IM_ROUND(_VAL)                  ((float)(int)((_VAL) + 0.5f))                           //

// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifdef _MSC_VER
#define IMGUI_CDECL __cdecl
#else
#define IMGUI_CDECL
#endif

// Warnings
#if defined(_MSC_VER) && !defined(__clang__)
#define IM_MSVC_WARNING_SUPPRESS(XXXX)  __pragma(warning(suppress: XXXX))
#else
#define IM_MSVC_WARNING_SUPPRESS(XXXX)
#endif

// Debug Tools
// Use 'Metrics->Tools->Item Picker' to break into the call-stack of a specific item.
#ifndef IM_DEBUG_BREAK
#if defined(__clang__)
#define IM_DEBUG_BREAK()    __builtin_debugtrap()
#elif defined (_MSC_VER)
#define IM_DEBUG_BREAK()    __debugbreak()
#else
#define IM_DEBUG_BREAK()    IM_ASSERT(0)    // It is expected that you define IM_DEBUG_BREAK() into something that will break nicely in a debugger!
#endif
#endif // #ifndef IM_DEBUG_BREAK

