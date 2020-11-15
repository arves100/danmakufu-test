#include "Task.hpp"

using namespace gstd;

/**********************************************************
//TaskFunction
**********************************************************/
std::wstring TaskFunction::GetInfoAsString()
{
	return task_->GetInfoAsString();
}

/**********************************************************
//TaskBase
**********************************************************/
TaskBase::TaskBase()
{
	indexTask_ = -1;
	idTask_ = TASK_FREE_ID;
	idTaskGroup_ = TASK_GROUP_FREE_ID;
}
TaskBase::~TaskBase()
{
}

/**********************************************************
//TaskManager
**********************************************************/
gstd::CriticalSection TaskManager::lockStatic_;
TaskManager::TaskManager()
{
	indexTaskManager_ = 0;
}
TaskManager::~TaskManager()
{
	this->Clear();
}
void TaskManager::_ArrangeTask()
{
	//タスク削除領域整理
	std::list<ref_count_ptr<TaskBase>>::iterator itrTask;
	for (itrTask = listTask_.begin(); itrTask != listTask_.end();) {
		if (*itrTask == NULL)
			itrTask = listTask_.erase(itrTask);
		else
			itrTask++;
	}

	//関数削除領域整理
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction>>>>::iterator itrType;
	for (itrType = mapFunc_.begin(); itrType != mapFunc_.end(); itrType++) {
		std::vector<std::list<ref_count_ptr<TaskFunction>>>* vectPri = &itrType->second;
		std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
		for (itrPri = vectPri->begin(); itrPri != vectPri->end(); itrPri++) {
			std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
			std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
			for (itrFunc = listFunc.begin(); itrFunc != listFunc.end();) {
				if (*itrFunc == NULL)
					itrFunc = listFunc.erase(itrFunc);
				else {
					int delay = (*itrFunc)->GetDelay();
					delay = max(0, delay - 1);
					(*itrFunc)->SetDelay(delay);
					itrFunc++;
				}
			}
		}
	}
}
void TaskManager::_CheckInvalidFunctionDivision(int divFunc)
{
	if (mapFunc_.find(divFunc) == mapFunc_.end())
		throw gstd::wexception(L"存在しない機能区分");
}
void TaskManager::Clear()
{
	listTask_.clear();
	mapFunc_.clear();
}
void TaskManager::ClearTask()
{
	listTask_.clear();
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction>>>>::iterator itrDiv;
	for (itrDiv = mapFunc_.begin(); itrDiv != mapFunc_.end(); itrDiv++) {
		std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = (*itrDiv).second;
		vectPri.clear();
	}
}
void TaskManager::AddTask(ref_count_ptr<TaskBase> task)
{
	std::list<ref_count_ptr<TaskBase>>::iterator itr;
	for (itr = listTask_.begin(); itr != listTask_.end(); itr++) {
		ref_count_ptr<TaskBase>& tTask = (*itr);
		if ((*itr) == NULL)
			continue;
		if ((*itr) == task)
			return;
	}
	// task->mTask_ = this;

	//TODO IDの割り振り
	task->indexTask_ = indexTaskManager_;
	indexTaskManager_++;
	listTask_.push_back(task);
}
ref_count_ptr<TaskBase> TaskManager::GetTask(int idTask)
{
	std::list<ref_count_ptr<TaskBase>>::iterator itr;
	for (itr = listTask_.begin(); itr != listTask_.end(); itr++) {
		if ((*itr) == NULL)
			continue;
		if ((*itr)->idTask_ != idTask)
			continue;
		return (*itr);
	}
	return NULL;
}
ref_count_ptr<TaskBase> TaskManager::GetTask(const std::type_info& info)
{
	std::list<ref_count_ptr<TaskBase>>::iterator itr;
	for (itr = listTask_.begin(); itr != listTask_.end(); itr++) {
		if ((*itr) == NULL)
			continue;
		const std::type_info& tInfo = typeid(*(*itr).GetPointer());
		if (info != tInfo)
			continue;
		return (*itr);
	}
	return NULL;
}
void TaskManager::RemoveTask(TaskBase* task)
{
	std::list<ref_count_ptr<TaskBase>>::iterator itr;
	for (itr = listTask_.begin(); itr != listTask_.end(); itr++) {
		if ((*itr) == NULL)
			continue;
		if ((*itr).GetPointer() != task)
			continue;
		if ((*itr)->idTask_ != task->idTask_)
			continue;
		this->RemoveFunction(task);
		(*itr) = NULL;
		break;
	}
}
void TaskManager::RemoveTask(int idTask)
{
	std::list<ref_count_ptr<TaskBase>>::iterator itr;
	for (itr = listTask_.begin(); itr != listTask_.end(); itr++) {
		if ((*itr) == NULL)
			continue;
		if ((*itr)->idTask_ != idTask)
			continue;
		this->RemoveFunction((*itr).GetPointer());
		(*itr) = NULL;
		break;
	}
}
void TaskManager::RemoveTaskGroup(int idGroup)
{
	std::list<ref_count_ptr<TaskBase>>::iterator itr;
	for (itr = listTask_.begin(); itr != listTask_.end(); itr++) {
		if ((*itr) == NULL)
			continue;
		if ((*itr)->idTaskGroup_ != idGroup)
			continue;
		this->RemoveFunction((*itr).GetPointer());
		(*itr) = NULL;
	}
}
void TaskManager::RemoveTask(const std::type_info& info)
{
	std::list<ref_count_ptr<TaskBase>>::iterator itr;
	for (itr = listTask_.begin(); itr != listTask_.end(); itr++) {
		if ((*itr) == NULL)
			continue;
		const std::type_info& tInfo = typeid(*(*itr).GetPointer());
		if (info != tInfo)
			continue;
		this->RemoveFunction((*itr).GetPointer());
		(*itr) = NULL;
	}
}
void TaskManager::RemoveTaskWithoutTypeInfo(std::set<const std::type_info*> listInfo)
{
	std::list<ref_count_ptr<TaskBase>>::iterator itr;
	for (itr = listTask_.begin(); itr != listTask_.end(); itr++) {
		if ((*itr) == NULL)
			continue;
		const std::type_info& tInfo = typeid(*(*itr).GetPointer());
		if (listInfo.find(&tInfo) != listInfo.end())
			continue;
		this->RemoveFunction((*itr).GetPointer());
		(*itr) = NULL;
	}
}
void TaskManager::InitializeFunctionDivision(int divFunc, int maxPri)
{
	if (mapFunc_.find(divFunc) != mapFunc_.end())
		throw gstd::wexception(L"すでに存在している機能区分を初期化しようとしました。");
	std::vector<std::list<ref_count_ptr<TaskFunction>>> vectPri;
	vectPri.resize(maxPri);
	mapFunc_[divFunc] = vectPri;
}
void TaskManager::CallFunction(int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);

	std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
	for (itrPri = vectPri.begin(); itrPri != vectPri.end(); itrPri++) {
		std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
		for (itrFunc = listFunc.begin(); itrFunc != listFunc.end(); itrFunc++) {
			if (*itrFunc == NULL)
				continue;
			if ((*itrFunc)->bEnable_ == false)
				continue;
			if ((*itrFunc)->IsDelay())
				continue;
			(*itrFunc)->Call();
		}
	}
	_ArrangeTask();
}
void TaskManager::AddFunction(int divFunc, ref_count_ptr<TaskFunction> func, int pri, int idFunc)
{
	if (mapFunc_.find(divFunc) == mapFunc_.end())
		throw gstd::wexception(L"存在しない機能区分");
	std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = mapFunc_[divFunc];
	func->id_ = idFunc;
	vectPri[pri].push_back(func);
}
void TaskManager::RemoveFunction(TaskBase* task)
{
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction>>>>::iterator itrDiv;
	for (itrDiv = mapFunc_.begin(); itrDiv != mapFunc_.end(); itrDiv++) {
		std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = (*itrDiv).second;
		std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
		for (itrPri = vectPri.begin(); itrPri != vectPri.end(); itrPri++) {
			std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
			std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
			for (itrFunc = listFunc.begin(); itrFunc != listFunc.end(); itrFunc++) {
				if (*itrFunc == NULL)
					continue;
				if ((*itrFunc)->task_ != task)
					continue;
				if ((*itrFunc)->task_->idTask_ != task->idTask_)
					continue;
				(*itrFunc) = NULL;
			}
		}
	}
}
void TaskManager::RemoveFunction(TaskBase* task, int divFunc, int idFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
	for (itrPri = vectPri.begin(); itrPri != vectPri.end(); itrPri++) {
		std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
		for (itrFunc = listFunc.begin(); itrFunc != listFunc.end(); itrFunc++) {
			if (*itrFunc == NULL)
				continue;
			if ((*itrFunc)->id_ != idFunc)
				continue;
			if ((*itrFunc)->task_->idTask_ != task->idTask_)
				continue;
			(*itrFunc) = NULL;
		}
	}
}
void TaskManager::RemoveFunction(const std::type_info& info)
{
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction>>>>::iterator itrDiv;
	for (itrDiv = mapFunc_.begin(); itrDiv != mapFunc_.end(); itrDiv++) {
		std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = (*itrDiv).second;
		std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
		for (itrPri = vectPri.begin(); itrPri != vectPri.end(); itrPri++) {
			std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
			std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
			for (itrFunc = listFunc.begin(); itrFunc != listFunc.end(); itrFunc++) {
				if (*itrFunc == NULL)
					continue;
				const std::type_info& tInfo = typeid(*(*itrFunc)->task_);
				if (info != tInfo)
					continue;
				(*itrFunc) = NULL;
			}
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable)
{
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction>>>>::iterator itrDiv;
	for (itrDiv = mapFunc_.begin(); itrDiv != mapFunc_.end(); itrDiv++) {
		std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = (*itrDiv).second;
		std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
		for (itrPri = vectPri.begin(); itrPri != vectPri.end(); itrPri++) {
			std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
			std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
			for (itrFunc = listFunc.begin(); itrFunc != listFunc.end(); itrFunc++) {
				if (*itrFunc == NULL)
					continue;
				(*itrFunc)->bEnable_ = bEnable;
			}
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable, int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
	for (itrPri = vectPri.begin(); itrPri != vectPri.end(); itrPri++) {
		std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
		for (itrFunc = listFunc.begin(); itrFunc != listFunc.end(); itrFunc++) {
			if (*itrFunc == NULL)
				continue;
			(*itrFunc)->bEnable_ = bEnable;
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable, int idTask, int divFunc)
{
	ref_count_ptr<TaskBase> task = this->GetTask(idTask);
	if (task == NULL)
		return;
	this->SetFunctionEnable(bEnable, task.GetPointer(), divFunc);
}
void TaskManager::SetFunctionEnable(bool bEnable, int idTask, int divFunc, int idFunc)
{
	ref_count_ptr<TaskBase> task = this->GetTask(idTask);
	if (task == NULL)
		return;
	this->SetFunctionEnable(bEnable, task.GetPointer(), divFunc, idFunc);
}
void TaskManager::SetFunctionEnable(bool bEnable, TaskBase* task, int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
	for (itrPri = vectPri.begin(); itrPri != vectPri.end(); itrPri++) {
		std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
		for (itrFunc = listFunc.begin(); itrFunc != listFunc.end(); itrFunc++) {
			if (*itrFunc == NULL)
				continue;
			if ((*itrFunc)->task_ != task)
				continue;
			if ((*itrFunc)->task_->idTask_ != task->idTask_)
				continue;
			(*itrFunc)->bEnable_ = bEnable;
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable, TaskBase* task, int divFunc, int idFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
	for (itrPri = vectPri.begin(); itrPri != vectPri.end(); itrPri++) {
		std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
		for (itrFunc = listFunc.begin(); itrFunc != listFunc.end(); itrFunc++) {
			if (*itrFunc == NULL)
				continue;
			if ((*itrFunc)->task_ != task)
				continue;
			if ((*itrFunc)->task_->idTask_ != task->idTask_)
				continue;
			if ((*itrFunc)->id_ != idFunc)
				continue;
			(*itrFunc)->bEnable_ = bEnable;
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable, const std::type_info& info, int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	std::vector<std::list<ref_count_ptr<TaskFunction>>>& vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction>>>::iterator itrPri;
	for (itrPri = vectPri.begin(); itrPri != vectPri.end(); itrPri++) {
		std::list<ref_count_ptr<TaskFunction>>& listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction>>::iterator itrFunc;
		for (itrFunc = listFunc.begin(); itrFunc != listFunc.end(); itrFunc++) {
			if (*itrFunc == NULL)
				continue;
			const std::type_info& tInfo = typeid(*(*itrFunc)->task_);
			if (info != tInfo)
				continue;
			(*itrFunc)->bEnable_ = bEnable;
		}
	}
}

/**********************************************************
//WorkRenderTaskManager
**********************************************************/
WorkRenderTaskManager::WorkRenderTaskManager()
{
}
WorkRenderTaskManager::~WorkRenderTaskManager()
{
}
void WorkRenderTaskManager::InitializeFunctionDivision(int maxPriWork, int maxPriRender)
{
	this->TaskManager::InitializeFunctionDivision(DIV_FUNC_WORK, maxPriWork);
	this->TaskManager::InitializeFunctionDivision(DIV_FUNC_RENDER, maxPriRender);
}
void WorkRenderTaskManager::CallWorkFunction()
{
	CallFunction(DIV_FUNC_WORK);
}
void WorkRenderTaskManager::AddWorkFunction(ref_count_ptr<TaskFunction> func, int pri, int idFunc)
{
	func->SetDelay(1);
	AddFunction(DIV_FUNC_WORK, func, pri, idFunc);
}
void WorkRenderTaskManager::RemoveWorkFunction(TaskBase* task, int idFunc)
{
	RemoveFunction(task, DIV_FUNC_WORK, idFunc);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable)
{
	SetFunctionEnable(bEnable, DIV_FUNC_WORK);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, int idTask)
{
	SetFunctionEnable(bEnable, idTask, DIV_FUNC_WORK);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, int idTask, int idFunc)
{
	SetFunctionEnable(bEnable, idTask, DIV_FUNC_WORK, idFunc);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, TaskBase* task)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_WORK);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, TaskBase* task, int idFunc)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_WORK, idFunc);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, const std::type_info& info)
{
	SetFunctionEnable(bEnable, info, DIV_FUNC_WORK);
}

void WorkRenderTaskManager::CallRenderFunction()
{
	CallFunction(DIV_FUNC_RENDER);
}
void WorkRenderTaskManager::AddRenderFunction(ref_count_ptr<TaskFunction> func, int pri, int idFunc)
{
	AddFunction(DIV_FUNC_RENDER, func, pri, idFunc);
}
void WorkRenderTaskManager::RemoveRenderFunction(TaskBase* task, int idFunc)
{
	RemoveFunction(task, DIV_FUNC_RENDER, idFunc);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable)
{
	SetFunctionEnable(bEnable, DIV_FUNC_RENDER);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, int idTask)
{
	SetFunctionEnable(bEnable, idTask, DIV_FUNC_RENDER);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, int idTask, int idFunc)
{
	SetFunctionEnable(bEnable, idTask, DIV_FUNC_RENDER, idFunc);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, TaskBase* task)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_RENDER);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, TaskBase* task, int idFunc)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_RENDER, idFunc);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, const std::type_info& info)
{
	SetFunctionEnable(bEnable, info, DIV_FUNC_RENDER);
}
