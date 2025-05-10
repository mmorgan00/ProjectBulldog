

# Add source to this project's executable.
add_executable (engine
    main.cpp
    engine.cpp
    engine.h
    engine_types.h
    vk_initializers.cpp
    vk_initializers.h
    vk_descriptors.cpp
    vk_descriptors.h
    vk_pipelines.cpp
    vk_pipelines.h
    vk_images.cpp
    vk_images.h
    vk_loader.cpp
    vk_loader.h
 "camera.h" "camera.cpp")


set_property(TARGET engine PROPERTY CXX_STANDARD 20)
target_compile_definitions(engine PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(engine PUBLIC vma glm Vulkan::Vulkan fmt::fmt stb_image SDL2::SDL2 vkbootstrap imgui fastgltf::fastgltf)

target_precompile_headers(engine PUBLIC <optional> <vector> <memory> <string> <vector> <unordered_map> <glm/mat4x4.hpp>  <glm/vec4.hpp> <vulkan/vulkan.h>)

if(WIN32)
 add_custom_command(TARGET engine POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:engine> $<TARGET_FILE_DIR:engine>
   COMMAND_EXPAND_LISTS
  )
endif()

# Debug target
add_executable(debug
    main.cpp
    engine.cpp
    engine.h
    engine_types.h
    vk_initializers.cpp
    vk_initializers.h
    vk_descriptors.cpp
    vk_descriptors.h
    vk_pipelines.cpp
    vk_pipelines.h
    vk_images.cpp
    vk_images.h
    vk_loader.cpp
    vk_loader.h

 "camera.h" "camera.cpp")

set_target_properties(debug PROPERTIES 
    CXX_STANDARD 20
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}"
    LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
)

target_compile_definitions(debug PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(debug PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(debug PUBLIC 
    vma 
    glm 
    Vulkan::Vulkan 
    fmt::fmt 
    stb_image 
    SDL2::SDL2 
    vkbootstrap 
    imgui 
    fastgltf::fastgltf
)

target_precompile_headers(debug PUBLIC 
    <optional> 
    <vector> 
    <memory> 
    <string> 
    <vector> 
    <unordered_map> 
    <glm/mat4x4.hpp>  
    <glm/vec4.hpp> 
    <vulkan/vulkan.h>
)
if(WIN32)
 add_custom_command(TARGET debug POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:debug> $<TARGET_FILE_DIR:debug>
     COMMAND_EXPAND_LISTS
 )
 endif()
# Add dependency on Shaders target from parent
add_dependencies(engine Shaders)
add_dependencies(debug Shaders)
