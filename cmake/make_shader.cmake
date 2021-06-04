# cmake shader maker
function (make_shader name type)
	if (NOT DEFINED BGFX_ROOT)
		set(BGFX_ROOT ${BGFX_INCLUDE_DIR}/../)
	endif()
	
	if (WIN32 AND NOT MINGW)
		set(MKS ${CMAKE_SOURCE_DIR}/scripts/mks.bat)
	else()
		set(MKS sh ${CMAKE_SOURCE_DIR}/scripts/mks.sh)
	endif()
	
	execute_process(COMMAND ${MKS} ${BGFX_SHADERC} ${type} ${name} ${CMAKE_SOURCE_DIR}/source/Shaders ${CMAKE_BINARY_DIR}/shaders ${BGFX_ROOT} ERROR_VARIABLE ERR RESULT_VARIABLE RSS)
	
	if (DEFINED ERR OR NOT RSS STREQUAL "0")
		message(FATAL_ERROR "Cannot build shader ${name} (type ${type})")
	endif()
	
endfunction()
