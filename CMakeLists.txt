# config
cmake_minimum_required(VERSION 1.0)


SET(APP_NAME EpollExample)
PROJECT(${APP_NAME})
 
# working directory
SET(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-std=c99 -D_GNU_SOURCE=500")
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

include_directories(
	${PROJECT_SOURCE_DIR}/system
	${PROJECT_SOURCE_DIR}/tasks
	${PROJECT_SOURCE_DIR}/pool
	${PROJECT_SOURCE_DIR}/common
)

# source
AUX_SOURCE_DIRECTORY(. src)
AUX_SOURCE_DIRECTORY(./pool pool_src)
AUX_SOURCE_DIRECTORY(./tasks tasks_src)
AUX_SOURCE_DIRECTORY(./system system_src)
 
# target
ADD_EXECUTABLE(${APP_NAME}  ${src} ${tasks_src} ${pool_src} ${system_src})

target_link_libraries(${APP_NAME}
  pthread
  )