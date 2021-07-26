cmake_minimum_required(VERSION 3.10)

project(ImGui)

# External Library
add_subdirectory(External)

# ImGui
file(GLOB_RECURSE  ImGuiHeaderFile ${CMAKE_CURRENT_SOURCE_DIR}/Include/ImGui/*.h)
file(GLOB_RECURSE  ImGuiSourceFile ${CMAKE_CURRENT_SOURCE_DIR}/Src/ImGui/*.cpp)

message(${ImGuiHeaderFile})
message(${ImGuiSourceFile})

add_library(imgui ${ImGuiHeaderFile} ${ImGuiSourceFile})
target_include_directories(
  imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include
               ${CMAKE_CURRENT_SOURCE_DIR}/Include/ImGui/)
target_link_libraries(imgui PUBLIC stb)


add_subdirectory(Example)
