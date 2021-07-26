
add_executable(ExampleDemo ./ExampleDemo.cpp ./imgui_impl_opengl3.cpp
                       ./imgui_impl_glfw.cpp)
target_link_libraries(ExampleDemo imgui glad glfw )
