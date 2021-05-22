#ifndef __TOUHOUDANMAKUFU_DNHSTG_STAGECONTROLLER__
#define __TOUHOUDANMAKUFU_DNHSTG_STAGECONTROLLER__

#include "StgCommon.hpp"
#include "StgEnemy.hpp"
#include "StgIntersection.hpp"
#include "StgItem.hpp"
#include "StgPlayer.hpp"
#include "StgShot.hpp"
#include "StgStageScript.hpp"
#include "StgUserExtendScene.hpp"

class StgStageInformation;
class StgStageStartData;
class PseudoSlowInformation;
/**********************************************************
//StgStageController
**********************************************************/
class StgStageController {
public:
	StgStageController(StgSystemController* systemController);
	virtual ~StgStageController();
	void Initialize(std::shared_ptr<StgStageStartData> startData);
	void CloseScene();
	void Work();
	void Render();
	void RenderToTransitionTexture();

	std::shared_ptr<StgStageScriptObjectManager> GetMainObjectManager() { return objectManagerMain_; }
	StgStageScriptManager* GetScriptManagerP() { return scriptManager_.GetPointer(); }
	std::shared_ptr<StgStageScriptManager> GetScriptManagerR() { return scriptManager_; }
	StgEnemyManager* GetEnemyManager() { return enemyManager_.GetPointer(); }
	StgShotManager* GetShotManager() { return shotManager_.GetPointer(); }
	StgItemManager* GetItemManager() { return itemManager_.GetPointer(); }
	StgIntersectionManager* GetIntersectionManager() { return intersectionManager_.GetPointer(); }
	std::shared_ptr<StgPauseScene> GetPauseManager() { return pauseManager_; }

	std::shared_ptr<DxScriptObjectBase>::unsync GetMainRenderObject(int idObject);
	std::shared_ptr<StgPlayerObject>::unsync GetPlayerObject();

	StgSystemController* GetSystemController() { return systemController_; }
	std::shared_ptr<StgSystemInformation> GetSystemInformation() { return infoSystem_; }
	std::shared_ptr<StgStageInformation> GetStageInformation() { return infoStage_; }
	std::shared_ptr<KeyReplayManager> GetKeyReplayManager() { return keyReplayManager_; }

	std::shared_ptr<PseudoSlowInformation> GetSlowInformation() { return infoSlow_; }

private:
	void _SetupReplayTargetCommonDataArea(_int64 idScript);

	StgSystemController* systemController_;
	std::shared_ptr<StgSystemInformation> infoSystem_;
	std::shared_ptr<StgStageInformation> infoStage_;
	std::shared_ptr<PseudoSlowInformation> infoSlow_;

	std::shared_ptr<StgPauseScene> pauseManager_;
	std::shared_ptr<KeyReplayManager> keyReplayManager_;
	std::shared_ptr<StgStageScriptObjectManager> objectManagerMain_;
	std::shared_ptr<StgStageScriptManager> scriptManager_;
	std::shared_ptr<StgEnemyManager> enemyManager_;
	std::shared_ptr<StgShotManager> shotManager_;
	std::shared_ptr<StgItemManager> itemManager_;
	std::shared_ptr<StgIntersectionManager> intersectionManager_;
};

/**********************************************************
//StgStageInformation
**********************************************************/
class StgStageInformation {
public:
	enum {
		RESULT_UNKNOWN,
		RESULT_BREAK_OFF,
		RESULT_PLAYER_DOWN,
		RESULT_CLEARED,
	};

public:
	StgStageInformation();
	virtual ~StgStageInformation();
	bool IsEnd() { return bEndStg_; }
	void SetEnd() { bEndStg_ = true; }
	bool IsPause() { return bPause_; }
	void SetPause(bool bPause) { bPause_ = bPause; }
	bool IsReplay() { return bReplay_; }
	void SetReplay(bool bReplay) { bReplay_ = bReplay; }
	int GetCurrentFrame() { return frame_; }
	void AdvanceFrame() { frame_++; }
	int GetStageIndex() { return stageIndex_; }
	void SetStageIndex(int index) { stageIndex_ = index; }

	std::shared_ptr<ScriptInformation> GetMainScriptInformation() { return infoMainScript_; }
	void SetMainScriptInformation(std::shared_ptr<ScriptInformation> info) { infoMainScript_ = info; }
	std::shared_ptr<ScriptInformation> GetPlayerScriptInformation() { return infoPlayerScript_; }
	void SetPlayerScriptInformation(std::shared_ptr<ScriptInformation> info) { infoPlayerScript_ = info; }
	std::shared_ptr<StgPlayerInformation> GetPlayerObjectInformation() { return infoPlayerObject_; }
	void SetPlayerObjectInformation(std::shared_ptr<StgPlayerInformation> info) { infoPlayerObject_ = info; }
	std::shared_ptr<ReplayInformation::StageData> GetReplayData() { return replayStageData_; }
	void SetReplayData(std::shared_ptr<ReplayInformation::StageData> data) { replayStageData_ = data; }

	RECT GetStgFrameRect() { return rcStgFrame_; }
	void SetStgFrameRect(RECT rect, bool bUpdateFocusResetValue = true);
	int GetStgFrameMinPriority() { return priMinStgFrame_; }
	void SetStgFrameMinPriority(int pri) { priMinStgFrame_ = pri; }
	int GetStgFrameMaxPriority() { return priMaxStgFrame_; }
	void SetStgFrameMaxPriority(int pri) { priMaxStgFrame_ = pri; }
	int GetShotObjectPriority() { return priShotObject_; }
	void SetShotObjectPriority(int pri) { priShotObject_ = pri; }
	int GetItemObjectPriority() { return priItemObject_; }
	void SetItemObjectPriority(int pri) { priItemObject_ = pri; }
	int GetCameraFocusPermitPriority() { return priCameraFocusPermit_; }
	void SetCameraFocusPermitPriority(int pri) { priCameraFocusPermit_ = pri; }
	RECT GetShotAutoDeleteClip() { return rcShotAutoDeleteClip_; }
	void SetShotAutoDeleteClip(RECT rect) { rcShotAutoDeleteClip_ = rect; }

	std::shared_ptr<MersenneTwister> GetMersenneTwister() { return rand_; }
	void SetMersenneTwisterSeed(int seed) { rand_->Initialize(seed); }
	_int64 GetScore() { return score_; }
	void SetScore(_int64 score) { score_ = score; }
	void AddScore(_int64 inc) { score_ += inc; }
	_int64 GetGraze() { return graze_; }
	void SetGraze(_int64 graze) { graze_ = graze; }
	void AddGraze(_int64 inc) { graze_ += inc; }
	_int64 GetPoint() { return point_; }
	void SetPoint(_int64 point) { point_ = point; }
	void AddPoint(_int64 inc) { point_ += inc; }

	int GetResult() { return result_; }
	void SetResult(int result) { result_ = result; }

	int GetStageStartTime() { return timeStart_; }
	void SetStageStartTime(int time) { timeStart_ = time; }

private:
	bool bEndStg_;
	bool bPause_;
	bool bReplay_; //リプレイ
	int frame_;
	int stageIndex_;

	std::shared_ptr<ScriptInformation> infoMainScript_;
	std::shared_ptr<ScriptInformation> infoPlayerScript_;
	std::shared_ptr<StgPlayerInformation> infoPlayerObject_;
	std::shared_ptr<ReplayInformation::StageData> replayStageData_;

	//STG設定
	RECT rcStgFrame_;
	int priMinStgFrame_;
	int priMaxStgFrame_;
	int priShotObject_;
	int priItemObject_;
	int priCameraFocusPermit_;
	RECT rcShotAutoDeleteClip_;

	//STG情報
	std::shared_ptr<MersenneTwister> rand_;
	_int64 score_;
	_int64 graze_;
	_int64 point_;
	int result_;
	int timeStart_;
};

/**********************************************************
//StgStageStartData
**********************************************************/
class StgStageStartData {
public:
	StgStageStartData() {}
	virtual ~StgStageStartData() {}

	std::shared_ptr<StgStageInformation> GetStageInformation() { return infoStage_; }
	void SetStageInformation(std::shared_ptr<StgStageInformation> info) { infoStage_ = info; }
	std::shared_ptr<ReplayInformation::StageData> GetStageReplayData() { return replayStageData_; }
	void SetStageReplayData(std::shared_ptr<ReplayInformation::StageData> data) { replayStageData_ = data; }
	std::shared_ptr<StgStageInformation> GetPrevStageInformation() { return prevStageInfo_; }
	void SetPrevStageInformation(std::shared_ptr<StgStageInformation> info) { prevStageInfo_ = info; }
	std::shared_ptr<StgPlayerInformation> GetPrevPlayerInformation() { return prevPlayerInfo_; }
	void SetPrevPlayerInformation(std::shared_ptr<StgPlayerInformation> info) { prevPlayerInfo_ = info; }

private:
	std::shared_ptr<StgStageInformation> infoStage_;
	std::shared_ptr<ReplayInformation::StageData> replayStageData_;
	std::shared_ptr<StgStageInformation> prevStageInfo_;
	std::shared_ptr<StgPlayerInformation> prevPlayerInfo_;
};

/**********************************************************
//PseudoSlowInformation
**********************************************************/
class PseudoSlowInformation : public gstd::FpsControlObject {
public:
	class SlowData;
	enum {
		OWNER_PLAYER = 0,
		OWNER_ENEMY,
		TARGET_ALL,
	};

public:
	PseudoSlowInformation() { current_ = 0; }
	virtual ~PseudoSlowInformation() {}
	virtual int GetFps();

	bool IsValidFrame(int target);
	void Next();

	void AddSlow(int fps, int owner, int target);
	void RemoveSlow(int owner, int target);

private:
	int current_;
	std::map<int, gstd::shared_ptr<SlowData>> mapDataPlayer_;
	std::map<int, gstd::shared_ptr<SlowData>> mapDataEnemy_;
	std::map<int, bool> mapValid_;
};

class PseudoSlowInformation::SlowData {
public:
	SlowData() { fps_ = STANDARD_FPS; }
	virtual ~SlowData() {}
	int GetFps() { return fps_; }
	void SetFps(int fps) { fps_ = fps; }

private:
	int fps_;
};

#endif
