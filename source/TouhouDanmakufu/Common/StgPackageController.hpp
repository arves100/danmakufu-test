#ifndef __TOUHOUDANMAKUFU_DNHSTG_PACKAGECONTROLLER__
#define __TOUHOUDANMAKUFU_DNHSTG_PACKAGECONTROLLER__

#include "StgCommon.hpp"
#include "StgPackageScript.hpp"

class StgPackageInformation;
/**********************************************************
//StgPackageController
**********************************************************/
class StgPackageController {
public:
	StgPackageController(StgSystemController* systemController);
	virtual ~StgPackageController();
	void Initialize();

	void Work();
	void Render();
	void RenderToTransitionTexture();

	StgSystemController* GetSystemController() { return systemController_; }
	std::shared_ptr<StgPackageInformation> GetPackageInformation() { return infoPackage_; }

	std::shared_ptr<StgPackageScriptManager> GetScriptManager() { return scriptManager_; }
	std::shared_ptr<DxScriptObjectManager> GetMainObjectManager() { return scriptManager_->GetObjectManager(); }

private:
	StgSystemController* systemController_;
	std::shared_ptr<StgPackageInformation> infoPackage_;
	std::shared_ptr<StgPackageScriptManager> scriptManager_;
};

/**********************************************************
//StgPackageInformation
**********************************************************/
class StgPackageInformation {
public:
	StgPackageInformation();
	virtual ~StgPackageInformation();

	bool IsEnd() { return bEndPackage_; }
	void SetEnd() { bEndPackage_ = true; }

	void InitializeStageData();
	void FinishCurrentStage();
	std::shared_ptr<StgStageStartData> GetNextStageData() { return nextStageStartData_; }
	void SetNextStageData(std::shared_ptr<StgStageStartData> data) { nextStageStartData_ = data; }
	std::vector<std::shared_ptr<StgStageStartData>>& GetStageDataList() { return listStageData_; }

	std::shared_ptr<ReplayInformation> GetReplayInformation() { return infoReplay_; }
	void SetReplayInformation(std::shared_ptr<ReplayInformation> info) { infoReplay_ = info; }

	std::shared_ptr<ScriptInformation> GetMainScriptInformation() { return infoMainScript_; }
	void SetMainScriptInformation(std::shared_ptr<ScriptInformation> info) { infoMainScript_ = info; }

	int GetPackageStartTime() { return timeStart_; }
	void SetPackageStartTime(int time) { timeStart_ = time; }

private:
	bool bEndPackage_;
	std::shared_ptr<StgStageStartData> nextStageStartData_;
	std::shared_ptr<ReplayInformation> infoReplay_;
	std::vector<std::shared_ptr<StgStageStartData>> listStageData_;
	std::shared_ptr<ScriptInformation> infoMainScript_;
	int timeStart_;
};

#endif
