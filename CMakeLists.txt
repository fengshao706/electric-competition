cmake_minimum_required(VERSION 3.6)
project(electric_competition)

find_package(OpenCV REQUIRED)

include_directories(vision/inc)

message(STATUS "Using OpenCV ${OpenCV_VERSION}")
message(STATUS "OpenCV config path: ${OpenCV_CONFIG_PATH}")
add_executable(${PROJECT_NAME} src/perspective_transform.cpp vision/src/image_processor.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})