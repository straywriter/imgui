#pragma once

#include <ImGui/ImGuiDefine.h>
#include <ImGui/ImGuiType.h>

namespace ImGui
{


// Widgets: Trees
// - TreeNode functions return true when the node is open, in which case you need to also call TreePop() when you are
// finished displaying the tree node contents.
IMGUI_API bool TreeNode(const char *label);
IMGUI_API bool TreeNode(const char *str_id, const char *fmt, ...)
    IM_FMTARGS(2); // helper variation to easily decorelate the id from the displayed string. Read the FAQ about why and
                   // how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet().
IMGUI_API bool TreeNode(const void *ptr_id, const char *fmt, ...) IM_FMTARGS(2); // "
IMGUI_API bool TreeNodeV(const char *str_id, const char *fmt, va_list args) IM_FMTLIST(2);
IMGUI_API bool TreeNodeV(const void *ptr_id, const char *fmt, va_list args) IM_FMTLIST(2);
IMGUI_API bool TreeNodeEx(const char *label, ImGuiTreeNodeFlags flags = 0);
IMGUI_API bool TreeNodeEx(const char *str_id, ImGuiTreeNodeFlags flags, const char *fmt, ...) IM_FMTARGS(3);
IMGUI_API bool TreeNodeEx(const void *ptr_id, ImGuiTreeNodeFlags flags, const char *fmt, ...) IM_FMTARGS(3);
IMGUI_API bool TreeNodeExV(const char *str_id, ImGuiTreeNodeFlags flags, const char *fmt, va_list args) IM_FMTLIST(3);
IMGUI_API bool TreeNodeExV(const void *ptr_id, ImGuiTreeNodeFlags flags, const char *fmt, va_list args) IM_FMTLIST(3);
IMGUI_API void TreePush(const char *str_id); // ~ Indent()+PushId(). Already called by TreeNode() when returning true,
                                             // but you can call TreePush/TreePop yourself if desired.
IMGUI_API void TreePush(const void *ptr_id = NULL); // "
IMGUI_API void TreePop();                           // ~ Unindent()+PopId()
IMGUI_API float GetTreeNodeToLabelSpacing(); // horizontal distance preceding label when using TreeNode*() or Bullet()
                                             // == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode
IMGUI_API bool CollapsingHeader(
    const char *label, ImGuiTreeNodeFlags flags = 0); // if returning 'true' the header is open. doesn't indent nor push
                                                      // on ID stack. user doesn't have to call TreePop().
IMGUI_API bool CollapsingHeader(
    const char *label, bool *p_visible,
    ImGuiTreeNodeFlags flags = 0); // when 'p_visible != NULL': if '*p_visible==true' display an additional small close
                                   // button on upper right of the header which will set the bool to false when clicked,
                                   // if '*p_visible==false' don't display the header.
IMGUI_API void SetNextItemOpen(bool is_open, ImGuiCond cond = 0); // set next TreeNode/CollapsingHeader open state.


IMGUI_API bool TreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char *label, const char *label_end = NULL);
IMGUI_API bool TreeNodeBehaviorIsOpen(
    ImGuiID id,
    ImGuiTreeNodeFlags flags = 0); // Consume previous SetNextItemOpen() data, if any. May return true when logging
IMGUI_API void TreePushOverrideID(ImGuiID id);

} // namespace ImGui