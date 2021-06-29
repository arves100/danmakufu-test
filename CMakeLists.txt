cmake_minimum_required(VERSION 3.10)
project(Danmakufu LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 14)

if (WIN32)
	## Bad hack
	function(pkg_check_modules prefix lib q)
	endfunction()
else()
	include(FindPkgConfig)
endif()

if (WIN32 OR MINGW)
	if (MSVC)
		include(FindDirectX) # mingw already includes dxsdk

		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /permissive- /MP")

		if ("${VCPKG_TARGET_TRIPLET}" MATCHES "-static")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
		endif()
	else()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -municode -mwindows")
	endif()
	
	if (MINGW)
		set(CMAKE_EXE_LINKER ${CMAKE_CXX_COMPILER})
	endif()
	
	add_definitions(-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0601) # Default target Win7
else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
endif()

pkg_check_modules(VORBIS vorbis QUIET)
if (NOT VORBIS_FOUND)
	find_package(Vorbis CONFIG REQUIRED)
	set(VORBIS_LIBRARIES Vorbis::vorbis Vorbis::vorbisenc Vorbis::vorbisfile)
	set(VORBIS_INCLUDE_DIRS ${VORBIS_INCLUDE_DIR})
endif()

pkg_check_modules(SDL2 sdl2 QUIET)
if (NOT SDL2_FOUND)
	find_package(SDL2 CONFIG) # Thank you sdl2-cmake.cmake for NOT SETTING SDL2_FOUND, forcing me to NOT use REQUIRED.
	
	if (DEFINED VCPKG_TARGET_TRIPLET)
		set(SDL2_LIBRARIES SDL2::SDL2-static)
	else()
		set(SDL2_LIBRARIES SDL2)
	endif()
endif()

pkg_check_modules(ZLIB zlib QUIET)
if (NOT ZLIB_FOUND)
	find_package(ZLIB REQUIRED)
	set(ZLIB_LIBRARIES ZLIB::ZLIB)
endif()

pkg_check_modules(GLM glm QUIET)
if (NOT GLM_FOUND)
	find_package(glm CONFIG REQUIRED)
	
	if (DEFINED VCPKG_TARGET_TRIPLET)
		set(GLM_LIBRARIES glm::glm)
	else()
		set(GLM_LIBRARIES glm)
	endif()
endif()

include(FindBgfx)

execute_process(COMMAND git branch --show-current WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE __BRANCH__ OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git rev-parse HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE __COMMIT__ OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git tag --points-at HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE TAG_DIRTY OUTPUT_STRIP_TRAILING_WHITESPACE)

if (NOT TAG_DIRTY STREQUAL "")
	string(REPLACE "^0" "" __TAG__ ${TAG_DIRTY})
endif()

configure_file(source/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

add_subdirectory(source)
