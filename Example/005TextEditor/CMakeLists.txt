find_package(OpenGL REQUIRED)
add_executable(005TextEditor ./005TextEditor.cpp)

target_link_libraries(005TextEditor ExampleCommon ${OPENGL_LIBRARIES})

add_custom_command(
  TARGET 005TextEditor
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${FontDir}
          $<TARGET_FILE_DIR:005TextEditor>/Font)
