#ifndef __TOUHOUDANMAKUFU_EXE_GCLIBIMPL__
#define __TOUHOUDANMAKUFU_EXE_GCLIBIMPL__

#include "../Common/DnhGcLibImpl.hpp"
#include "Constant.hpp"

/**********************************************************
//EApplication
**********************************************************/
class EApplication : public Singleton<EApplication>, public Application {
	friend Singleton<EApplication>;

private:
	EApplication();

protected:
	bool _Initialize();
	bool _Loop();
	bool _Finalize();

public:
	~EApplication();
};

/**********************************************************
//EDirectGraphics
**********************************************************/
class EDirectGraphics : public Singleton<EDirectGraphics>, public DirectGraphicsPrimaryWindow {
	friend Singleton<EDirectGraphics>;

private:
	EDirectGraphics();

protected:
	virtual void EventProcedure(SDL_Event* evt);

public:
	~EDirectGraphics();
	virtual bool Initialize();
	void SetRenderStateFor2D(int blend);
};

#endif
