#ifndef __TOUHOUDANMAKUFU_EXE_SCRIPT_SELECT_SCENE__
#define __TOUHOUDANMAKUFU_EXE_SCRIPT_SELECT_SCENE__

#include"GcLibImpl.hpp"
#include"Common.hpp"

class ScriptSelectSceneMenuItem;
class ScriptSelectModel;
/**********************************************************
//ScriptSelectScene
**********************************************************/
class ScriptSelectScene : public TaskBase , public MenuTask
{
	public:
		enum
		{
			TASK_PRI_WORK = 5,
			TASK_PRI_RENDER = 5,
		};
		enum
		{
			TYPE_SINGLE,
			TYPE_PLURAL,
			TYPE_STAGE,
			TYPE_PACKAGE,
			TYPE_DIR,
			TYPE_ALL,
		};
		enum
		{
			COUNT_MENU_TEXT = 10,
		};
		class Sort;

	private:
		std::shared_ptr<ScriptSelectModel> model_;
		std::shared_ptr<Sprite2D> spriteBack_;
		std::shared_ptr<Sprite2D> spriteImage_;
		std::vector<std::shared_ptr<DxTextRenderObject> > objMenuText_;
		int frameSelect_;

		virtual void _ChangePage();

	public:
		ScriptSelectScene();
		~ScriptSelectScene();
		virtual void Work();
		virtual void Render();
		virtual void Clear();

		int GetType();
		void SetModel(std::shared_ptr<ScriptSelectModel> model);
		void ClearModel();
		void AddMenuItem(std::list<std::shared_ptr<ScriptSelectSceneMenuItem> > &listItem);

};

class ScriptSelectSceneMenuItem : public MenuItem
{
	friend ScriptSelectScene;
	public:
		enum
		{
			TYPE_SINGLE,
			TYPE_PLURAL,
			TYPE_STAGE,
			TYPE_PACKAGE,
			TYPE_DIR,
		};

	private:
		int type_;
		std::wstring path_;
		std::shared_ptr<ScriptInformation> info_;
		ScriptSelectScene* _GetScriptSelectScene(){return (ScriptSelectScene*)menu_;}

	public:
		ScriptSelectSceneMenuItem(int type, std::wstring path, std::shared_ptr<ScriptInformation> info);
		~ScriptSelectSceneMenuItem();

		int GetType(){return type_;}
		std::wstring GetPath(){return path_;}
		std::shared_ptr<ScriptInformation> GetScriptInformation(){return info_;}
};

class ScriptSelectScene::Sort
{
	public:
	BOOL operator()(const std::shared_ptr<MenuItem>& lf, const std::shared_ptr<MenuItem>& rf)
	{
		std::shared_ptr<MenuItem> lsp = lf;
		std::shared_ptr<MenuItem> rsp = rf;
		ScriptSelectSceneMenuItem* lp = (ScriptSelectSceneMenuItem*)lsp.GetPointer();
		ScriptSelectSceneMenuItem* rp = (ScriptSelectSceneMenuItem*)rsp.GetPointer();

		if(lp->GetType() == ScriptSelectSceneMenuItem::TYPE_DIR &&
			rp->GetType() != ScriptSelectSceneMenuItem::TYPE_DIR)return TRUE;
		if(lp->GetType() != ScriptSelectSceneMenuItem::TYPE_DIR &&
			rp->GetType() == ScriptSelectSceneMenuItem::TYPE_DIR)return FALSE;

		std::wstring lPath = lp->GetPath();
		std::wstring rPath = rp->GetPath();
		BOOL res = CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
			lPath.c_str(), -1, rPath.c_str(), -1);
		return res == CSTR_LESS_THAN ? TRUE : FALSE;
	}
};

/**********************************************************
//ScriptSelectModel
**********************************************************/
class ScriptSelectModel
{
	friend ScriptSelectScene;
	protected:
		volatile bool bCreated_;
		ScriptSelectScene* scene_;

	public:
		ScriptSelectModel();
		virtual ~ScriptSelectModel();

		virtual void CreateMenuItem() = 0;
		bool IsCreated(){return bCreated_;}
};

class ScriptSelectFileModel : public ScriptSelectModel , public Thread
{
	public:
		enum
		{
			TYPE_SINGLE = ScriptSelectScene::TYPE_SINGLE,
			TYPE_PLURAL = ScriptSelectScene::TYPE_PLURAL,
			TYPE_STAGE = ScriptSelectScene::TYPE_STAGE,
			TYPE_PACKAGE = ScriptSelectScene::TYPE_PACKAGE,
			TYPE_DIR = ScriptSelectScene::TYPE_DIR,
			TYPE_ALL = ScriptSelectScene::TYPE_ALL,
		};

	protected:
		int type_;
		std::wstring dir_;
		std::wstring pathWait_;
		int timeLastUpdate_;

		std::list<std::shared_ptr<ScriptSelectSceneMenuItem> > listItem_;
		virtual void _Run();
		virtual void _SearchScript(std::wstring dir);
		void _CreateMenuItem(std::wstring path);
		bool _IsValidScriptInformation(std::shared_ptr<ScriptInformation> info);
		int _ConvertTypeInfoToItem(int typeInfo);
	public:
		ScriptSelectFileModel(int type, std::wstring dir);
		virtual ~ScriptSelectFileModel();
		virtual void CreateMenuItem();

		int GetType(){return type_;}
		std::wstring GetDirectory(){return dir_;}

		std::wstring GetWaitPath(){return pathWait_;}
		void SetWaitPath(std::wstring path){pathWait_ = path;}
};

/**********************************************************
//PlayTypeSelectScene
**********************************************************/
class PlayTypeSelectScene : public TaskBase , public MenuTask
{
	public:
		enum
		{
			TASK_PRI_WORK = 6,
			TASK_PRI_RENDER = 6,
		};

	private:
		std::shared_ptr<ScriptInformation> info_;
		std::shared_ptr<ReplayInformationManager> replayInfoManager_;

	public:
		PlayTypeSelectScene(std::shared_ptr<ScriptInformation> info);
		void Work();
		void Render();
};
class PlayTypeSelectMenuItem : public TextLightUpMenuItem
{
		std::shared_ptr<DxTextRenderObject> objText_;
		POINT pos_;

		PlayTypeSelectScene* _GetTitleScene(){return (PlayTypeSelectScene*)menu_;}
	public:
		PlayTypeSelectMenuItem(std::wstring text, int x, int y);
		virtual ~PlayTypeSelectMenuItem();
		void Work();
		void Render();
};

/**********************************************************
//PlayerSelectScene
**********************************************************/
class PlayerSelectScene : public TaskBase , public MenuTask
{
	public:
		enum
		{
			TASK_PRI_WORK = 7,
			TASK_PRI_RENDER = 7,
		};

	private:
		std::shared_ptr<Sprite2D> spriteBack_;
		std::shared_ptr<Sprite2D> spriteImage_;
		std::shared_ptr<ScriptInformation> info_;
		std::vector<std::shared_ptr<ScriptInformation> > listPlayer_;
		int frameSelect_;

		virtual void _ChangePage(){frameSelect_ = 0;};
	public:
		PlayerSelectScene(std::shared_ptr<ScriptInformation> info);
		void Work();
		void Render();
};
class PlayerSelectMenuItem : public TextLightUpMenuItem
{
		std::shared_ptr<ScriptInformation> info_;

		PlayerSelectScene* _GetTitleScene(){return (PlayerSelectScene*)menu_;}
	public:
		PlayerSelectMenuItem(std::shared_ptr<ScriptInformation> info);
		virtual ~PlayerSelectMenuItem();
		void Work();
		void Render();

		std::shared_ptr<ScriptInformation> GetScriptInformation(){return info_;}
};


#endif

