#ifndef __DIRECTX_SCRIPTMANAGER__
#define __DIRECTX_SCRIPTMANAGER__

#include "DxScript.hpp"

namespace directx {

class ManagedScript;
/**********************************************************
//ScriptManager
**********************************************************/
class ScriptManager : public gstd::FileManager::LoadThreadListener {
public:
	enum {
		MAX_CLOSED_SCRIPT_RESULT = 100,
		ID_INVALID = -1,
	};

public:
	ScriptManager();
	virtual ~ScriptManager();
	virtual void Work();
	virtual void Work(int targetType);
	virtual void Render();

	virtual void SetError(std::wstring error) { error_ = error; }
	virtual bool IsError() { return error_ != L""; }

	int GetMainThreadID() { return mainThreadID_; }
	_int64 IssueScriptID()
	{
		{
			gstd::Lock lock(lock_);
			idScript_++;
			return idScript_;
		}
	}

	std::shared_ptr<ManagedScript> GetScript(_int64 id);
	void StartScript(_int64 id);
	void CloseScript(_int64 id);
	void CloseScriptOnType(int type);
	bool IsCloseScript(_int64 id);
	int IsHasCloseScliptWork() { return bHasCloseScriptWork_; }
	int GetAllScriptThreadCount();
	void TerminateScriptAll(std::wstring message);

	_int64 LoadScript(std::wstring path, std::shared_ptr<ManagedScript> script);
	_int64 LoadScript(std::wstring path, int type);
	_int64 LoadScriptInThread(std::wstring path, std::shared_ptr<ManagedScript> script);
	_int64 LoadScriptInThread(std::wstring path, int type);
	virtual void CallFromLoadThread(std::unique_ptr<FileManager::LoadThreadEvent>& event)

	virtual std::shared_ptr<ManagedScript> Create(int type) = 0;
	virtual void RequestEventAll(int type, std::vector<gstd::value>& listValue = std::vector<gstd::value>());
	gstd::value GetScriptResult(_int64 idScript);
	void AddRelativeScriptManager(gstd::ref_count_weak_ptr<ScriptManager> manager) { listRelativeManager_.push_back(manager); }
	static void AddRelativeScriptManagerMutual(gstd::ref_count_weak_ptr<ScriptManager> manager1, gstd::ref_count_weak_ptr<ScriptManager> manager2);

protected:
	gstd::CriticalSection lock_;
	static _int64 idScript_;
	bool bHasCloseScriptWork_;

	std::wstring error_;
	std::map<_int64, std::shared_ptr<ManagedScript>> mapScriptLoad_;
	std::list<std::shared_ptr<ManagedScript>> listScriptRun_;
	std::map<_int64, gstd::value> mapClosedScriptResult_;
	std::list<gstd::ref_count_weak_ptr<ScriptManager>> listRelativeManager_;

	int mainThreadID_;

	_int64 _LoadScript(std::wstring path, std::shared_ptr<ManagedScript> script);
};

/**********************************************************
	//ManagedScript
	**********************************************************/
class ManagedScriptParameter {
public:
	ManagedScriptParameter() {}
	virtual ~ManagedScriptParameter() {}
};
class ManagedScript : public DxScript, public gstd::FileManager::LoadObject {
	friend ScriptManager;

public:
	enum {
		TYPE_ALL = -1,
	};

public:
	ManagedScript();
	virtual void SetScriptManager(ScriptManager* manager);
	virtual void SetScriptParameter(std::shared_ptr<ManagedScriptParameter> param) { scriptParam_ = param; }
	std::shared_ptr<ManagedScriptParameter> GetScriptParameter() { return scriptParam_; }

	int GetScriptType() { return typeScript_; }
	bool IsLoad() { return bLoad_; }
	bool IsEndScript() { return bEndScript_; }
	void SetEndScript() { bEndScript_ = true; }
	bool IsAutoDeleteObject() { return bAutoDeleteObject_; }
	void SetAutoDeleteObject(bool bEneble) { bAutoDeleteObject_ = bEneble; }

	gstd::value RequestEvent(int type, std::vector<gstd::value>& listValue = std::vector<gstd::value>());

	//制御共通関数：共通データ
	static gstd::value Func_SaveCommonDataAreaA1(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_LoadCommonDataAreaA1(gstd::script_machine* machine, int argc, gstd::value const* argv);

	//制御共通関数：スクリプト操作
	static gstd::value Func_LoadScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_LoadScriptInThread(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_StartScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_CloseScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_IsCloseScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetOwnScriptID(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetEventType(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetEventArgument(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_SetScriptArgument(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetScriptResult(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_SetAutoDeleteObject(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_NotifyEvent(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_NotifyEventAll(gstd::script_machine* machine, int argc, gstd::value const* argv);

protected:
	ScriptManager* scriptManager_;

	int typeScript_;
	std::shared_ptr<ManagedScriptParameter> scriptParam_;
	volatile bool bLoad_;
	bool bEndScript_;
	bool bAutoDeleteObject_;

	int typeEvent_;
	std::vector<gstd::value> listValueEvent_;
};

} // namespace directx

#endif
