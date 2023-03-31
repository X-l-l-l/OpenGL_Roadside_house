cmake_minimum_required(VERSION 3.24)
project(PGPrj)

set(CMAKE_CXX_STANDARD 23)

# add extra include directories
include_directories("C:/Users/rares/OneDrive/Documents/Facultate/PG/Tools/OpenGL dev libs/include")
include_directories("C:/Users/rares/OneDrive/Documents/Facultate/PG/PGPrj/externals/includes")

# add extra lib directories
link_libraries("C:/Users/rares/OneDrive/Documents/Facultate/PG/Tools/OpenGL dev libs/lib/Release/glfw3.lib")
link_libraries("C:/Users/rares/OneDrive/Documents/Facultate/PG/Tools/OpenGL dev libs/lib/Release/libglew32.lib")
link_libraries("opengl32.lib")
link_libraries("C:/Users/rares/OneDrive/Documents/Facultate/PG/PGPrj/externals/libs/irrKlang.lib")
link_directories("C:/Users/rares/OneDrive/Documents/Facultate/PG/PGPrj/externals/libs")

file(
        GLOB proj
        "*.hpp"
        "*.h"
        "*.cpp"
)

add_executable(PGPrj ${proj} SkyBox.cpp)
