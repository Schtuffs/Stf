cmake_minimum_required(VERSION 3.22)
project(Stf C)

set(CMAKE_C_STANDARD 23)
set(SRC ${CMAKE_CURRENT_LIST_DIR}/src)
set(LIB ${CMAKE_CURRENT_LIST_DIR}/lib)
set(TYPES ${SRC}/types)
set(UTILS ${SRC}/utils)

add_library(${PROJECT_NAME} STATIC
    ${SRC}/glad.c
    ${SRC}/stb_image.c
    ${SRC}/Stf.c
    ${SRC}/StfLoad.c
    ${TYPES}/ArrayList.c
    ${UTILS}/Utils.c
)

target_include_directories(${PROJECT_NAME} PUBLIC ${SRC})

# OpenGL required
find_package(OpenGL REQUIRED)

# GLFW required
find_package(glfw3 CONFIG REQUIRED)

# Add required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL glfw)

# glad.c compileable
set_source_files_properties(
    ${SRC}/glad.c
    PROPERTIES
        COMPILE_OPTIONS "-Wno-pedantic;-Wno-error=pedantic"
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR
    ${CMAKE_C_COMPILER_ID} STREQUAL "Clang"
)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wpedantic -march=x86-64-v3 $<$<CONFIG:Release>:-flto> $<$<COMPILE_LANGUAGE:CXX>:-fconstexpr-ops-limit=1000000000>)
    set_source_files_properties(${WINDOW_SRC}/glad.c PROPERTIES COMPILE_OPTIONS -Wno-pedantic)

    if (${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
        target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address> $<$<CONFIG:Debug>:-static-libasan>)
    endif()
endif()

