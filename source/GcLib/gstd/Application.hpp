#ifndef __GSTD_APPLICATION__
#define __GSTD_APPLICATION__

#include "GstdConstant.hpp"
#include "GstdUtility.hpp"

namespace gstd {

/**********************************************************
//Application
**********************************************************/
class Application {
public:
	virtual ~Application();
	static Application* GetBase() { return thisBase_; }
	bool Initialize();

	virtual bool Run();
	bool IsActive() { return this->bAppActive_; }
	void SetActive(bool b) { this->bAppActive_ = b; }
	bool IsRun() { return bAppRun_; }
	void End() { bAppRun_ = false; }

protected:
	bool bAppRun_;
	bool bAppActive_;
	virtual bool _Initialize() { return true; }
	virtual bool _Loop() { return true; }
	virtual bool _Finalize() { return true; }
	virtual void OnEvent(SDL_Event*) {}
	Application();

private:
	static Application* thisBase_;
};

} // namespace gstd

#endif
