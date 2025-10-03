# Resource manager/CMakeLists.txt
#set(RESOURCES_NAME Resources)

# file(GLOB_RECURSE PROJECT_FILES 
    # ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
# )

# source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_FILES})

# add_library(${DATA_NAME})
# target_sources(${DATA_NAME} PRIVATE ${PROJECT_FILES})

# target_include_directories(${DATA_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/header)
# target_include_directories(${DATA_NAME} PRIVATE ${LIB_INCLUDE_DIR})
# # Link with dependencies
# target_link_libraries(${DATA_NAME}
    # PRIVATE Dependencies          
    # PRIVATE opengl32
    # PRIVATE user32
    # PRIVATE gdi32
    # PRIVATE shell32
    # #PRIVATE ${LIB_NAME}          
# )

# set(DATA_NAME ${DATA_NAME} PARENT_SCOPE) #outer scope variable only

# set(DATA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Header PARENT_SCOPE)

# # Post-build copy for DLLs
# add_custom_command(TARGET ${DATA_NAME} POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E copy_directory
        # "${CMAKE_SOURCE_DIR}/Dependencies/binairies"
        # $<TARGET_FILE_DIR:${DATA_NAME}>
# )

set(RESOURCES_NAME Resources) #Local variable only

file(GLOB_RECURSE PROJECT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/header/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

add_library(${RESOURCES_NAME})

target_sources(${RESOURCES_NAME} PRIVATE ${PROJECT_FILES})

target_include_directories(${RESOURCES_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Header)
target_include_directories(${RESOURCES_NAME} PRIVATE ${LIB_INCLUDE_DIR})
target_link_libraries(${RESOURCES_NAME}
    PRIVATE Dependencies 
    PRIVATE opengl32
    PRIVATE user32
    PRIVATE gdi32
    PRIVATE shell32
    PRIVATE ${LIB_NAME}  
)
set(RESOURCES_NAME ${RESOURCES_NAME} PARENT_SCOPE) #outer scope variable only

set(RESOURCES_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Header PARENT_SCOPE)
#This command sets the LIB_INCLUDE_DIR variable to the path of the 'include' directory
