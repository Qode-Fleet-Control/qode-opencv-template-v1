cmake_minimum_required(VERSION 3.16)

project(app VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The layout OpenCV's "Using OpenCV with gcc and CMake" tutorial teaches:
# find_package(OpenCV) + link ${OpenCV_LIBS}. Only the modules used are named.
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

add_executable(app src/main.cpp)
target_include_directories(app PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(app PRIVATE ${OpenCV_LIBS})

install(TARGETS app RUNTIME DESTINATION bin)
