
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

target_compile_definitions(sandbox PRIVATE
  ENGINE_CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/config/engine.conf")
  target_compile_definitions(sandbox PRIVATE
  ENGINE_ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets")
  
find_program(SLANGC slangc
  NAMES slangc slang.exe
  PATHS 
    $ENV{VULKAN_SDK}/Bin
    $ENV{VULKAN_SDK}/Bin32
    $ENV{PROGRAMFILES}/Slang/bin
  NO_DEFAULT_PATH
)

if(NOT SLANGC)
  message(FATAL_ERROR "Slang compiler (slangc) not found. Please install from https://github.com/shader-slang/slang/releases/")
endif()

message(STATUS "Found SLANGC: ${SLANGC}")


file(GLOB_RECURSE SLANG_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/**/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/**/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/**/*.comp"
    )
foreach(SLANG ${SLANG_SOURCE_FILES})
  message(STATUS "BUILDING SHADER: ${SLANG}")
  get_filename_component(FILE_NAME ${SLANG} NAME)
  set(SPIRV "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/${FILE_NAME}.spv")
  message(STATUS COMMAND ${SLANGC} -V ${SLANG} -o ${SPIRV})
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${SLANGC}
      -v ${SLANG} 
      -o ${SPIRV}
      -target spirv
      -profile spirv_1_4
      -matrix-layout-column-major
      -emit-spirv-directly
      -fvk-use-entrypoint-name
    DEPENDS ${SLANGC})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(SLANG)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )
add_dependencies(sandbox Shaders)