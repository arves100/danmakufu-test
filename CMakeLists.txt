cmake_minimum_required(VERSION 3.16)
project(Danmakufu)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 14)

if (WIN32)
	## Bad hack
	function(pkg_check_modules prefix lib q)
	endfunction()
else()
	include(FindPkgConfig)
endif()

if (WIN32)
	include(FindDirectX)

	if (MSVC)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /permissive-")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /permissive-")
	endif()

	if ("${VCPKG_TARGET_TRIPLET}" MATCHES "-static")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
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
	find_package(SDL2 CONFIG REQUIRED)
	set(SDL2_LIBRARIES SDL2::SDL2-static SDL2::SDL2main)
endif()

pkg_check_modules(ZLIB zlib QUIET)
if (NOT ZLIB_FOUND)
	find_package(ZLIB REQUIRED)
	set(ZLIB_LIBRARIES ZLIB::ZLIB)
endif()

add_subdirectory(source)
