# -----------------------------------
# Core/CMakeLists.txt
# -----------------------------------

# Collect code files
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/header/*.h)
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

# Collect non-code files (assets & shaders)
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl)
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}../assets/**/*.obj
    ${CMAKE_CURRENT_SOURCE_DIR}../assets/**/*.tga
    ${CMAKE_CURRENT_SOURCE_DIR}../assets/**/*.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}../assets/**/*.png
)

# Group files for IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES 
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCES}
    ${SHADER_FILES}
    ${ASSET_FILES}
)

# Create executable
add_executable(${EXE_NAME})

# Add source files
target_sources(${EXE_NAME} PRIVATE ${PROJECT_HEADERS} ${PROJECT_SOURCES})

# Add shaders & assets to IDE (not compiled)
foreach(file IN LISTS SHADER_FILES ASSET_FILES)
    set_source_files_properties(${file} PROPERTIES HEADER_FILE_ONLY TRUE)
    target_sources(${EXE_NAME} PRIVATE ${file})
endforeach()

# ---- Include directories ----
target_include_directories(${EXE_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/header
    PRIVATE ${LIB_INCLUDE_DIR}
    PRIVATE ${DAT_INCLUDE_DIR}
    PRIVATE ${LOW_INCLUDE_DIR}
    PRIVATE ${RESOURCES_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/External/irrKlang/include  # IrrKlang headers
)

# ---- Link libraries ----
target_link_libraries(${EXE_NAME}
    PRIVATE Dependencies
    PRIVATE opengl32
    PRIVATE user32 
    PRIVATE gdi32 
    PRIVATE shell32
    PRIVATE ${LIB_NAME}
    PRIVATE ${RESOURCES_NAME}
    PRIVATE ${DAT_NAME}
    PRIVATE ${LOW_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/External/irrKlang/lib/irrKlang.lib  # IrrKlang static link
)

# ---- Optional: Post-build copy for IrrKlang DLL ----
# This ensures the executable can find the DLL at runtime
add_custom_command(TARGET ${EXE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/External/irrKlang/bin/irrKlang.dll"
        $<TARGET_FILE_DIR:${EXE_NAME}>
)
