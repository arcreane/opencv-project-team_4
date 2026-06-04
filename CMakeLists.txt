cmake_minimum_required(VERSION 3.20)
project(MyImageEditor)

set(CMAKE_CXX_STANDARD 17)

set(OpenCV_DIR "C:/opencv/build")
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(MyImageEditor
    src/main.cpp
    src/thresholding.cpp
    src/creative_effects.cpp
    src/interactive_tools.cpp
)

target_link_libraries(MyImageEditor ${OpenCV_LIBS})