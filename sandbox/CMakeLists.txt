
add_executable(sandbox
  ${CMAKE_SOURCE_DIR}/orion/entry.cc
  game.cc
)



target_link_libraries(sandbox PRIVATE orion)

IF(WIN32)
set_target_properties(sandbox PROPERTIES WIN32_EXECUTABLE FALSE)
target_link_options(sandbox PRIVATE
  -static-libgcc
  -static-libstdc++
  -static -lwinpthread
)
ENDIF()
