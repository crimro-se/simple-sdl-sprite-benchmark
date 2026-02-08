cmake_minimum_required(VERSION 3.11)
project(BENCH)
include(FindPkgConfig)
option(USE_SDL3 "Build with SDL3 instead of SDL2" ON)

if(USE_SDL3)
    add_executable(${PROJECT_NAME} sdl3.c)
    
    pkg_search_module(SDL3 REQUIRED sdl3)
    
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL3_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL3_LIBRARIES})

else()
    add_executable(${PROJECT_NAME} sdl2.c)
    
    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
    
    target_include_directories(${PROJECT_NAME} PRIVATE 
        ${SDL2_INCLUDE_DIRS} 
        ${SDL2_IMAGE_INCLUDE_DIRS}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ${SDL2_LIBRARIES} 
        ${SDL2_IMAGE_LIBRARIES}
    )
endif()

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)

# Create an EBOOT.PBP file
if(PSP)
create_pbp_file(
    TARGET ${PROJECT_NAME}
    ICON_PATH NULL
    BACKGROUND_PATH NULL
    PREVIEW_PATH NULL
    TITLE ${PROJECT_NAME}
    VERSION 01.00
)
endif()