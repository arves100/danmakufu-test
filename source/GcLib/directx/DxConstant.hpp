#ifndef __DIRECTX_DXCONSTANT__
#define __DIRECTX_DXCONSTANT__

#include "../gstd/GstdLib.hpp"

#if MSCV_LEAK_DETECTION
#undef new
#endif

#define GLM_ENABLE_EXPERIMENTAL 1

#include <unordered_map>

#include <SDL.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

// BGFX
#include <bx/bx.h>
#include <bx/timer.h>
#include <bx/os.h>
#include <bx/allocator.h>
#include <bimg/decode.h>
#include <bgfx/bgfx.h>

// GLM (math library)
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/ext/quaternion_transform.hpp>

#if MSCV_LEAK_DETECTION
#include <crtdbg.h>
#include <cstdlib>
#define _CRTDBG_MAP_ALLOC
#define new ::new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

enum class BlendMode
{
	None, //なし
	Alpha, //αで半透明合成
	Add_RGB, //RGBで加算合成
	Add_ARGB, //αで加算合成
	Multiply, //乗算合成
	Subtract, //減算合成
	Shadow, //影描画用
	InvDestRGB, //描画先色反転合成
};

enum class TextureFilterMode
{
	None, //フィルタなし
	Point, //補間なし
	//Linear, //線形補間
	Anisotropic,
};

enum class CullingMode
{
	None,
	Cw,
	Ccw,
};

enum class ShadeMode
{
	Flat,
	Gouraud,
	Phong,
};

enum class DepthMode
{
	None,
	Less,
	LessEqual,
	Equal,
	GreaterEqual,
	NotEqual,
	Never,
	Always_,
};

enum class ColorMode
{
	Bit16,
	Bit32,
};

enum class ScreenMode
{
	Fullscreen,
	DesktopFullscreen,
	Windowed,
};

#endif
