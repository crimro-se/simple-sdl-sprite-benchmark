cmake_minimum_required(VERSION 3.16)
option(VITA "Build for PS Vita" OFF)

if(VITA)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
    endif()
endif()

project(bench) 
set(CMAKE_C_STANDARD 99)
set(VITA_APP_NAME "An SDL Sprite Benchmark")
set(VITA_TITLEID  "CRSE00001")
set(VITA_VERSION  "01.00")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
include(FindPkgConfig)
option(USE_SDL3 "Build with SDL3 instead of SDL2" ON)

if(VITA)
    include("${VITASDK}/share/vita.cmake" REQUIRED)
endif()

if(USE_SDL3)
    add_executable(${PROJECT_NAME} sdl3.c)
    target_compile_options(${PROJECT_NAME} PRIVATE -DSDL3)
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

# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -pg -g")

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

if(VITA)
    ## Create Vita files
    vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
    # The FILE directive lets you add additional files to the VPK, the syntax is
    # FILE src_path dst_path_in_vpk. In this case, we add the LiveArea paths.
    vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
        VERSION ${VITA_VERSION}
        NAME ${VITA_APP_NAME}
        FILE sce_sys/icon0.png sce_sys/icon0.png
        FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
        FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
        FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
    )
endif()
