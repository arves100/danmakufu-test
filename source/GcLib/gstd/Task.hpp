#ifndef __GSTD_TASK__
#define __GSTD_TASK__

#include "GstdConstant.hpp"
#include "GstdUtility.hpp"
#include "Logger.hpp"

namespace gstd {

class TaskBase;
class TaskFunction;
class TaskManager;

enum {
	TASK_FREE_ID = 0xffffffff,
	TASK_GROUP_FREE_ID = 0xffffffff,
};
/**********************************************************
//TaskFunction
**********************************************************/
class TaskFunction : public IStringInfo {
	friend TaskManager;

protected:
	std::shared_ptr<TaskBase> task_; //タスクへのポインタ
	int id_; //id
	bool bEnable_;
	int delay_;

public:
	TaskFunction()
	{
		task_ = NULL;
		id_ = TASK_FREE_ID;
		bEnable_ = true;
		delay_ = 0;
	}
	virtual ~TaskFunction() {}
	virtual void Call() = 0;

	std::shared_ptr<TaskBase>& GetTask() { return task_; }
	int GetID() { return id_; }
	bool IsEnable() { return bEnable_; }

	int GetDelay() { return delay_; }
	void SetDelay(int delay) { delay_ = delay; }
	bool IsDelay() { return delay_ > 0; }

	virtual std::string GetInfoAsString();
};

template <class T>
class TTaskFunction : public TaskFunction {
public:
	typedef void (T::*Function)();

public:
	TTaskFunction(std::shared_ptr<T>& task, Function func)
	{
		task_ = task;
		pFunc = func;
	}
	virtual void Call()
	{
		if (task_ != NULL)
			((T*)task_->*pFunc)();
	}

	static std::shared_ptr<TaskFunction>& Create(std::shared_ptr<TaskBase> task, Function func)
	{
		std::shared_ptr<T> dTask = std::static_pointer_cast<T>(task);
		return TTaskFunction<T>::Create(dTask, func);
	}
	static std::shared_ptr<TaskFunction>& Create(std::shared_ptr<T> task, Function func)
	{
		return new TTaskFunction<T>(task, func);
	}

protected:
	Function pFunc; //メンバ関数ポインタ
};

/**********************************************************
//TaskBase
**********************************************************/
class TaskBase : public IStringInfo {
	friend TaskManager;

public:
	TaskBase();
	virtual ~TaskBase();
	int GetTaskID() { return idTask_; }
	int64_t GetTaskIndex() { return indexTask_; }

protected:
	int64_t indexTask_; //TaskManagerによってつけられる一意のインデックス
	int idTask_; //ID
	int idTaskGroup_; //グループID
};

/**********************************************************
//TaskManager
**********************************************************/
class TaskManager : public TaskBase {

public:
	typedef std::map<int, std::vector<std::list<std::shared_ptr<TaskFunction>>>> function_map;

public:
	TaskManager();
	virtual ~TaskManager();
	void Clear(); //全タスク削除
	void ClearTask();
	void AddTask(std::shared_ptr<TaskBase>& task); //タスクを追加
	std::shared_ptr<TaskBase> GetTask(int idTask); //指定したIDのタスクを取得
	std::shared_ptr<TaskBase> GetTask(const std::type_info& info);
	void RemoveTask(std::shared_ptr<TaskBase>& task); //指定したタスクを削除
	void RemoveTask(int idTask); //タスク元IDで削除
	void RemoveTaskGroup(int idGroup); //タスクをグループで削除
	void RemoveTask(const std::type_info& info); //クラス型で削除
	void RemoveTaskWithoutTypeInfo(std::set<const std::type_info*> listInfo); //クラス型以外のタスクを削除
	std::list<std::shared_ptr<TaskBase>>& GetTaskList() { return listTask_; }

	void InitializeFunctionDivision(int divFunc, int maxPri);
	void CallFunction(int divFunc); //タスク機能実行
	void AddFunction(int divFunc, std::shared_ptr<TaskFunction>& func, int pri, int idFunc = TASK_FREE_ID); //タスク機能追加
	void RemoveFunction(std::shared_ptr<TaskBase>& task); //タスク機能削除
	void RemoveFunction(std::shared_ptr<TaskBase>& task, int divFunc, int idFunc); //タスク機能削除
	void RemoveFunction(const std::type_info& info); //タスク機能削除
	function_map GetFunctionMap() { return mapFunc_; }

	void SetFunctionEnable(bool bEnable); //全タスク機能の状態を切り替える
	void SetFunctionEnable(bool bEnable, int divFunc); //タスク機能の状態を切り替える
	void SetFunctionEnable(bool bEnable, int idTask, int divFunc); //タスク機能の状態を切り替える
	void SetFunctionEnable(bool bEnable, int idTask, int divFunc, int idFunc); //タスク機能の状態を切り替える
	void SetFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task, int divFunc); //タスク機能の状態を切り替える
	void SetFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task, int divFunc, int idFunc); //タスク機能の状態を切り替える
	void SetFunctionEnable(bool bEnable, const std::type_info& info, int divFunc); //タスク機能の状態を切り替える

	gstd::CriticalSection& GetStaticLock() { return lockStatic_; }

protected:
	static gstd::CriticalSection lockStatic_;
	std::list<std::shared_ptr<TaskBase>> listTask_; //タスクの元クラス
	function_map mapFunc_; //タスク機能のリスト(divFunc, priority, func)
	int64_t indexTaskManager_; //一意のインデックス

	void _ArrangeTask(); //必要のなくなった領域削除
	void _CheckInvalidFunctionDivision(int divFunc);
};

/**********************************************************
//WorkRenderTaskManager
//動作、描画機能を保持するTaskManager
**********************************************************/
class WorkRenderTaskManager : public TaskManager {
public:
	WorkRenderTaskManager();
	~WorkRenderTaskManager();
	virtual void InitializeFunctionDivision(int maxPriWork, int maxPriRender);

	//動作機能
	void CallWorkFunction();
	void AddWorkFunction(std::shared_ptr<TaskFunction>& func, int pri, int idFunc = TASK_FREE_ID);
	void RemoveWorkFunction(std::shared_ptr<TaskBase>& task, int idFunc);
	void SetWorkFunctionEnable(bool bEnable);
	void SetWorkFunctionEnable(bool bEnable, int idTask);
	void SetWorkFunctionEnable(bool bEnable, int idTask, int idFunc);
	void SetWorkFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task);
	void SetWorkFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task, int idFunc);
	void SetWorkFunctionEnable(bool bEnable, const std::type_info& info);

	//描画機能
	void CallRenderFunction();
	void AddRenderFunction(std::shared_ptr<TaskFunction>& func, int pri, int idFunc = TASK_FREE_ID);
	void RemoveRenderFunction(std::shared_ptr<TaskBase>& task, int idFunc);
	void SetRenderFunctionEnable(bool bEnable);
	void SetRenderFunctionEnable(bool bEnable, int idTask);
	void SetRenderFunctionEnable(bool bEnable, int idTask, int idFunc);
	void SetRenderFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task);
	void SetRenderFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task, int idFunc);
	void SetRenderFunctionEnable(bool bEnable, const std::type_info& info);

private:
	enum {
		DIV_FUNC_WORK, //動作
		DIV_FUNC_RENDER, //描画
	};
};

} // namespace gstd

#endif
