#ifndef __TOUHOUDANMAKUFU_EXE_COMMON__
#define __TOUHOUDANMAKUFU_EXE_COMMON__

#include "../Common/DnhCommon.hpp"
#include "../Common/DnhReplay.hpp"

#include "GcLibImpl.hpp"

/**********************************************************
//MenuTask
**********************************************************/
class MenuItem;
class MenuTask {
protected:
	enum {
		CURSOR_NONE = -1,
		CURSOR_LEFT = 0,
		CURSOR_RIGHT,
		CURSOR_UP,
		CURSOR_DOWN,
		CURSOR_COUNT = 4,
	};

public:
	MenuTask();
	virtual ~MenuTask();
	virtual void Clear();
	virtual void Work();
	virtual void Render();
	void SetActive(bool bActive) { bActive_ = bActive; }

	virtual void AddMenuItem(std::shared_ptr<MenuItem> item);

	int GetPageCount();
	int GetSelectedItemIndex();
	std::shared_ptr<MenuItem> GetSelectedMenuItem();

	int GetCurrentPageItemCount();
	int GetCurrentPageMaxX();
	int GetCurrentPageMaxY();

protected:
	int _GetCursorKeyState();
	virtual void _MoveCursor();
	virtual void _ChangePage(){};

	bool _IsWaitedKeyFree() { return bWaitedKeyFree_; }

	CriticalSection cs_;
	bool bActive_;
	std::vector<std::shared_ptr<MenuItem>> item_;

	std::vector<int> cursorFrame_;
	int cursorX_;
	int cursorY_;
	int pageCurrent_;
	int pageMaxX_;
	int pageMaxY_;
	bool bPageChangeX_;
	bool bPageChangeY_;
	bool bWaitedKeyFree_;
};
class MenuItem {
	friend MenuTask;

public:
	MenuItem() {}
	virtual ~MenuItem() {}
	virtual void Work() {}
	virtual void Render() {}

protected:
	MenuTask* menu_;
};
class TextLightUpMenuItem : public MenuItem {
public:
	TextLightUpMenuItem();

protected:
	void _WorkSelectedItem();
	int _GetSelectedItemAlpha();

	int frameSelected_;
};

#endif
