cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
project(ProjectBulldog)

find_package(Vulkan REQUIRED)
find_package(simdjson REQUIRED)

add_subdirectory(sandbox)

add_subdirectory(third_party)
add_subdirectory(orion)

find_program(SLANGC slangc HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
file(GLOB_RECURSE SLANG_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/assets/shaders/**/*.frag"
    "${PROJECT_SOURCE_DIR}/assets/shaders/**/*.vert"
    "${PROJECT_SOURCE_DIR}/assets/shaders/**/*.comp"
    )
foreach(SLANG ${SLANG_SOURCE_FILES})
  message(STATUS "BUILDING SHADER: ${SLANG}")
  get_filename_component(FILE_NAME ${SLANG} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/assets/shaders/${FILE_NAME}.spv")
  message(STATUS COMMAND ${SLANGC} -V ${SLANG} -o ${SPIRV})
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${SLANGC}
      -v ${SLANG} 
      -o ${SPIRV}
      -target spirv
      -profile spirv_1_4
      -emit-spirv-directly
      -fvk-use-entrypoint-name
    DEPENDS ${SLANGC})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(SLANG)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )
