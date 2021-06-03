#ifndef __DIRECTX_DXCONSTANT__
#define __DIRECTX_DXCONSTANT__

#include "../gstd/GstdLib.hpp"

#ifdef _DEBUG
#undef new
#endif

//include

#include <mmreg.h> //for acm
#include <msacm.h> //for acm

#include <basetsd.h>
#include <dsound.h>
#include <SDL.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

// TODO: DEFINE NOMINMAX AND GET RID OF THIS ON WINDOWS
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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

#ifdef _DEBUG
#include <crtdbg.h>
#include <cstdlib>
#define _CRTDBG_MAP_ALLOC
#define new ::new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#endif
