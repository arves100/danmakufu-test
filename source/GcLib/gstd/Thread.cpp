#include "Thread.hpp"
#include "GstdUtility.hpp"

using namespace gstd;

//================================================================
//Thread
Thread::Thread()
{
	hThread_ = nullptr;
	idThread_ = 0;
	status_ = Status::Stop;
}
Thread::~Thread()
{
	this->Stop();
	this->Join();
	if (hThread_ != nullptr) {
		SDL_DetachThread(hThread_);
		hThread_ = nullptr;
		idThread_ = 0;
	}
}
int Thread::_StaticRun(void* data)
{
	try {
		Thread* thread = reinterpret_cast<Thread*>(data);
		thread->status_ = Status::Run;
		thread->_Run();
		thread->status_ = Status::Stop;
	} catch (...) {
		//エラーは無視
	}
	return 0;
}
void Thread::Start()
{
	if (status_ != Status::Stop) {
		this->Stop();
		this->Join();
	}

	char threadName[28];
	sprintf(threadName, "DanmakufuThread_%8p", this);

	hThread_ = SDL_CreateThread(_StaticRun, threadName, (void*)this);

	if (hThread_ != nullptr)
	{
		idThread_ = SDL_GetThreadID(hThread_);
		status_ = Status::Run;
	}
}
void Thread::Stop()
{
	if (status_ == Status::Run)
		status_ = Status::RequestStop;
}
bool Thread::IsStop()
{
	return hThread_ == nullptr || status_ == Status::Stop;
}
void Thread::Join()
{
	if (hThread_ != nullptr) {
		int ret;
		SDL_WaitThread(hThread_, &ret);
		hThread_ = nullptr;
		idThread_ = 0;
		status_ = Status::Stop;
	}
}

//================================================================
//CriticalSection
CriticalSection::CriticalSection()
{
	idThread_ = 0;
	countLock_ = 0;
	cs_ = SDL_CreateMutex();
}
CriticalSection::~CriticalSection()
{
	if (cs_ != nullptr)
		SDL_DestroyMutex(cs_);
	cs_ = nullptr;
}
void CriticalSection::Enter()
{
	if (SDL_ThreadID() == idThread_) { //カレントスレッド
		countLock_++;
		return;
	}

	SDL_LockMutex(cs_);
	countLock_ = 1;
	idThread_ = SDL_ThreadID();
}
void CriticalSection::Leave()
{
	if (SDL_ThreadID() == idThread_) {
		countLock_--;
		if (countLock_ != 0)
			return;
		if (countLock_ < 0)
			throw std::runtime_error(u8"CriticalSection：Lockしていません");
	} else {
		throw std::runtime_error(u8"CriticalSection：LockしていないのにUnlockしようとしました");
	}
	idThread_ = 0;
	SDL_UnlockMutex(cs_);
}

//================================================================
//ThreadSignal
ThreadSignal::ThreadSignal()
{
	pCond_ = SDL_CreateCond();
	pMutex_ = SDL_CreateMutex();
}
ThreadSignal::~ThreadSignal()
{
	if (pCond_)
	{
		SDL_DestroyCond(pCond_);
		pCond_ = nullptr;
	}

	if (pMutex_)
	{
		SDL_DestroyMutex(pMutex_);
		pMutex_ = nullptr;
	}

}
void ThreadSignal::Wait(Uint32 mills)
{
	while (SDL_CondWaitTimeout(pCond_, pMutex_, mills) == 0);
	SDL_UnlockMutex(pMutex_);
}

void ThreadSignal::SetSignal(bool bLock)
{
	if (bLock)
		SDL_LockMutex(pMutex_);
	else
	{
		SDL_CondSignal(pCond_);
		SDL_UnlockMutex(pMutex_);
	}
}
