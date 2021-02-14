#include "Window.hpp"

using namespace gstd;

/**********************************************************
//WindowBase
**********************************************************/
WindowBase::WindowBase()
{
	hWnd_ = nullptr;
	bShutdown_ = false;

	// SDL2 does have a window id for us
}

WindowBase::~WindowBase()
{
	if (hWnd_)
	{
		SDL_DestroyWindow(hWnd_);
		hWnd_ = nullptr;
	}
}

void WindowBase::MoveWindowCenter()
{
	SDL_SetWindowPosition(hWnd_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

/**********************************************************
//WindowUtility
**********************************************************/
void WindowUtility::SetMouseVisible(bool bVisible)
{
	if (bVisible) {
		SDL_ShowCursor(SDL_ENABLE);
	} else {
		SDL_ShowCursor(SDL_DISABLE);
	}
}
