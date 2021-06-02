set(GCLIBDX_SOURCES DirectGraphics.cpp
DirectGraphics.hpp
DirectInput.cpp
DirectInput.hpp
DirectSound.cpp
DirectSound.hpp
DxConstant.hpp
DxLib.hpp
DxScript.cpp
DxScript.hpp
DxText.cpp
DxText.hpp
DxUtility.cpp
DxUtility.hpp
DxWindow.cpp
DxWindow.hpp
ElfreinaMesh.cpp
ElfreinaMesh.hpp
EventScript.cpp
EventScript.hpp
HLSL.hpp
MetasequoiaMesh.cpp
MetasequoiaMesh.hpp
RenderObject.cpp
RenderObject.hpp
ScriptManager.cpp
ScriptManager.hpp
Shader.cpp
Shader.hpp
Texture.cpp
Texture.hpp
TransitionEffect.cpp
TransitionEffect.hpp)

add_library(GcLibDx STATIC ${GCLIBDX_SOURCES})
target_include_directories(GcLibDx PUBLIC ${BGFX_INCLUDE_DIR} ${BIMG_INCLUDE_DIR} ${BX_INCLUDE_DIR})
target_link_libraries(GcLibDx PUBLIC GcLibStd glm::glm)
target_link_libraries(GcLibDx LINK_PUBLIC debug ${BGFX_LIBRARY_DEBUG} ${BIMG_DECODE_LIBRARY_DEBUG} ${BIMG_LIBRARY_DEBUG} ${BX_LIBRARY_DEBUG} LINK_PUBLIC optimized ${BGFX_LIBRARY_RELEASE} ${BIMG_DECODE_LIBRARY_RELEASE} ${BIMG_LIBRARY_RELEASE} ${BX_LIBRARY_RELEASE} )

if (WIN32)
	target_link_libraries(GcLibDx PUBLIC Psapi)
	
	if (MSVC)
		target_include_directories(GcLibDx PUBLIC ${BX_INCLUDE_DIR}/compat/msvc)
	endif()
endif()
