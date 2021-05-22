#ifndef __TOUHOUDANMAKUFU_DNHSTG_SYSTEM__
#define __TOUHOUDANMAKUFU_DNHSTG_SYSTEM__

#include "StgCommon.hpp"
#include "StgEnemy.hpp"
#include "StgIntersection.hpp"
#include "StgItem.hpp"
#include "StgPackageController.hpp"
#include "StgPlayer.hpp"
#include "StgShot.hpp"
#include "StgStageController.hpp"
#include "StgStageScript.hpp"
#include "StgUserExtendScene.hpp"

class StgSystemInformation;
/**********************************************************
//StgSystemController
**********************************************************/
class StgSystemController : public TaskBase {
public:
	enum {
		TASK_PRI_WORK = 4,
		TASK_PRI_RENDER = 4,
	};

public:
	StgSystemController();
	~StgSystemController();
	void Initialize(std::shared_ptr<StgSystemInformation> infoSystem);
	void Start(std::shared_ptr<ScriptInformation> infoPlayer, std::shared_ptr<ReplayInformation> infoReplay);
	void Work();
	void Render();
	void RenderScriptObject();
	void RenderScriptObject(int priMin, int priMax);

	std::shared_ptr<StgSystemInformation>& GetSystemInformation() { return infoSystem_; }
	StgStageController* GetStageController() { return stageController_.GetPointer(); }
	StgPackageController* GetPackageController() { return packageController_.GetPointer(); }
	std::shared_ptr<StgControlScriptInformation>& GetControlScriptInformation() { return infoControlScript_; }

	gstd::shared_ptr<ScriptEngineCache> GetScriptEngineCache() { return scriptEngineCache_; }
	gstd::shared_ptr<ScriptCommonDataManager> GetCommonDataManager() { return commonDataManager_; }

	void StartStgScene(std::shared_ptr<StgStageInformation> infoStage, std::shared_ptr<ReplayInformation::StageData> replayStageData);
	void StartStgScene(std::shared_ptr<StgStageStartData> startData);

	void TransStgEndScene();
	void TransReplaySaveScene();

	std::shared_ptr<ReplayInformation> CreateReplayInformation();
	void TerminateScriptAll();

protected:
	virtual void DoEnd() = 0;
	virtual void DoRetry() = 0;
	void _ControlScene();

	std::shared_ptr<StgSystemInformation> infoSystem_;
	std::shared_ptr<ScriptEngineCache> scriptEngineCache_;
	gstd::shared_ptr<ScriptCommonDataManager> commonDataManager_;

	std::shared_ptr<StgEndScene> endScene_;
	std::shared_ptr<StgReplaySaveScene> replaySaveScene_;

	std::shared_ptr<StgStageController> stageController_;

	std::shared_ptr<StgPackageController> packageController_;
	std::shared_ptr<StgControlScriptInformation> infoControlScript_;
};

/**********************************************************
//StgSystemInformation
**********************************************************/
class StgSystemInformation {
public:
	enum {
		SCENE_NULL,
		SCENE_PACKAGE_CONTROL,
		SCENE_STG,
		SCENE_REPLAY_SAVE,
		SCENE_END,
	};

public:
	StgSystemInformation();
	virtual ~StgSystemInformation();

	bool IsPackageMode();
	void ResetRetry();
	int GetScene() { return scene_; }
	void SetScene(int scene) { scene_ = scene; }
	bool IsStgEnd() { return bEndStg_; }
	void SetStgEnd() { bEndStg_ = true; }
	bool IsRetry() { return bRetry_; }
	void SetRetry() { bRetry_ = true; }
	bool IsError() { return listError_.size() > 0; }
	void SetError(std::wstring error) { listError_.push_back(error); }
	std::wstring GetErrorMessage();

	std::wstring GetPauseScriptPath() { return pathPauseScript_; }
	void SetPauseScriptPath(std::wstring path) { pathPauseScript_ = path; }
	std::wstring GetEndSceneScriptPath() { return pathEndSceneScript_; }
	void SetEndSceneScriptPath(std::wstring path) { pathEndSceneScript_ = path; }
	std::wstring GetReplaySaveSceneScriptPath() { return pathReplaySaveSceneScript_; }
	void SetReplaySaveSceneScriptPath(std::wstring path) { pathReplaySaveSceneScript_ = path; }

	std::shared_ptr<ScriptInformation> GetMainScriptInformation() { return infoMain_; }
	void SetMainScriptInformation(std::shared_ptr<ScriptInformation> info) { infoMain_ = info; }

	std::shared_ptr<ReplayInformation> GetActiveReplayInformation() { return infoReplayActive_; }
	void SetActiveReplayInformation(std::shared_ptr<ReplayInformation> info) { infoReplayActive_ = info; }

	void SetInvaridRenderPriority(int priMin, int priMax);
	int GetInvaridRenderPriorityMin() { return invalidPriMin_; }
	int GetInvaridRenderPriorityMax() { return invalidPriMax_; }

	void AddReplayTargetKey(int id) { listReplayTargetKey_.insert(id); }
	std::set<int> GetReplayTargetKeyList() { return listReplayTargetKey_; }

private:
	int scene_;
	bool bEndStg_;
	bool bRetry_;

	std::wstring pathPauseScript_;
	std::wstring pathEndSceneScript_;
	std::wstring pathReplaySaveSceneScript_;

	std::list<std::wstring> listError_;
	std::shared_ptr<ScriptInformation> infoMain_;
	std::shared_ptr<ReplayInformation> infoReplayActive_; //アクティブリプレイ情報

	int invalidPriMin_;
	int invalidPriMax_;
	std::set<int> listReplayTargetKey_;
};

#endif
