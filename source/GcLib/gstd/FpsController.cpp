#include "FpsController.hpp"

using namespace gstd;

/**********************************************************
//FpsController
**********************************************************/
FpsController::FpsController()
{
	// SDL2 DOES USE timeGetPeriod(1) making the API call here useless

	fps_ = 60;
	bUseTimer_ = true;
	bCriticalFrame_ = true;
	bFastMode_ = false;
}
FpsController::~FpsController()
{
}
Uint64 FpsController::_GetTime()
{
	return (SDL_GetPerformanceCounter() * 1000) / SDL_GetPerformanceFrequency();
}
void FpsController::_Sleep(int msec)
{
	::Sleep(msec);
}
void FpsController::AddFpsControlObject(std::weak_ptr<FpsControlObject> obj)
{
	listFpsControlObject_.push_back(obj);
}
void FpsController::RemoveFpsControlObject(std::weak_ptr<FpsControlObject> obj)
{
	std::list<std::weak_ptr<FpsControlObject>>::iterator itr = listFpsControlObject_.begin();
	;
	for (; itr != listFpsControlObject_.end(); itr++) {
		std::weak_ptr<FpsControlObject> tObj = (*itr);
		if (obj.lock() == tObj.lock()) {
			listFpsControlObject_.erase(itr);
			break;
		}
	}
}
int FpsController::GetControlObjectFps()
{
	int res = fps_;
	std::list<std::weak_ptr<FpsControlObject>>::iterator itr = listFpsControlObject_.begin();
	;
	for (; itr != listFpsControlObject_.end();) {
		std::weak_ptr<FpsControlObject> obj = (*itr);
		if (!obj.expired()) {
			int fps = obj.lock()->GetFps();
			res = MIN(res, fps);
			itr++;
		} else
			itr = listFpsControlObject_.erase(itr);
	}
	return res;
}

/**********************************************************
//StaticFpsController
**********************************************************/
StaticFpsController::StaticFpsController()
{
	rateSkip_ = 0;
	fpsCurrent_ = 60;
	timePrevious_ = _GetTime();
	timeCurrentFpsUpdate_ = 0;
	bUseTimer_ = true;
	timeError_ = 0;
	countSkip_ = 0;
}
StaticFpsController::~StaticFpsController()
{
}
void StaticFpsController::Wait()
{
	auto time = _GetTime();

	double tFps = fps_;
	tFps = MIN(tFps, static_cast<double>(GetControlObjectFps()));
	if (bFastMode_)
		tFps = FPS_FAST_MODE;

	auto sTime = time - timePrevious_; //前フレームとの時間差

	Uint64 frameAs1Sec = sTime * tFps;
	auto time1Sec = 1000 + timeError_;
	int sleepTime = 0;
	timeError_ = 0;
	if (frameAs1Sec < time1Sec) {
		sleepTime = static_cast<int>((time1Sec - frameAs1Sec) / tFps); //待機時間
		if (sleepTime < 0)
			sleepTime = 0;
		if (bUseTimer_ || rateSkip_ != 0) {
			_Sleep(sleepTime); //一定時間たつまで、sleep
			timeError_ = (time1Sec - frameAs1Sec) % (int)tFps;
		}

		if (timeError_ < 0)
			timeError_ = 0;
	}

	//1frameにかかった時間を保存
	double timeCorrect = static_cast<double>(sleepTime);
	if (time - timePrevious_ > 0)
		listFps_.push_back(time - timePrevious_ + ceil(timeCorrect));
	timePrevious_ = _GetTime();

	if (time - timeCurrentFpsUpdate_ >= 1000) { //一秒ごとに表示フレーム数を更新
		if (listFps_.size() != 0) {
			double tFpsCurrent = 0;
			auto itr = listFps_.begin(), end = listFps_.end();
			for (; itr != end; itr++) {
				tFpsCurrent += (*itr);
			}
			fpsCurrent_ = 1000.0 / (tFpsCurrent / static_cast<double>(listFps_.size()));
			listFps_.clear();
		} else
			fpsCurrent_ = 0;

		timeCurrentFpsUpdate_ = _GetTime();
	}
	countSkip_++;

	int rateSkip = rateSkip_;
	if (bFastMode_)
		rateSkip = FAST_MODE_SKIP_RATE;
	countSkip_ %= (rateSkip + 1);
	bCriticalFrame_ = false;
}
bool StaticFpsController::IsSkip()
{
	int rateSkip = rateSkip_;
	if (bFastMode_)
		rateSkip = FAST_MODE_SKIP_RATE;
	if (rateSkip == 0 || bCriticalFrame_)
		return false;
	if (countSkip_ % (rateSkip + 1) != 0)
		return true;
	return false;
}
void StaticFpsController::SetSkipRate(int value)
{
	rateSkip_ = value;
	countSkip_ = 0;
}
float StaticFpsController::GetCurrentFps()
{
	float fps = fpsCurrent_ / (rateSkip_ + 1);
	return fps;
}
float StaticFpsController::GetCurrentWorkFps()
{
	return fpsCurrent_;
}
float StaticFpsController::GetCurrentRenderFps()
{
	float fps = fpsCurrent_ / (rateSkip_ + 1);
	return fps;
}

/**********************************************************
//AutoSkipFpsController
**********************************************************/
AutoSkipFpsController::AutoSkipFpsController()
{
	timeError_ = 0;
	timePrevious_ = _GetTime();
	timePreviousWork_ = timePrevious_;
	timePreviousRender_ = timePrevious_;
	countSkip_ = 0;
	countSkipMax_ = 20;

	fpsCurrentWork_ = 0;
	fpsCurrentRender_ = 0;
	timeCurrentFpsUpdate_ = 0;
	timeError_ = 0;
}
AutoSkipFpsController::~AutoSkipFpsController()
{
}
void AutoSkipFpsController::Wait()
{
	auto time = _GetTime();

	double tFps = fps_;
	tFps = MIN(tFps, static_cast<double>(GetControlObjectFps()));
	if (bFastMode_)
		tFps = FPS_FAST_MODE;

	auto sTime = time - timePrevious_; //前フレームとの時間差
	Uint64 frameAs1Sec = sTime * tFps;
	auto time1Sec = 1000 + timeError_;
	int sleepTime = 0;
	timeError_ = 0;
	if (frameAs1Sec < time1Sec || bCriticalFrame_) {
		sleepTime = (time1Sec - frameAs1Sec) / tFps; //待機時間
		if (sleepTime < 0 || countSkip_ - 1 >= 0)
			sleepTime = 0;
		if (bUseTimer_)
			_Sleep(sleepTime); //一定時間たつまで、sleep

		timeError_ = (time1Sec - frameAs1Sec) % (int)tFps;
		// if (timeError_< 0 )
		// 	timeError_ = 0;
	} else if (countSkip_ <= 0) {
		countSkip_ += (double)sTime * tFps / 1000 + 1;
		if (countSkip_ > countSkipMax_)
			countSkip_ = countSkipMax_;
	}

	countSkip_--;
	bCriticalFrame_ = false;

	{
		//1Workにかかった時間を保存
		double timeCorrect = (double)sleepTime;
		if (time - timePrevious_ > 0)
			listFpsWork_.push_back(time - timePrevious_ + ceil(timeCorrect));
		timePrevious_ = _GetTime();
		;
	}
	if (countSkip_ <= 0) {
		//1描画にかかった時間を保存
		time = _GetTime();
		if (time - timePreviousRender_ > 0)
			listFpsRender_.push_back(time - timePreviousRender_);
		timePreviousRender_ = _GetTime();
	}

	timePrevious_ = _GetTime();
	if (time - timeCurrentFpsUpdate_ >= 1000) { //一秒ごとに表示フレーム数を更新
		if (listFpsWork_.size() != 0) {
			float tFpsCurrent = 0;
			auto itr = listFpsWork_.begin(), end = listFpsWork_.end();
			for (; itr != end; itr++) {
				tFpsCurrent += (*itr);
			}
			fpsCurrentWork_ = 1000.0f / (tFpsCurrent / (float)listFpsWork_.size());
			listFpsWork_.clear();
		} else
			fpsCurrentWork_ = 0;

		if (listFpsRender_.size() != 0) {
			float tFpsCurrent = 0;
			auto itr = listFpsRender_.begin(), end = listFpsRender_.end();
			for (; itr != end; itr++) {
				tFpsCurrent += (*itr);
			}
			fpsCurrentRender_ = 1000.0f / (tFpsCurrent / (float)listFpsRender_.size());
			listFpsRender_.clear();
		} else
			fpsCurrentRender_ = 0;

		timeCurrentFpsUpdate_ = _GetTime();
	}
}
bool AutoSkipFpsController::IsSkip()
{
	return countSkip_ > 0;
}
