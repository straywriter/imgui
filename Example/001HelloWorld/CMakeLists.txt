find_package(OpenGL REQUIRED)
add_executable(001HelloWorld ./001HelloWorld.cpp)
target_link_libraries(001HelloWorld ExampleCommon ${OPENGL_LIBRARIES})
