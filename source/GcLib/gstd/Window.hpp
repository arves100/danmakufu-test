#ifndef __GSTD_WINDOW__
#define __GSTD_WINDOW__

#include "GstdConstant.hpp"
#include "GstdUtility.hpp"
#include "Thread.hpp"

namespace gstd {

/**********************************************************
//WindowBase
**********************************************************/
class WindowBase {
public:
	WindowBase();
	virtual ~WindowBase();
	SDL_Window* GetWindowHandle() { return hWnd_; }

	Uint32 GetWindowId() { return SDL_GetWindowID(hWnd_); }

	virtual void SetBounds(int x, int y, int width, int height)
	{
		SDL_SetWindowSize(hWnd_, width, height);
		SDL_SetWindowPosition(hWnd_, x, y);
	}

	int GetClientX()
	{
		int x, y;
		SDL_GetWindowPosition(hWnd_, &x, &y);
		return x;
	}
	int GetClientY()
	{
		int x, y;
		SDL_GetWindowPosition(hWnd_, &x, &y);
		return y;
	}
	int GetClientWidth()
	{
		int w, h;
		SDL_GetWindowSize(hWnd_, &w, &h);
		return w;
	}
	int GetClientHeight()
	{
		int w, h;
		SDL_GetWindowSize(hWnd_, &w, &h);
		return h;
	}

	void SetWindowVisible(bool bVisible)
	{
		if (bVisible)
			SDL_ShowWindow(hWnd_);
		else
			SDL_HideWindow(hWnd_);
	}

	bool IsWindowVisible() { return (SDL_GetWindowFlags(hWnd_) & SDL_WINDOW_HIDDEN) ? false : true; }

	Uint32 GetWindowFlags() { return SDL_GetWindowFlags(hWnd_); }

	virtual void LocateParts() {} //画面部品配置
	void MoveWindowCenter();

	bool ShouldShutdown() { return bShutdown_; }

protected:
	bool bShutdown_;
	SDL_Window* hWnd_;
	virtual void EventProcedure(SDL_Event* evt) {} //オーバーライド用プロシージャ
};

/**********************************************************
//WindowUtility
**********************************************************/
class WindowUtility {
public:
	static void SetMouseVisible(bool bVisible);
};

} // namespace gstd

#endif
