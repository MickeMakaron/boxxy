cmake_minimum_required(VERSION 3.5)

execute_process(COMMAND git submodule update --init --recursive)

# sdl2 #########################
add_subdirectory(sdl2)
target_include_directories(SDL2main PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sdl2/include>)
target_include_directories(SDL2 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sdl2/include>)
################################