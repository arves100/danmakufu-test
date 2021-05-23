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
target_include_directories(GcLibDx PUBLIC ${DirectX_INCLUDE_DIR})
target_link_directories(GcLibDx PUBLIC ${DirectX_LIBRARY_PATHS})
target_link_libraries(GcLibDx PUBLIC GcLibStd d3d9 d3dx9 dxguid dinput8 dsound d3dxof msacm32)
