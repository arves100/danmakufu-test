# Find BGFX

if (NOT DEFINED BGFX_ROOT OR NOT DEFINED BGFX_BUILD_NAME) # manual find
	if (NOT DEFINED BGFX_INCLUDE_DIR OR NOT DEFINED BGFX_LIBRARY_DEBUG OR NOT DEFINED BGFX_LIBRARY_RELEASE)
		message(FATAL_ERROR "Please set BGFX_INCLUDE_DIR, BGFX_LIBRARY_DEBUG and BGFX_LIBRARY_RELEASE")
	else()
		message(STATUS "Found BGFX ${BGFX_INCLUDE_DIR} (debug: ${BGFX_LIBRARY_DEBUG} optimized: ${BGFX_LIBRARY_RELEASE})")
	endif()

	# Find BIMG
	if (NOT DEFINED BIMG_INCLUDE_DIR OR NOT DEFINED BIMG_LIBRARY_DEBUG OR NOT DEFINED BIMG_LIBRARY_RELEASE)
		message(FATAL_ERROR "Please set BIMG_INCLUDE_DIR, BIMG_LIBRARY_DEBUG and BIMG_LIBRARY_RELEASE")
	else()
		message(STATUS "Found BIMG ${BIMG_INCLUDE_DIR} (debug: ${BIMG_LIBRARY_DEBUG} optimized: ${BIMG_LIBRARY_RELEASE})")
	endif()

	if (NOT DEFINED BIMG_DECODE_LIBRARY_DEBUG OR NOT DEFINED BIMG_DECODE_LIBRARY_RELEASE)
		message(FATAL_ERROR "Please set BIMG_DECODE_LIBRARY_DEBUG and BIMG_DECODE_LIBRARY_RELEASE")
	else()
		message(STATUS "Found BIMG Decode (debug: ${BIMG_DECODE_LIBRARY_DEBUG} optimized: ${BIMG_DECODE_LIBRARY_RELEASE})")
	endif()

	# Find BX
	if (NOT DEFINED BX_INCLUDE_DIR OR NOT DEFINED BX_LIBRARY_DEBUG OR NOT DEFINED BX_LIBRARY_RELEASE)
		message(FATAL_ERROR "Please set BGFX_INCLUDE_DIR, BGFX_LIBRARY_DEBUG and BX_LIBRARY_RELEASE")
	else()
		message(STATUS "Found BX ${BX_INCLUDE_DIR} (debug: ${BX_LIBRARY_DEBUG} optimized: ${BX_LIBRARY_RELEASE})")
	endif()

	# Find bgfx shaderc
	if (NOT DEFINED BGFX_SHADERC)
		message(FATAL_ERROR "Please set BGFX_SHADERC")
	else()
		message(STATUS "Shader compiler ${BGFX_SHADERC}")
	endif()
else() # auto find
	set(BGFX_INCLUDE_DIR ${BGFX_ROOT}/include)
	set(BIMG_INCLUDE_DIR ${BGFX_ROOT}/../bimg/include)
	set(BX_INCLUDE_DIR ${BGFX_ROOT}/../bx/include)

	if (NOT WIN32)
		set(BGFX_LIBRARY_DEBUG ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/libbgfxDebug.a)
		set(BGFX_LIBRARY_RELEASE ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/libbgfxRelease.a)
		set(BIMG_LIBRARY_DEBUG ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/libbimgDebug.a)
		set(BIMG_LIBRARY_RELEASE ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/libbimgRelease.a)
		set(BIMG_DECODE_LIBRARY_DEBUG ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/libbimg_decodeDebug.a)
		set(BIMG_DECODE_LIBRARY_RELEASE ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/libbimg_decodeRelease.a)
		set(BX_LIBRARY_DEBUG ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/libbxDebug.a)
		set(BX_LIBRARY_RELEASE ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/libbxRelease.a)
		set(BGFX_SHADERC ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/shadercRelease)
	else()
		set(BGFX_LIBRARY_DEBUG ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/bgfxDebug.lib)
		set(BGFX_LIBRARY_RELEASE ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/bgfxRelease.lib)
		set(BIMG_LIBRARY_DEBUG ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/bimgDebug.lib)
		set(BIMG_LIBRARY_RELEASE ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/bimgRelease.lib)
		set(BIMG_DECODE_LIBRARY_DEBUG ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/bimg_decodeDebug.lib)
		set(BIMG_DECODE_LIBRARY_RELEASE ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/bimg_decodeRelease.lib)
		set(BX_LIBRARY_DEBUG ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/bxDebug.lib)
		set(BX_LIBRARY_RELEASE ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/bxRelease.lib)
		set(BGFX_SHADERC ${BGFX_ROOT}/.build/${BGFX_BUILD_NAME}/bin/shadercRelease.exe)
	endif()
	
	message(STATUS "Found BGFX ${BGFX_INCLUDE_DIR} (debug: ${BGFX_LIBRARY_DEBUG} optimized: ${BGFX_LIBRARY_RELEASE})")
	message(STATUS "Found BIMG ${BIMG_INCLUDE_DIR} (debug: ${BIMG_LIBRARY_DEBUG} optimized: ${BIMG_LIBRARY_RELEASE})")
	message(STATUS "Found BIMG Decode (debug: ${BIMG_DECODE_LIBRARY_DEBUG} optimized: ${BIMG_DECODE_LIBRARY_RELEASE})")
	message(STATUS "Found BX ${BX_INCLUDE_DIR} (debug: ${BX_LIBRARY_DEBUG} optimized: ${BX_LIBRARY_RELEASE})")
	message(STATUS "Shader compiler ${BGFX_SHADERC}")
endif()
