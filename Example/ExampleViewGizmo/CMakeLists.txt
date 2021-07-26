
add_executable(ExampleViewGizmo ./ExampleViewGizmo.cpp ./imgui_impl_opengl3.cpp
                       ./imgui_impl_glfw.cpp)
target_link_libraries(ExampleViewGizmo imgui glad glfw )
