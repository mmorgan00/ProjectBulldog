cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
project(ProjectBulldog)

find_package(Vulkan REQUIRED)
find_package(simdjson REQUIRED)

add_subdirectory(sandbox)

add_subdirectory(third_party)
add_subdirectory(orion)
