cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(YoloV5_NPU)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 17)
  endif ()
endmacro(use_cxx11)

find_package(OpenCV REQUIRED)                              

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -fexceptions")

set(SOURCES src/main.cpp src/postprocess.cpp)

set(EXTRA_LIBS ${OpenCV_LIBS} /usr/local/lib/librknnrt.so)

set(EXECUTABLE_OUTPUT_PATH "../")

include_directories(include ${OpenCV_INCLUDE_DIRS})

add_link_options(-fopenmp -s -ldl -lpthread -pthread -lgomp -rdynamic -march=armv8.2-a)

add_executable(YoloV5_NPU ${SOURCES})

target_link_libraries(YoloV5_NPU ${EXTRA_LIBS})
