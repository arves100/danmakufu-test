# cmake shader maker
function (make_shader name type path)
	if (NOT DEFINED BGFX_ROOT)
		set(BGFX_ROOT ${BGFX_INCLUDE_DIR}/../)
	endif()
	
	if (WIN32 AND NOT MINGW)
		set(MKS ${CMAKE_SOURCE_DIR}/scripts/mks.bat)
	else()
		set(MKS sh ${CMAKE_SOURCE_DIR}/scripts/mks.sh)
	endif()
	
	execute_process(COMMAND ${MKS} ${BGFX_SHADERC} ${type} ${name} ${CMAKE_SOURCE_DIR}/source/Shaders ${CMAKE_BINARY_DIR}/shaders ${BGFX_ROOT} ${path} ERROR_VARIABLE ERR RESULT_VARIABLE RSS)

	if (NOT ERR STREQUAL "" OR NOT RSS STREQUAL "0")
		message(FATAL_ERROR "Cannot build shader ${name} (type ${type})")
	endif()
endfunction()

# vertex + fragmentation shader
function(make_shader_vf name path)
	make_shader(${name} vs ${path})
	make_shader(${name} fs ${path})
endfunction()

# computation shader
function(make_shader_cs name path)
	make_shader(${name} cs ${path})
endfunction()
