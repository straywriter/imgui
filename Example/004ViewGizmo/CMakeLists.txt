find_package(OpenGL REQUIRED)
add_executable(004ViewGizmo ./004ViewGizmo.cpp)
target_link_libraries(004ViewGizmo ExampleCommon ${OPENGL_LIBRARIES})
