#ifndef __GSTD_THREAD__
#define __GSTD_THREAD__

#include "GstdConstant.hpp"

namespace gstd {

//================================================================
//Thread
class Thread {
public:
	enum class Status {
		Run,
		Stop,
		RequestStop,
	};

public:
	Thread();
	virtual ~Thread();
	virtual void Start();
	virtual void Stop();
	bool IsStop();
	void Join();

	Status GetStatus() { return status_; }

protected:
	SDL_Thread* hThread_;
	SDL_threadID idThread_;
	volatile Status status_;
	virtual void _Run() = 0;

private:
	static int _StaticRun(void* data);
};

//================================================================
//CriticalSection
class CriticalSection {
public:
	CriticalSection();
	~CriticalSection();
	void Enter();
	void Leave();

private:
	SDL_mutex* cs_;
	volatile SDL_threadID idThread_;
	volatile int countLock_;
};

//================================================================
//Lock
class Lock {
public:
	Lock(CriticalSection& cs)
	{
		cs_ = &cs;
		cs_->Enter();
	}
	virtual ~Lock() { cs_->Leave(); }

protected:
	CriticalSection* cs_;
};

//================================================================
//ThreadSignal
class ThreadSignal {
public:
	ThreadSignal();
	virtual ~ThreadSignal();
	void Wait(Uint32 mills = SDL_MUTEX_MAXWAIT);
	void SetSignal(bool bLock);

private:
	SDL_cond* pCond_;
	SDL_mutex* pMutex_;
};

} // namespace gstd

#endif
