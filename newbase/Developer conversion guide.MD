# Developer conversion guide

## GcLibStd
TODO: Integrate changes from oldbase lnxr, this part will 100% change. (such as the possible changes?)

- gstd::shared_ptr_ref no longer exists, please use modern C++ pointers (unique_ptr, weak_ptr, shared_ptr)
- Windows messages no longer applies, please switch to SDL_events

## GcLibDx
NOTE: this will mostly be renamed into something else (like GcLibBGFX)

NOTE: DInput will mostly receive a refactor anyway, I'd prefer seeing a virtual key management like Godot/UE4

- All DirectX code is now on top of BGFX, DirectXMath uses GLM now, please switch all your relative custom code to that
- DirectInput now uses SDL2, please switch all relative key converion into SDL2 Keysim
- ShaderData is no longer a class! all the access methods are changed (it hosts Pixel Shader, Vertex Shader and Program)
- ShaderManager no longer restore or release directgraphics (as Shaders no longer have effects)
- Shaders no longer hosts D3DX effects
- Shaders hosts uniformed handlers about their parameters, no reason to keep ShaderParameter anymore (TODO: this may change)
- BX allocator is present in DxUtility.hpp, always use that for bgfx/bimg/bx default allocator
- Texture data is no longer a class! It hosts now the texture width, height, the handle to the texture and the name
- FrameBuffers and Textures ARE NOW DIVIDED. All the Textures that were Framebuffers now needs to create a FrameBuffer class.
- TextureManager now contains distinct API for framebuffers and textures
- RenderObject based classes would now need to provide their own Vertex Shader and Vertex Declaration
- DirectGraphics config now support changing the render, see bgfx supported renders for more information
- DirectGraphics config now supports vertical syncronization
- Shaders do not have a Being and End stage anymore, they only have a submit stage (which acts as the End)
- Debug builds will enable displaying debug data and possibly profiling
- Lights, specular, fog are no longer computed in the CPU, all D3DLIGHT9 code should be converted to shaders
- Alpha test, shading and blend mode texture stages does not work at the moment, they might never do?
- Bgfx does not have the concept of VIEWPORT, it has the concept of View IDs which is different (and Dhn might use more than just one)
- DirectGraphic GetMousePosition is now in the input
- DirectGraphicsPrimaryWindow IS NO LONGER PROVIDED by GcLibDx, you will find it inside the backend implementations
