cmake_minimum_required(VERSION 3.10)
project(camera_sender_gst_opencv)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0)

add_executable(camera_sender main.cpp)

target_include_directories(camera_sender PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
)

target_link_libraries(camera_sender PRIVATE
    ${OpenCV_LIBS}
    ${GSTREAMER_LIBRARIES}
)
