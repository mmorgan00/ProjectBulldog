cmake_minimum_required (VERSION 3.8)

add_library(OE_DEMO STATIC entry.cpp)

target_include_directories(OE_DEMO PUBLIC ${CMAKE_SOURCE_DIR}/orion)

target_link_libraries(OE_DEMO fmt::fmt)




