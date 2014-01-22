# config
cmake_minimum_required(VERSION 1.0)


SET(APP_NAME EpollExample)
PROJECT(${APP_NAME})
 
# working directory
SET(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-std=c99 -D_GNU_SOURCE=500")
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

# source
AUX_SOURCE_DIRECTORY(. src)
 
# target
ADD_EXECUTABLE(${APP_NAME}  ${src})

target_link_libraries(${APP_NAME}
  pthread
  )