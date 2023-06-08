cmake_minimum_required(VERSION 2.8)
project(Packet_example)

add_definitions(-w)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find Eigen
find_package(Eigen3 REQUIRED)

# Add OpenCV's includes
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${OpenCV_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
)

add_library(${PROJECT_NAME} SHARED
  src/Packet.cc
)

# Link OpenCV's libraries to our library
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# Build examples
add_executable(publisher src/Publisher.cc)
target_link_libraries(publisher ${PROJECT_NAME} pthread)
add_executable(subscriber src/Subscriber.cc)
target_link_libraries(subscriber ${PROJECT_NAME} pthread)