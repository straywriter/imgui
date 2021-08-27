find_package(OpenGL REQUIRED)
add_executable(002Style ./002Style.cpp)
target_link_libraries(002Style ExampleCommon ${OPENGL_LIBRARIES})

add_custom_command(
  TARGET 002Style
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${FontDir}
          $<TARGET_FILE_DIR:002Style>/Font)
