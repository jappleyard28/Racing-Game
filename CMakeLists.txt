cmake_minimum_required(VERSION 3.17)
project(racing_game)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc -Wno-c++98-compat")
	endif()
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(lib/glfw-3.3.3)

add_executable(racing_game src/main.cpp src/glw.h src/glw.cpp src/assets.h src/assets.cpp src/window.h src/window.cpp src/render.h src/render.cpp src/engine.h src/engine.cpp src/game.h src/game.cpp src/input.h src/input.cpp src/util.h src/util.cpp src/config.h)

target_link_libraries(racing_game glfw)
target_include_directories(racing_game PUBLIC lib/glad/include lib/glm-0.9.9.8/glm lib/stb)
