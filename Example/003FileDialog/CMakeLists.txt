find_package(OpenGL REQUIRED)
add_executable(003FileDialog ./003FileDialog.cpp )

target_link_libraries(003FileDialog ExampleCommon ${OPENGL_LIBRARIES})

add_custom_command(
  TARGET 003FileDialog
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${FontDir}
          $<TARGET_FILE_DIR:003FileDialog>/Font)
