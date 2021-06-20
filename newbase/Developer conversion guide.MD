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
- BX allocator is present in DxUtility.hpp, always use that for bgfx/bimg/bx default allocator
- Texture data is no longer a class! It hosts now the texture width, height, the handle to the texture and the name
- FrameBuffers and Textures ARE NOW DIVIDED. All the Textures that were Framebuffers now needs to create a FrameBuffer class.
- TextureManager now contains distinct API for framebuffers and textures
- DirectGraphics config now support changing the render, see bgfx supported renders for more information
- DirectGraphics config now supports vertical syncronization
- Debug builds will enable displaying debug data and possibly profiling
- Lights, specular, fog are no longer computed in the CPU, all D3DLIGHT9 code should be converted to shaders
- Alpha test, shading and blend mode texture stages does not work at the moment, they might never do?
- Bgfx does not have the concept of VIEWPORT, it has the concept of View IDs which is different (and Dhn might use more than just one)
- DirectGraphic GetMousePosition is now in the input
- DirectGraphicsPrimaryWindow IS NO LONGER PROVIDED by GcLibDx, you will find it inside the backend implementations
- RenderObjects are completely revamped, please see its specific section for more information
- Shaders are completely revampred, please see its specific section for more information
- SetProjectionMatrix can now work with fov (default 45.0f like original dhn)
- Changed return time of some classes to pointers (while using unique_ptr, NEVER FREE those classes)
- UpdateState is now PUBLIC, you have to MANULLY update the state before submitting meshes
- SetZBufferEnable -> SetDepthTest
- Viewport concepts are now removed, please use multiple view instead

### RenderObjects
Vertex declaractions (ex FVF) uses BGFX vertexlayout now, please switch to the new code.

All Vertex declarations MUST IMPLEMENT a get function otherwise they WILL NOT WORK. (See VERTEX_LTA for example)

Removed 99% of duplicated code in get/set inside the multiple RenderObject classes.

The RenderObject now requires a TEMPLATE PARAMETER which is the vertex declaration, all the
relative set/get of vertex are now handled inside renderobject by itself.

A custom class based must inheratate a RenderObject WITH a vertex declaration as a template, then
it MUST include one or more RO_IMPL functions.

The RO_IMPL functions are simple defines that replaces all the duplicated SetUV, SetVertexPos and so on,
just add the defines after the public definition to expose such functions, see RenderObjectLTA for an example.

The vertex declaration is automatically computed, you no longer need to call the creation.

`_ReleaseVertexBuffer` and `_RestoreVertexBuffer` are now called `_ReleaseBuffers` and `_RestoreBuffers`

`BeginShader` and `EndShader`, as expected by shaders, do not exist anymore.

The `Render` function is now called `Submit`.

A render object now **ALWAYS** require a Shader (at least Vertex shader) to work!

`_ReleaseBuffers`, `_RestoreBuffers`, `_CreateVertexDeclaration` are not longer virtual functions, no reason to
replace them with a custom implementation.

As RenderObject is now a template, all it's implementation is done entirely in the header file.

All counts and index of vectors now uses size_t and not signed intengers

SetTexture now requires an unique pointer of a texture to move inside the texture array. (No longer a shader pointers)

Almost all returns of shader pointers are now replaced with reference return.

`strideVertexStreamZero_` is removed, please use `pVertexDecl.getStride()` instead.

The RenderObject now has a `bRecreate_` function that will instruct the submit process to recreate the
VB and IB before they are submitted, this is done to avoid creating VBOs at runtime.

`SetVertexIndicies` is now called `SetIndices`.

Added new functions for Indices/Index Buffer.

Modified the type of the vertex buffer, it is now a simple std::vector with the template.

`SetRalativeMatrix` is now called `SetRalativeMatrix`.

You can have to load shaders from RenderObject, get/set doesn't exist anymore.

shader_ is now unique pointer!

bgfx does not support direct drawing of different primitives, each primitive is converted to tristrip, API still works but you have to convert the names into bgfx ones.

## Shaders/ShaderManager/ShaderData

ShaderData is no longer a class! all the access methods are changed (it hosts Pixel Shader, Vertex Shader and Program)

ShaderManager no longer restore or release directgraphics (as Shaders no longer have effects)

ShaderManager NO LONGER HOLDS points to a shader, as they are subjectable to changes due to their parameter/uniform handlers.

ShaderManager is only able to create ShaderDatas now.

Shaders no longer hosts D3DX effects

Shaders hosts uniformed handlers about their parameters, no reason to keep ShaderParameter anymore (TODO: this may change)

Shaders do not have a Being and End stage anymore, they only have a submit stage (which acts as the End)

Removed Add,Get,Remove of shaders.

Removed ReleaseDxResource, RestoreDxResource in ShaderManager

All the old set of shaders does not exist anymore, you now have to pass a special "shader interface" in order
to perform uniform handle creation and destruction.

ShaderParameter uses bgfx UniformHandler types

TextureParameter now exists! Use that for passing textures

CreateFromText currently doesn't exist anymore, this may change later...

You can add a new shader by placing it inside the source/Shaders folder, keep in mind you need to follow bgfx shader rules.

Specify shadername.def and shadername.vs or shadername.fs or shadername.cs (based from the type of shaders you need)
