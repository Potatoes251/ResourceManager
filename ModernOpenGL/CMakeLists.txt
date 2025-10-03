# # OpenGLSetup/CMakeLists.txt

# file(GLOB_RECURSE PROJECT_FILES 
    # ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
# )

# source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_FILES})

# add_executable(${EXE_NAME})
# target_sources(${EXE_NAME} PRIVATE ${PROJECT_FILES})

# target_include_directories(${EXE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/header)
# target_include_directories(${EXE_NAME} PRIVATE ${LIB_INCLUDE_DIR})
# target_include_directories(${EXE_NAME} PRIVATE ${RESOURCE_INCLUDE_DIR})
# # Link with dependencies
# target_link_libraries(${EXE_NAME}
    # PRIVATE Dependencies          
    # PRIVATE opengl32
    # PRIVATE user32
    # PRIVATE gdi32
    # PRIVATE shell32
    # PRIVATE ${LIB_NAME}  
	# PRIVATE ${RESOURCE}
# )

# # Post-build copy for DLLs
# add_custom_command(TARGET ${EXE_NAME} POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E copy_directory
        # "${CMAKE_SOURCE_DIR}/Dependencies/binairies"
        # $<TARGET_FILE_DIR:${EXE_NAME}>
# )

# Core/CMakeLists.txt

# Collect code files
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/header/*.h)
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

# Collect non-code files
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl)
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/**/*.obj
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/**/*.tga
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/**/*.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/**/*.png
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

# Only add compilable source files to target_sources
target_sources(${EXE_NAME} PRIVATE
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCES}
)

# Add shaders & assets as header-only sources (so they show in IDE but aren't compiled)
foreach(file IN LISTS SHADER_FILES ASSET_FILES)
    set_source_files_properties(${file} PROPERTIES HEADER_FILE_ONLY TRUE)
    target_sources(${EXE_NAME} PRIVATE ${file})
endforeach()

# Include directories
target_include_directories(${EXE_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/header
    PRIVATE ${LIB_INCLUDE_DIR}
    PRIVATE ${DAT_INCLUDE_DIR}
    PRIVATE ${LOW_INCLUDE_DIR}
    #PRIVATE ${PHYSICS_INCLUDE_DIR}
    PRIVATE ${RESOURCES_INCLUDE_DIR}
)

# Link libraries
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
    #PRIVATE ${PHYSICS_NAME}
    
)

# # Post-build copy for DLLs
# add_custom_command(TARGET ${EXE_NAME} POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E copy_directory
        # "${CMAKE_SOURCE_DIR}/dependencies/binairies"
        # $<TARGET_FILE_DIR:${EXE_NAME}>
# )