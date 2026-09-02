
add_executable(sandbox
  ${CMAKE_SOURCE_DIR}/orion/entry.cc
  entry.cc
)


target_link_libraries(sandbox PRIVATE orion)
