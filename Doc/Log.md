

## 动态导入字体





## 添加文本编辑器 成功



## 应用框架



## 字体设计软件

[aiekick/SdfFontDesigner: Offline font tuning/bitmap generation via shaders (github.com)](https://github.com/aiekick/SdfFontDesigner)





## C++ clang format 头文件排序问题

设置sort 不排序



## C++ clang format 

[clang-format的介绍和使用 - Tudou_Blog - 博客园 (cnblogs.com)](https://www.cnblogs.com/__tudou__/p/13322854.html)



```
# 语言: None, Cpp, Java, JavaScript, ObjC, Proto, TableGen, TextProto
Language: Cpp
# BasedOnStyle:	LLVM

# 访问说明符(public、private等)的偏移
AccessModifierOffset: -4

# 开括号(开圆括号、开尖括号、开方括号)后的对齐: Align, DontAlign, AlwaysBreak(总是在开括号后换行)
AlignAfterOpenBracket: Align

# 连续赋值时，对齐所有等号
AlignConsecutiveAssignments: false

# 连续声明时，对齐所有声明的变量名
AlignConsecutiveDeclarations: false

# 右对齐逃脱换行(使用反斜杠换行)的反斜杠
AlignEscapedNewlines: Right

# 水平对齐二元和三元表达式的操作数
AlignOperands: true

# 对齐连续的尾随的注释
AlignTrailingComments: true

# 不允许函数声明的所有参数在放在下一行
AllowAllParametersOfDeclarationOnNextLine: false

# 不允许短的块放在同一行
AllowShortBlocksOnASingleLine: true

# 允许短的case标签放在同一行
AllowShortCaseLabelsOnASingleLine: true

# 允许短的函数放在同一行: None, InlineOnly(定义在类中), Empty(空函数), Inline(定义在类中，空函数), All
AllowShortFunctionsOnASingleLine: None

# 允许短的if语句保持在同一行
AllowShortIfStatementsOnASingleLine: true

# 允许短的循环保持在同一行
AllowShortLoopsOnASingleLine: true

# 总是在返回类型后换行: None, All, TopLevel(顶级函数，不包括在类中的函数), 
# AllDefinitions(所有的定义，不包括声明), TopLevelDefinitions(所有的顶级函数的定义)
AlwaysBreakAfterReturnType: None

# 总是在多行string字面量前换行
AlwaysBreakBeforeMultilineStrings: false

# 总是在template声明后换行
AlwaysBreakTemplateDeclarations: true

# false表示函数实参要么都在同一行，要么都各自一行
BinPackArguments: true

# false表示所有形参要么都在同一行，要么都各自一行
BinPackParameters: true

# 大括号换行，只有当BreakBeforeBraces设置为Custom时才有效
BraceWrapping:
  # class定义后面
  AfterClass: false
  # 控制语句后面
  AfterControlStatement: false
  # enum定义后面
  AfterEnum: false
  # 函数定义后面
  AfterFunction: false
  # 命名空间定义后面
  AfterNamespace: false
  # struct定义后面
  AfterStruct: false
  # union定义后面
  AfterUnion: false
  # extern之后
  AfterExternBlock: false
  # catch之前
  BeforeCatch: false
  # else之前
  BeforeElse: false
  # 缩进大括号
  IndentBraces: false
  # 分离空函数
  SplitEmptyFunction: false
  # 分离空语句
  SplitEmptyRecord: false
  # 分离空命名空间
  SplitEmptyNamespace: false

# 在二元运算符前换行: None(在操作符后换行), NonAssignment(在非赋值的操作符前换行), All(在操作符前换行)
BreakBeforeBinaryOperators: NonAssignment

# 在大括号前换行: Attach(始终将大括号附加到周围的上下文), Linux(除函数、命名空间和类定义，与Attach类似), 
#   Mozilla(除枚举、函数、记录定义，与Attach类似), Stroustrup(除函数定义、catch、else，与Attach类似), 
#   Allman(总是在大括号前换行), GNU(总是在大括号前换行，并对于控制语句的大括号增加额外的缩进), WebKit(在函数前换行), Custom
#   注：这里认为语句块也属于函数
BreakBeforeBraces: Custom

# 在三元运算符前换行
BreakBeforeTernaryOperators: false

# 在构造函数的初始化列表的冒号后换行
BreakConstructorInitializers: AfterColon

#BreakInheritanceList: AfterColon

BreakStringLiterals: false

# 每行字符的限制，0表示没有限制
ColumnLimit: 0

CompactNamespaces: true

# 构造函数的初始化列表要么都在同一行，要么都各自一行
ConstructorInitializerAllOnOneLineOrOnePerLine: false

# 构造函数的初始化列表的缩进宽度
ConstructorInitializerIndentWidth: 4

# 延续的行的缩进宽度
ContinuationIndentWidth: 4

# 去除C++11的列表初始化的大括号{后和}前的空格
Cpp11BracedListStyle: true

# 继承最常用的指针和引用的对齐方式
DerivePointerAlignment: false

# 固定命名空间注释
FixNamespaceComments: true

# 缩进case标签
IndentCaseLabels: false

IndentPPDirectives: None

# 缩进宽度
IndentWidth: 4

# 函数返回类型换行时，缩进函数声明或函数定义的函数名
IndentWrappedFunctionNames: false

# 保留在块开始处的空行
KeepEmptyLinesAtTheStartOfBlocks: false

# 连续空行的最大数量
MaxEmptyLinesToKeep: 1

# 命名空间的缩进: None, Inner(缩进嵌套的命名空间中的内容), All
NamespaceIndentation: None

# 指针和引用的对齐: Left, Right, Middle
PointerAlignment: Right

# 允许重新排版注释
ReflowComments: true

# 允许排序#include
SortIncludes: false

# 允许排序 using 声明
SortUsingDeclarations: false

# 在C风格类型转换后添加空格
SpaceAfterCStyleCast: false

# 在Template 关键字后面添加空格
SpaceAfterTemplateKeyword: true

# 在赋值运算符之前添加空格
SpaceBeforeAssignmentOperators: true

# SpaceBeforeCpp11BracedList: true

# SpaceBeforeCtorInitializerColon: true

# SpaceBeforeInheritanceColon: true

# 开圆括号之前添加一个空格: Never, ControlStatements, Always
SpaceBeforeParens: ControlStatements

# SpaceBeforeRangeBasedForLoopColon: true

# 在空的圆括号中添加空格
SpaceInEmptyParentheses: false

# 在尾随的评论前添加的空格数(只适用于//)
SpacesBeforeTrailingComments: 1

# 在尖括号的<后和>前添加空格
SpacesInAngles: false

# 在C风格类型转换的括号中添加空格
SpacesInCStyleCastParentheses: false

# 在容器(ObjC和JavaScript的数组和字典等)字面量中添加空格
SpacesInContainerLiterals: true

# 在圆括号的(后和)前添加空格
SpacesInParentheses: false

# 在方括号的[后和]前添加空格，lamda表达式和未指明大小的数组的声明不受影响
SpacesInSquareBrackets: false

# 标准: Cpp03, Cpp11, Auto
Standard: Cpp11

# tab宽度
TabWidth: 4

# 使用tab字符: Never, ForIndentation, ForContinuationAndIndentation, Always
UseTab: Never
```



编辑器

[Clang Power Tools | Bringing clang-tidy magic to Visual Studio C++ developers](https://clangpowertools.com/clang-format-editor.html)

[Caphyon/clang-format-editor: Clang-Format Editor is a tool that helps you find the best Clang-Format Style for your C++, C#, Java, JavaScript, and Objective-C code. (github.com)](https://github.com/Caphyon/clang-format-editor)

[Getting started with Clang-Format Style Options (clangpowertools.com)](https://clangpowertools.com/blog/getting-started-with-clang-format-style-options.html)

[My clang format file. In sample.cpp is sample output (github.com)](https://gist.github.com/gelldur/d7bc3ea226aebcf8cc879df1e8524236#file-sample-cpp)

[Clang-Format-Options 中文翻译 （未完待续）_S1xe的博客-CSDN博客_clang-format off](https://blog.csdn.net/qq_31359295/article/details/75016195)

[Clang-Format格式化选项介绍_子丰的博客-CSDN博客](https://blog.csdn.net/softimite_zifeng/article/details/78357898)

[Clang-Format Style Options[翻译\] - 简书 (jianshu.com)](https://www.jianshu.com/p/5dea6bdbbabb)	

## 添加文本编辑器



[BalazsJako/ImGuiColorTextEdit: Colorizing text editor for ImGui (github.com)](https://github.com/BalazsJako/ImGuiColorTextEdit)

[Rezonality/zep: Zep - An embeddable editor, with optional support for using vim keystrokes. (github.com)](https://github.com/Rezonality/zep)

[Scintilla for ImGui · Issue #108 · ocornut/imgui (github.com)](https://github.com/ocornut/imgui/issues/108)

## freetype 使用





## ImGuiFontStudio 使用

合并 字体，，

种种原因 放弃



视频 

[ImGuiFontStudio Progress on Vimeo](https://vimeo.com/395328316)

## 添加文件管理器



[aiekick/ImGuiFileDialog: File Dialog for Dear ImGui (github.com)](https://github.com/aiekick/ImGuiFileDialog)



相关参考

[aiekick/ImGuiFileDialog: File Dialog for Dear ImGui (github.com)](https://github.com/aiekick/ImGuiFileDialog)



## cmake 拷贝目录



```cmake
add_custom_command(
  TARGET ExampleDemoStyle
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Font/
    $<TARGET_FILE_DIR:ExampleDemoStyle>/Font)
```



相关参考

[c++ - How to copy contents of a directory into build directory after make with CMake? - Stack Overflow](https://stackoverflow.com/questions/13429656/how-to-copy-contents-of-a-directory-into-build-directory-after-make-with-cmake)



## 导入字体代码记录

```
io.Fonts->AddFontFromFileTTF("c:/windows/fonts/simhei.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 20.0f);
```



## 修改关闭按钮

更改源代码

```

// Button to close a window
bool ImGui::CloseButton(ImGuiID id, const ImVec2& pos)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Tweak 1: Shrink hit-testing area if button covers an abnormally large proportion of the visible region. That's in order to facilitate moving the window away. (#3825)
    // This may better be applied as a general hit-rect reduction mechanism for all widgets to ensure the area to move window is always accessible?
    const ImRect bb(pos, pos + ImVec2(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
    ImRect bb_interact = bb;
    const float area_to_visible_ratio = window->OuterRectClipped.GetArea() / bb.GetArea();
    if (area_to_visible_ratio < 1.5f)
        bb_interact.Expand(ImFloor(bb_interact.GetSize() * -0.25f));

    // Tweak 2: We intentionally allow interaction when clipped so that a mechanical Alt,Right,Activate sequence can always close a window.
    // (this isn't the regular behavior of buttons, but it doesn't affect the user much because navigation tends to keep items visible).
    bool is_clipped = !ItemAdd(bb_interact, id);

    bool hovered, held;
    bool pressed = ButtonBehavior(bb_interact, id, &hovered, &held);
    if (is_clipped)
        return pressed;

    // Render
    // FIXME: Clarify this mess
    ImU32 col = GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
    ImVec2 center = bb.GetCenter();
    if (hovered)
        window->DrawList->AddCircleFilled(center, ImMax(2.0f, g.FontSize * 0.5f + 1.0f), col, 12);

    float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;
    ImU32 cross_col = GetColorU32(ImGuiCol_Text);
    center -= ImVec2(0.5f, 0.5f);
    window->DrawList->AddLine(center + ImVec2(+cross_extent, +cross_extent), center + ImVec2(-cross_extent, -cross_extent), cross_col, 1.0f);
    window->DrawList->AddLine(center + ImVec2(+cross_extent, -cross_extent), center + ImVec2(-cross_extent, +cross_extent), cross_col, 1.0f);

    return pressed;
}
```



```

// Button to close a window
bool ImGui::CloseButton(ImGuiID id, const ImVec2 &pos)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;

    // Tweak 1: Shrink hit-testing area if button covers an abnormally large proportion of the visible region. That's in
    // order to facilitate moving the window away. (#3825) This may better be applied as a general hit-rect reduction
    // mechanism for all widgets to ensure the area to move window is always accessible?
    const ImRect bb(pos, pos + ImVec2(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
    ImRect bb_interact = bb;
    const float area_to_visible_ratio = window->OuterRectClipped.GetArea() / bb.GetArea();
    if (area_to_visible_ratio < 1.5f)
        bb_interact.Expand(ImFloor(bb_interact.GetSize() * -0.25f));

    // Tweak 2: We intentionally allow interaction when clipped so that a mechanical Alt,Right,Activate sequence can
    // always close a window. (this isn't the regular behavior of buttons, but it doesn't affect the user much because
    // navigation tends to keep items visible).
    bool is_clipped = !ItemAdd(bb_interact, id);

    bool hovered, held;
    bool pressed = ButtonBehavior(bb_interact, id, &hovered, &held);
    if (is_clipped)
        return pressed;

    // Render
    // FIXME: Clarify this mess
    ImU32 col = GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
    ImVec2 center = bb.GetCenter();

    float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;

    float rect_extent = g.FontSize * 0.5f - 1.0f;

    ImU32 cross_col = GetColorU32(ImGuiCol_Text);
    ImU32 red_color = ColorConvertFloat4ToU32(ImVec4(0.9f, 0.1f, 0.f, 0.8f));
    ImU32 write_color = ColorConvertFloat4ToU32(ImVec4(0.14f, 0.14f, 0.14f, 0.8f));
    center -= ImVec2(0.5f, 0.5f);
    if (hovered)
        window->DrawList->AddRectFilled(center + ImVec2(-rect_extent, -rect_extent),
                                        center + ImVec2(+rect_extent, +rect_extent), red_color, 2.f);

    window->DrawList->AddLine(center + ImVec2(+cross_extent, +cross_extent) * 0.9f,
                              center + ImVec2(-cross_extent, -cross_extent) * 0.9f, cross_col, 3.0f);
    window->DrawList->AddLine(center + ImVec2(+cross_extent, -cross_extent) * 0.9f,
                              center + ImVec2(-cross_extent, +cross_extent) * 0.9f, cross_col, 3.0f);

    return pressed;
}
```



## imgui 修改标题栏



相关参考

如何自定义标题栏元素

[如何自定义标题栏元素？问题#1539 •奥科努特/伊姆吉 (github.com)](https://github.com/ocornut/imgui/issues/1539)



## PushStyleColor 使用

和    `ImGui::PopStyleColor();` 配合使用

```
ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(4 / 7.0f, 0.6f, 0.6f));
ImGui::Button(ICON_FA_WINDOW_CLOSE);
ImGui::PopStyleColor();
```



## 修改按钮字体颜色实现图标颜色



方案一

```

void beginTextColor()
{
     ImGuiStyle *style = &ImGui::GetStyle();
    ImVec4 *colors = style->Colors;
    colors[ImGuiCol_Text] = ImVec4(1.f, 0.f, 0.f, 1.00f);
}

void endTextColor()
{
       ImGuiStyle *style = &ImGui::GetStyle();
    ImVec4 *colors = style->Colors;
    colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
}

```

导致运行时 无法更改主题



方案二

```
PushStyleColor
```





## 按钮修改颜色





相关参考

[Selected ImGui::Button & Change Color · Issue #777 · ocornut/imgui (github.com)](https://github.com/ocornut/imgui/issues/777)

[c++ - How can I change text color of my InputText in ImGui? (unanswered) - Stack Overflow](https://stackoverflow.com/questions/61853584/how-can-i-change-text-color-of-my-inputtext-in-imgui-unanswered)

[Selected ImGui::Button & Change Color · Issue #777 · ocornut/imgui (github.com)](https://github.com/ocornut/imgui/issues/777)

## 显示图标



> ## Icon Fonts
>
> ### [Font Awesome](https://fontawesome.com/)
>
> - https://github.com/FortAwesome/Font-Awesome
>
> #### Font Awesome 4
>
> - [icons.yml](https://github.com/FortAwesome/Font-Awesome/blob/fa-4/src/icons.yml)
> - [fontawesome-webfont.ttf](https://github.com/FortAwesome/Font-Awesome/blob/fa-4/fonts/fontawesome-webfont.ttf)
>
> #### Font Awesome 5 - see [notes below](https://github.com/juliettef/IconFontCppHeaders#notes-about-font-awesome-5)
>
> - [icons.yml](https://github.com/FortAwesome/Font-Awesome/blob/master/metadata/icons.yml)
> - [fa-brands-400.ttf](https://github.com/FortAwesome/Font-Awesome/blob/master/webfonts/fa-brands-400.ttf)
> - [fa-regular-400.ttf](https://github.com/FortAwesome/Font-Awesome/blob/master/webfonts/fa-regular-400.ttf)
> - [fa-solid-900.ttf](https://github.com/FortAwesome/Font-Awesome/blob/master/webfonts/fa-solid-900.ttf)
>
> #### Font Awesome 5 Pro - this is a paid product, see [notes below](https://github.com/juliettef/IconFontCppHeaders#notes-about-font-awesome-5)
>
> Files downloaded from [fontawesome.com](https://fontawesome.com/)
>
> - ..\fontawesome-pro-n.n.n-web\metadata\icons.yml
> - ..\fontawesome-pro-n.n.n-web\webfonts\fa-brands-400.ttf
> - ..\fontawesome-pro-n.n.n-web\webfonts\fa-light-300.ttf
> - ..\fontawesome-pro-n.n.n-web\webfonts\fa-regular-400.ttf
> - ..\fontawesome-pro-n.n.n-web\webfonts\fa-solid-900.ttf
>
> ### [Fork Awesome](https://forkawesome.github.io/Fork-Awesome)
>
> - https://github.com/ForkAwesome/Fork-Awesome
> - [icons.yml](https://github.com/ForkAwesome/Fork-Awesome/blob/master/src/icons/icons.yml)
> - [forkawesome-webfont.ttf](https://github.com/ForkAwesome/Fork-Awesome/blob/master/fonts/forkawesome-webfont.ttf)
>
> ### [Google Material Design icons](https://design.google.com/icons) - see [Issue #19](https://github.com/juliettef/IconFontCppHeaders/issues/19)
>
> - https://github.com/google/material-design-icons
> - [codepoints](https://github.com/google/material-design-icons/blob/master/iconfont/codepoints)
> - [MaterialIcons-Regular.ttf](https://github.com/google/material-design-icons/blob/master/iconfont/MaterialIcons-Regular.ttf)
>
> ### [Kenney Game icons](http://kenney.nl/assets/game-icons) and [Game icons expansion](http://kenney.nl/assets/game-icons-expansion)
>
> - https://github.com/nicodinh/kenney-icon-font
> - [kenney-icons.css](https://github.com/nicodinh/kenney-icon-font/blob/master/css/kenney-icons.css)
> - [kenney-icon-font.ttf](https://github.com/nicodinh/kenney-icon-font/blob/master/fonts/kenney-icon-font.ttf)
>
> ### [Fontaudio](https://github.com/fefanto/fontaudio)
>
> - https://github.com/fefanto/fontaudio
> - [fontaudio.css](https://github.com/fefanto/fontaudio/blob/master/font/fontaudio.css)
> - [fontaudio.ttf](https://github.com/fefanto/fontaudio/blob/master/font/fontaudio.ttf)



相关参考

[juliettef/IconFontCppHeaders: C, C++ headers and C# classes for icon fonts: Font Awesome, Fork Awesome, Material Design, Kenney game icons and Fontaudio (github.com)](https://github.com/juliettef/IconFontCppHeaders)

https://github.com/aiekick/ImGuiFontStudio

## dock 使用





相关参考：

[Simple example, of how to use the dock builder API. (Adapted from the dock space example in the demo window) You need to use the docking branch and set the ImGuiConfigFlags_DockingEnable config flag. Learn more about Dear ImGui here: https://github.com/ocornut/imgui](https://gist.github.com/PossiblyAShrub/0aea9511b84c34e191eaa90dd7225969)

[Home · ocornut/imgui Wiki (github.com)](https://github.com/ocornut/imgui/wiki#docking)

[Docking · ocornut/imgui Wiki (github.com)](https://github.com/ocornut/imgui/wiki/Docking)

## 我的主题

```C++
    ImGuiStyle *style = &ImGui::GetStyle();
    ImVec4 *colors = style->Colors;

    colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.14f, 0.14f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.34f, 0.34f, 0.34f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.32f, 0.62f, 0.75f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.70f, 0.75f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.34f, 0.34f, 0.34f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 0.78f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.29f, 0.42f, 0.63f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.26f, 0.26f, 0.40f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.38f, 0.38f, 0.78f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.29f, 0.42f, 0.63f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.32f, 0.31f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.14f, 0.14f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.34f, 0.50f, 0.76f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.34f, 0.50f, 0.76f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style->ChildRounding = 4.0f;
    style->FrameBorderSize = 1.0f;
    style->FrameRounding = 1.0f;
    style->GrabMinSize = 12.0f;
    style->PopupRounding = 1.0f;
    style->ScrollbarRounding = 12.0f;
    style->ScrollbarSize = 13.0f;
    style->TabBorderSize = 1.0f;
    style->TabRounding = 3.0f;
    style->WindowRounding = 4.0f;
    style->ItemSpacing = ImVec2(8.f, 5.f);
    style->WindowRounding = 1.f;
```



## 使用字符字体



## 使用Consolas 汉字字体

[crvdgc/Consolas-with-Yahei: 向consolas字体中添加微软雅黑的字体，sublime-text中粗体、斜体显示正常，并且不会和系统内置的Consolas冲突。增加了powerline 字符。 (github.com)](https://github.com/crvdgc/Consolas-with-Yahei)



**相关参考**

[MATLAB使用自定义Consolas+Yahei字体解决中文乱码问题【2018-04-29】 - 简书 (jianshu.com)](https://www.jianshu.com/p/ffb9eddb8f21)

[adobe-fonts/source-han-sans: Source Han Sans | 思源黑体 | 思源黑體 | 思源黑體 香港 | 源ノ角ゴシック | 본고딕 (github.com)](https://github.com/adobe-fonts/source-han-sans)