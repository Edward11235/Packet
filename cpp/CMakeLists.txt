cmake_minimum_required(VERSION 2.8)
project(Packet)

add_definitions(-w)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

# Add OpenCV's includes
include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

add_library(${PROJECT_NAME} SHARED
  src/Packet.cc
)

# Build examples
# add_executable(publisher src/Publisher.cc)
# target_link_libraries(publisher ${PROJECT_NAME} pthread)
