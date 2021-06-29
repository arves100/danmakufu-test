
#include "Task.hpp"

using namespace gstd;

/**********************************************************
//TaskFunction
**********************************************************/
std::string TaskFunction::GetInfoAsString()
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
	auto itrTask = listTask_.begin(), end = listTask_.end();
	for (; itrTask != end;) {
		if (*itrTask == NULL)
			itrTask = listTask_.erase(itrTask);
		else
			itrTask++;
	}

	//関数削除領域整理
	auto itrType = mapFunc_.begin(), end3 = mapFunc_.end();
	for (itrType; itrType != end3; itrType++) {
		auto& vectPri = itrType->second;
		auto itrPri = vectPri.begin(), end2 = vectPri.end();
		for (itrPri ; itrPri != end2; itrPri++) {
			auto& listFunc = *itrPri;
			auto itrFunc = listFunc.begin(), end4 = listFunc.end();
			for (; itrFunc != end4;) {
				if (*itrFunc == NULL)
					itrFunc = listFunc.erase(itrFunc);
				else {
					int delay = (*itrFunc)->GetDelay();
					delay = _MAX(0, delay - 1);
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
		throw std::runtime_error(u8"存在しない機能区分");
}
void TaskManager::Clear()
{
	listTask_.clear();
	mapFunc_.clear();
}
void TaskManager::ClearTask()
{
	listTask_.clear();
	auto itrDiv = mapFunc_.begin(), end = mapFunc_.end();
	for (; itrDiv != end; itrDiv++) {
		auto& vectPri = (*itrDiv).second;
		vectPri.clear();
	}
}
void TaskManager::AddTask(std::shared_ptr<TaskBase>& task)
{
	auto itr = listTask_.begin(), end = listTask_.end();
	for (; itr != end; itr++) {
		auto& tTask = (*itr);
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
std::shared_ptr<TaskBase> TaskManager::GetTask(int idTask)
{
	auto itr = listTask_.begin(), end = listTask_.end();
	for (; itr != end; itr++) {
		if ((*itr) == NULL)
			continue;
		if ((*itr)->idTask_ != idTask)
			continue;
		return (*itr);
	}
	return std::shared_ptr<TaskBase>();
}
std::shared_ptr<TaskBase> TaskManager::GetTask(const std::type_info& info)
{
	auto itr = listTask_.begin(), end = listTask_.end();
	for (; itr != end; itr++) {
		if ((*itr) == NULL)
			continue;
		const std::type_info& tInfo = typeid(*(*itr).get());
		if (info != tInfo)
			continue;
		return (*itr);
	}
	return std::shared_ptr<TaskBase>();
}
void TaskManager::RemoveTask(std::shared_ptr<TaskBase>& task)
{
	auto itr = listTask_.begin(), end = listTask_.end();
	for (; itr != end; itr++) {
		if ((*itr) == NULL)
			continue;
		if ((*itr) != task)
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
	auto itr = listTask_.begin(), end = listTask_.end();
	for (; itr != end; itr++) {
		if ((*itr) == NULL)
			continue;
		if ((*itr)->idTask_ != idTask)
			continue;
		this->RemoveFunction((*itr));
		(*itr) = NULL;
		break;
	}
}
void TaskManager::RemoveTaskGroup(int idGroup)
{
	auto itr = listTask_.begin(), end = listTask_.end();
	for (; itr != end; itr++) {
		if ((*itr) == NULL)
			continue;
		if ((*itr)->idTaskGroup_ != idGroup)
			continue;
		this->RemoveFunction(*itr);
		(*itr).reset();
	}
}
void TaskManager::RemoveTask(const std::type_info& info)
{
	auto itr = listTask_.begin(), end = listTask_.end();
	for (; itr != end; itr++) {
		if ((*itr) == NULL)
			continue;
		const std::type_info& tInfo = typeid(*(*itr).get());
		if (info != tInfo)
			continue;
		this->RemoveFunction(*itr);
		(*itr).reset();
	}
}
void TaskManager::RemoveTaskWithoutTypeInfo(std::set<const std::type_info*> listInfo)
{
	auto itr = listTask_.begin(), end = listTask_.end();
	for (; itr != end; itr++) {
		if ((*itr) == NULL)
			continue;
		const std::type_info& tInfo = typeid(*(*itr).get());
		if (listInfo.find(&tInfo) != listInfo.end())
			continue;
		this->RemoveFunction((*itr));
		(*itr) = NULL;
	}
}
void TaskManager::InitializeFunctionDivision(int divFunc, int maxPri)
{
	if (mapFunc_.find(divFunc) != mapFunc_.end())
		throw std::runtime_error(u8"すでに存在している機能区分を初期化しようとしました。");
	std::vector<std::list<std::shared_ptr<TaskFunction>>> vectPri;
	vectPri.resize(maxPri);
	mapFunc_[divFunc] = vectPri;
}
void TaskManager::CallFunction(int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);

	auto& vectPri = mapFunc_[divFunc];
	auto itrPri = vectPri.begin(), end = vectPri.end();
	for (; itrPri != end; itrPri++) {
		auto& listFunc = *itrPri;
		auto itrFunc = listFunc.begin(), end2 = listFunc.end();
		for (; itrFunc != end2; itrFunc++) {
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
void TaskManager::AddFunction(int divFunc, std::shared_ptr<TaskFunction>& func, int pri, int idFunc)
{
	if (mapFunc_.find(divFunc) == mapFunc_.end())
		throw std::runtime_error(u8"存在しない機能区分");
	auto& vectPri = mapFunc_[divFunc];
	func->id_ = idFunc;
	vectPri[pri].push_back(func);
}
void TaskManager::RemoveFunction(std::shared_ptr<TaskBase>& task)
{
	auto itrDiv = mapFunc_.begin(), end = mapFunc_.end();
	for (; itrDiv != end; itrDiv++) {
		auto& vectPri = (*itrDiv).second;
		auto itrPri = vectPri.begin(), end3 = vectPri.end();
		for (; itrPri != end3; itrPri++) {
			auto& listFunc = *itrPri;
			auto itrFunc = listFunc.begin(), end2 = listFunc.end();
			for (; itrFunc != end2; itrFunc++) {
				if (itrFunc->get() == nullptr)
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
void TaskManager::RemoveFunction(std::shared_ptr<TaskBase>& task, int divFunc, int idFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	auto& vectPri = mapFunc_[divFunc];
	auto itrPri = vectPri.begin(), end = vectPri.end();
	for (; itrPri != end; itrPri++) {
		auto& listFunc = *itrPri;
		auto itrFunc = listFunc.begin(), end2 = listFunc.end();
		for (; itrFunc != end2; itrFunc++) {
			if (itrFunc->get() == nullptr)
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
	auto itrDiv = mapFunc_.begin(), end = mapFunc_.end();
	for (; itrDiv != end; itrDiv++) {
		auto& vectPri = (*itrDiv).second;
		auto itrPri = vectPri.begin(), end2 = vectPri.end();
		for (; itrPri != end2; itrPri++) {
			auto& listFunc = *itrPri;
			auto itrFunc = listFunc.begin(), end3 = listFunc.end();
			for (; itrFunc != end3; itrFunc++) {
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
	auto itrDiv = mapFunc_.begin(), end = mapFunc_.end();
	for (; itrDiv != end; itrDiv++) {
		auto& vectPri = (*itrDiv).second;
		auto itrPri = vectPri.begin(), end2 = vectPri.end();
		for (; itrPri != end2; itrPri++) {
			auto& listFunc = *itrPri;
			auto itrFunc = listFunc.begin(), end3 = listFunc.end();
			for (; itrFunc != end3; itrFunc++) {
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
	auto& vectPri = mapFunc_[divFunc];
	auto itrPri = vectPri.begin(), end = vectPri.end();
	for (; itrPri != end; itrPri++) {
		auto& listFunc = *itrPri;
		auto itrFunc = listFunc.begin(), end2 = listFunc.end();
		for (; itrFunc != end2; itrFunc++) {
			if (*itrFunc == NULL)
				continue;
			(*itrFunc)->bEnable_ = bEnable;
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable, int idTask, int divFunc)
{
	auto task = this->GetTask(idTask);
	if (task == NULL)
		return;
	this->SetFunctionEnable(bEnable, task, divFunc);
}
void TaskManager::SetFunctionEnable(bool bEnable, int idTask, int divFunc, int idFunc)
{
	auto task = this->GetTask(idTask);
	if (task == NULL)
		return;
	this->SetFunctionEnable(bEnable, task, divFunc, idFunc);
}
void TaskManager::SetFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task, int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	auto& vectPri = mapFunc_[divFunc];
	auto itrPri = vectPri.begin(), end = vectPri.end();
	for (; itrPri != end; itrPri++) {
		auto& listFunc = *itrPri;
		auto itrFunc = listFunc.begin(), end2 = listFunc.end();
		for (; itrFunc != end2; itrFunc++) {
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
void TaskManager::SetFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task, int divFunc, int idFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	auto& vectPri = mapFunc_[divFunc];
	auto itrPri = vectPri.begin(), end = vectPri.end();
	for (; itrPri != end; itrPri++) {
		auto& listFunc = *itrPri;
		auto itrFunc = listFunc.begin(), end2 = listFunc.end();
		for (; itrFunc != end2; itrFunc++) {
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
	auto& vectPri = mapFunc_[divFunc];
	auto itrPri = vectPri.begin(), end = vectPri.end();
	for (; itrPri != end; itrPri++) {
		auto& listFunc = *itrPri;
		auto itrFunc = listFunc.begin(), end2 = listFunc.end();
		for (; itrFunc != end2; itrFunc++) {
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
void WorkRenderTaskManager::AddWorkFunction(std::shared_ptr<TaskFunction>& func, int pri, int idFunc)
{
	func->SetDelay(1);
	AddFunction(DIV_FUNC_WORK, func, pri, idFunc);
}
void WorkRenderTaskManager::RemoveWorkFunction(std::shared_ptr<TaskBase>& task, int idFunc)
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
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_WORK);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task, int idFunc)
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
void WorkRenderTaskManager::AddRenderFunction(std::shared_ptr<TaskFunction>& func, int pri, int idFunc)
{
	AddFunction(DIV_FUNC_RENDER, func, pri, idFunc);
}
void WorkRenderTaskManager::RemoveRenderFunction(std::shared_ptr<TaskBase>& task, int idFunc)
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
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_RENDER);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, std::shared_ptr<TaskBase>& task, int idFunc)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_RENDER, idFunc);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, const std::type_info& info)
{
	SetFunctionEnable(bEnable, info, DIV_FUNC_RENDER);
}
