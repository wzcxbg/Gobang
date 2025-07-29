cmake_minimum_required(VERSION 3.14)
project(Gobang CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch GLFW
FetchContent_Populate(
        glfw_project
        URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Define GLFW as an imported library
add_library(glfw_lib INTERFACE)
target_include_directories(glfw_lib INTERFACE "${glfw_project_SOURCE_DIR}/include")
target_link_libraries(glfw_lib INTERFACE "${glfw_project_SOURCE_DIR}/lib-vc2022/glfw3.lib")

# Fetch GLAD
FetchContent_Populate(
        glad_project
        URL https://glad.dav1d.de/generated/tmphh169cusglad/glad.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
add_library(glad_src INTERFACE)
target_sources(glad_src INTERFACE ${glad_project_SOURCE_DIR}/src/glad.c)
target_include_directories(glad_src INTERFACE ${glad_project_SOURCE_DIR}/include)


add_executable(Gobang src/main.cpp src/board.cpp src/shader.cpp)
target_link_libraries(Gobang
        PRIVATE
        opengl32
        glfw_lib
        glad_src
)