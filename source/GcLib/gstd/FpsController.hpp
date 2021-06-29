#ifndef __GSTD_FPSCONTROLLER__
#define __GSTD_FPSCONTROLLER__

#include "GstdConstant.hpp"


namespace gstd {

class FpsControlObject;
/**********************************************************
//FpsController
**********************************************************/
class FpsController {
public:
	enum {
		FPS_FAST_MODE = 1200,
	};

public:
	FpsController();
	virtual ~FpsController();
	virtual void SetFps(int fps) { fps_ = fps; }
	virtual int GetFps() { return fps_; }
	virtual void SetTimerEnable(bool b) { bUseTimer_ = b; }

	virtual void Wait() = 0;
	virtual bool IsSkip() { return false; }
	virtual void SetCriticalFrame() { bCriticalFrame_ = true; }
	virtual float GetCurrentFps() = 0;
	virtual float GetCurrentWorkFps() { return GetCurrentFps(); }
	virtual float GetCurrentRenderFps() { return GetCurrentFps(); }
	bool IsFastMode() { return bFastMode_; }
	void SetFastMode(bool b) { bFastMode_ = b; }

	void AddFpsControlObject(std::weak_ptr<FpsControlObject> obj);
	void RemoveFpsControlObject(std::weak_ptr<FpsControlObject> obj);
	int GetControlObjectFps();

protected:
	int fps_; //設定されているFPS
	bool bUseTimer_; //タイマー制御
	bool bCriticalFrame_;
	bool bFastMode_;

	std::list<std::weak_ptr<FpsControlObject>> listFpsControlObject_;

	inline Uint64 _GetTime();
	inline void _Sleep(int msec);
};

/**********************************************************
//StaticFpsController
**********************************************************/
class StaticFpsController : public FpsController {
public:
	StaticFpsController();
	~StaticFpsController();

	virtual void Wait();
	virtual bool IsSkip();
	virtual void SetCriticalFrame()
	{
		bCriticalFrame_ = true;
		timeError_ = 0;
		countSkip_ = 0;
	}

	void SetSkipRate(int value);
	virtual float GetCurrentFps();
	virtual float GetCurrentWorkFps();
	virtual float GetCurrentRenderFps();

protected:
	float fpsCurrent_; //現在のFPS
	Uint64 timePrevious_; //前回Waitしたときの時間
	Uint64 timeError_; //持ち越し時間(誤差)
	Uint64 timeCurrentFpsUpdate_; //1秒を測定するための時間保持
	int rateSkip_; //描画スキップ数
	int countSkip_; //描画スキップカウント
	std::list<Uint64> listFps_; //1秒ごとに現在fpsを計算するためにfpsを保持

private:
	enum {
		FAST_MODE_SKIP_RATE = 10,
	};
};

/**********************************************************
//AutoSkipFpsController
**********************************************************/
class AutoSkipFpsController : public FpsController {
public:
	AutoSkipFpsController();
	~AutoSkipFpsController();

	virtual void Wait();
	virtual bool IsSkip();
	virtual void SetCriticalFrame()
	{
		bCriticalFrame_ = true;
		timeError_ = 0;
		countSkip_ = 0;
	}

	virtual float GetCurrentFps() { return GetCurrentWorkFps(); }
	float GetCurrentWorkFps() { return fpsCurrentWork_; };
	float GetCurrentRenderFps() { return fpsCurrentRender_; };

protected:
	float fpsCurrentWork_; //実際のfps
	float fpsCurrentRender_; //実際のfps
	Uint64 timePrevious_; //前回Waitしたときの時間
	Uint64 timePreviousWork_;
	Uint64 timePreviousRender_;
	Uint64 timeError_; //持ち越し時間(誤差)
	Uint64 timeCurrentFpsUpdate_; //1秒を測定するための時間保持
	std::list<Uint64> listFpsWork_;
	std::list<Uint64> listFpsRender_;
	double countSkip_; //連続描画スキップ数
	int countSkipMax_; //最大連続描画スキップ数
};

/**********************************************************
//FpsControlObject
**********************************************************/
class FpsControlObject {
public:
	FpsControlObject() {}
	virtual ~FpsControlObject() {}
	virtual int GetFps() = 0;
};

} // namespace gstd

#endif
