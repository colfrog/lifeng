cmake_minimum_required(VERSION 3.16)
project(objectview C)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_library(engine SHARED engine.c)
add_executable(sdl2 sdl2.c)
target_link_libraries(sdl2 engine ${SDL2_LIBRARIES} pthread)

find_package(OpenCL)
if(OpenCL_FOUND)
	add_library(engineOCL SHARED engine-ocl.c)
	add_executable(OCL_sdl2 sdl2.c)
	target_link_libraries(OCL_sdl2 engineOCL ${SDL2_LIBRARIES} pthread OpenCL::OpenCL)
	configure_file(grow.cl grow.cl COPYONLY)
endif()
