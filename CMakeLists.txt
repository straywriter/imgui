cmake_minimum_required(VERSION 3.10)

project(ImGui)

# External Library
add_subdirectory(External)

# ImGui
file(GLOB ImGuiHeaderFile ${CMAKE_CURRENT_SOURCE_DIR}/Include/ImGui/*.h)
file(GLOB ImGuiSourceFile ${CMAKE_CURRENT_SOURCE_DIR}/Src/ImGui/*.cpp)

message(${ImGuiHeaderFile})
message(${ImGuiSourceFile})

add_library(imgui ${ImGuiHeaderFile} ${ImGuiSourceFile})
target_include_directories(
  imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include
               ${CMAKE_CURRENT_SOURCE_DIR}/Include/ImGui/)

add_executable(Example Example/Example.cpp Example/imgui_impl_opengl3.cpp
                       Example/imgui_impl_glfw.cpp)
target_link_libraries(Example imgui glad glfw)
