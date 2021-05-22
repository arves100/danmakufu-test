#ifndef __TOUHOUDANMAKUFU_EXE_SYSTEM__
#define __TOUHOUDANMAKUFU_EXE_SYSTEM__

#include "Common.hpp"
#include "GcLibImpl.hpp"

class SystemController;
class SceneManager;
class TransitionManager;
class SystemInformation;
/**********************************************************
//SystemController
**********************************************************/
class SystemController : public Singleton<SystemController> {
	friend Singleton<SystemController>;

public:
	virtual ~SystemController();
	void Reset();
	void ClearTaskWithoutSystem();

	SceneManager* GetSceneManager() { return sceneManager_.GetPointer(); }
	TransitionManager* GetTransitionManager() { return transitionManager_.GetPointer(); }
	SystemInformation* GetSystemInformation() { return infoSystem_.GetPointer(); }

	void ShowErrorDialog(std::wstring msg);

private:
	SystemController();

	std::shared_ptr<SceneManager> sceneManager_;
	std::shared_ptr<TransitionManager> transitionManager_;
	std::shared_ptr<SystemInformation> infoSystem_;
};

/**********************************************************
//SceneManager
**********************************************************/
class SceneManager {
public:
	SceneManager();
	virtual ~SceneManager();

	//シーン変更
	void TransTitleScene();

	void TransScriptSelectScene(int type);
	void TransScriptSelectScene_All();
	void TransScriptSelectScene_Single();
	void TransScriptSelectScene_Plural();
	void TransScriptSelectScene_Stage();
	void TransScriptSelectScene_Package();
	void TransScriptSelectScene_Directory();
	void TransScriptSelectScene_Last();

	void TransStgScene(std::shared_ptr<ScriptInformation> infoMain, std::shared_ptr<ScriptInformation> infoPlayer, std::shared_ptr<ReplayInformation> infoReplay);
	void TransStgScene(std::shared_ptr<ScriptInformation> infoMain, std::shared_ptr<ReplayInformation> infoReplay);

	void TransPackageScene(std::shared_ptr<ScriptInformation> infoMain, bool bOnlyPackage = false);
};

/**********************************************************
//TransitionManager
**********************************************************/
class TransitionManager {
	enum {
		TASK_PRI = 8,
	};

public:
	TransitionManager();
	virtual ~TransitionManager();
	void DoFadeOut();

private:
	void _CreateCurrentSceneTexture();
	void _AddTask(std::shared_ptr<TransitionEffect> effect);
};

class SystemTransitionEffectTask : public TransitionEffectTask {
public:
	void Work();
	void Render();
};

/**********************************************************
//SystemInformation
**********************************************************/
class SystemInformation {
public:
	SystemInformation();
	virtual ~SystemInformation();

	int GetLastTitleSelectedIndex() { return lastTitleSelectedIndex_; }
	void SetLastTitleSelectedIndex(int index) { lastTitleSelectedIndex_ = index; }
	std::wstring GetLastScriptSearchDirectory() { return dirLastScriptSearch_; }
	void SetLastScriptSearchDirectory(std::wstring dir) { dirLastScriptSearch_ = dir; }
	std::wstring GetLastSelectedScriptPath() { return pathLastSelectedScript_; }
	void SetLastSelectedScriptPath(std::wstring path) { pathLastSelectedScript_ = path; }
	int GetLastSelectScriptSceneType() { return lastSelectScriptSceneType_; }
	void SetLastSelectScriptSceneType(int type) { lastSelectScriptSceneType_ = type; }

	int GetLastSelectedPlayerIndex() { return lastPlayerSelectIndex_; }
	std::vector<std::shared_ptr<ScriptInformation>>& GetLastPlayerSelectedList() { return listLastPlayerSelect_; }
	void SetLastSelectedPlayerIndex(int index, std::vector<std::shared_ptr<ScriptInformation>>& list)
	{
		lastPlayerSelectIndex_ = index;
		listLastPlayerSelect_ = list;
	}

	void UpdateFreePlayerScriptInformationList();
	std::vector<std::shared_ptr<ScriptInformation>>& GetFreePlayerScriptInformationList() { return listFreePlayer_; }

private:
	void _SearchFreePlayerScript(std::wstring dir);

	int lastTitleSelectedIndex_;
	std::wstring dirLastScriptSearch_;
	std::wstring pathLastSelectedScript_;
	int lastSelectScriptSceneType_;

	int lastPlayerSelectIndex_;
	std::vector<std::shared_ptr<ScriptInformation>> listLastPlayerSelect_;

	std::vector<std::shared_ptr<ScriptInformation>> listFreePlayer_;
};

/**********************************************************
//SystemResidentTask
**********************************************************/
class SystemResidentTask : public TaskBase {
public:
	enum {
		TASK_PRI_RENDER_FPS = 9,
	};

public:
	SystemResidentTask();
	~SystemResidentTask();
	void RenderFps();

private:
	DxText textFps_;
};

#endif
