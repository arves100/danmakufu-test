#ifndef __DIRECTX_DXCONSTANT__
#define __DIRECTX_DXCONSTANT__

#include "../gstd/GstdLib.hpp"

//define
#ifdef _WIN32
#define D3D_OVERLOADS
#define DIRECTINPUT_VERSION 0x0800
#define DIRECTSOUND_VERSION 0x0900

#define DWORD_PTR DWORD*
#endif

#if defined(_DEBUG) && defined(_MSC_VER)
#undef new
#endif

//include
#ifdef _WIN32
#include <mmreg.h> //for acm
#include <msacm.h> //for acm

#include <basetsd.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dsound.h>
#include <dxerr.h>
#endif

#include <SDL.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#if defined(_DEBUG) && defined(_MSC_VER)
#include <crtdbg.h>
#include <cstdlib>
#define _CRTDBG_MAP_ALLOC
#define new ::new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#endif
