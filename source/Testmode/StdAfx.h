#pragma once

#include "../GcLib/directx/DxConstant.hpp"
#include "../GcLib/directx/DirectGraphics.hpp"
#include "../GcLib/directx/DirectInput.hpp"
#include "../GcLib/directx/Texture.hpp"
#include "../GcLib/directx/DxUtility.hpp"
#include "../GcLib/directx/Shader.hpp"
#include "../GcLib/directx/RenderObject.hpp"

#include <SDL2/SDL_syswm.h>
#include <Version.h>

#include "TestApp.hpp"

#define DEFINE_MAIN(appName) int main(int, char**) \
{ \
	appName app; \
	if (!app.Initialize())  \
		return EXIT_FAILURE; \
	app.Loop(); \
	return EXIT_SUCCESS; \
}
