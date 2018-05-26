cmake_minimum_required(VERSION 3.0.0)
project(boxxy VERSION 0.1.0)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include(CTest)
enable_testing()

add_executable(boxxy main.cpp)

set_property(TARGET boxxy PROPERTY CXX_STANDARD_REQUIRED 17)
set_property(TARGET boxxy PROPERTY CXX_STANDARD 17)

target_link_libraries(boxxy SDL2 glew)


add_subdirectory(deps)