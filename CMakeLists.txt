cmake_minimum_required(VERSION 3.5)

project(SoundBird LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

file(GLOB SRC "src/*")

add_subdirectory(external/portaudio)
add_subdirectory(external/GLFW)
add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} PortAudio glfw OpenGL::GL)
target_include_directories(${PROJECT_NAME} PUBLIC external/stb/include/)

include(GNUInstallDirs)

configure_file(${CMAKE_SOURCE_DIR}/assets/sprites.don ${CMAKE_BINARY_DIR}/SoundBird/assets/sprites.don COPYONLY)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/SoundBird/bin")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
