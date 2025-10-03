# Dependencies/CMakeLists.txt

# Create actual Glad lib (C file)
add_library(Glad STATIC source/glad.c)
target_include_directories(Glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(Glad PROPERTIES FOLDER "Dependencies")

# Create GLFW imported lib
add_library(glfw3 STATIC IMPORTED GLOBAL)
set_target_properties(glfw3 PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    FOLDER "Dependencies"
)

# INTERFACE target to expose both to consumers
add_library(Dependencies INTERFACE)
target_link_libraries(Dependencies INTERFACE Glad glfw3)
target_include_directories(Dependencies INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(Dependencies PROPERTIES FOLDER "Dependencies")