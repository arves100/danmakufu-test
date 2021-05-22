#ifndef __DIRECTX_DXWINDOW__
#define __DIRECTX_DXWINDOW__

#include "DxText.hpp"
#include "DxUtility.hpp"
#include "RenderObject.hpp"

namespace directx {

class DxWindowManager;
class DxWindow;
class DxWindowEvent;

/**********************************************************
//DxWindowEvent
**********************************************************/
class DxWindowEvent {
	friend DxWindowManager;

public:
	enum {
		TYPE_MOUSE_LEFT_PUSH = 1 << 0,
		TYPE_MOUSE_LEFT_RELEASE = 1 << 1,
		TYPE_MOUSE_LEFT_CLICK = 1 << 2,
		TYPE_MOUSE_LEFT_HOLD = 1 << 3,

		TYPE_MOUSE_RIGHT_PUSH = 1 << 4,
		TYPE_MOUSE_RIGHT_RELEASE = 1 << 5,
		TYPE_MOUSE_RIGHT_CLICK = 1 << 6,
		TYPE_MOUSE_RIGHT_HOLD = 1 << 7,
	};

public:
	DxWindowEvent() { type_ = 0; };
	virtual ~DxWindowEvent(){};
	void SetSourceWindow(std::shared_ptr<DxWindow> source) { windowSource_ = source; }
	std::shared_ptr<DxWindow> GetSourceWindow() { return windowSource_; }
	void AddEventType(int type) { type_ |= type; }
	bool HasEventType(int type) { return (type_ & type) != 0; }
	bool IsEmpty() { return type_ == 0; }

protected:
	std::shared_ptr<DxWindow> windowSource_; //ウィンドウ
	int type_;
};

/**********************************************************
//DxWindowManager
**********************************************************/
class DxWindowManager : public gstd::TaskBase {
public:
	DxWindowManager();
	virtual ~DxWindowManager();
	void Clear();

	void AddWindow(std::shared_ptr<DxWindow> window);
	void DeleteWindow(DxWindow* window);
	void DeleteWindowFromID(int id);

	virtual void Work();
	virtual void Render();

	void SetAllWindowEnable(bool bEnable);
	void SetWindowEnableWithoutArgumentWindow(bool bEnable, DxWindow* window);

	std::shared_ptr<DxWindow> GetIntersectedWindow();
	std::shared_ptr<DxWindow> GetIntersectedWindow(POINT& pos, std::shared_ptr<DxWindow> parent = NULL);

protected:
	std::list<std::shared_ptr<DxWindow>> listWindow_; //最前面がアクティブ
	std::shared_ptr<DxWindow> wndCapture_;
	std::list<int> listLockID_;

	void _ArrangeWindow(); //必要のなくなった領域削除
	virtual void _DispatchMouseEvent();
};

/**********************************************************
//DxWindow
**********************************************************/
class DxWindow {
	friend DxWindowManager;

public:
	DxWindow();
	virtual ~DxWindow();
	virtual void DeleteWindow(); //削除フラグを立てます
	bool IsWindowDelete() { return bWindowDelete_; }
	void Dispose(); //各参照などを解放します
	virtual void AddedManager() {}
	void AddChild(std::shared_ptr<DxWindow> window);
	virtual void Work() { _WorkChild(); }
	virtual void Render()
	{
		_RenderFrame();
		_RenderChild();
	}
	virtual void DispatchedEvent(std::shared_ptr<DxWindowEvent> event) { _DispatchEventToChild(event); }

	virtual void IntersectMouseCursor() {}

	int GetID() { return idWindow_; }
	virtual bool IsIntersected(POINT pos);
	void SetWindowRect(RECT rect) { rectWindow_ = rect; }
	RECT GetWindowRect() { return rectWindow_; }
	RECT GetAbsoluteWindowRect();
	virtual void SetWindowEnable(bool bEnable) { bWindowEnable_ = bEnable; }
	virtual bool IsWindowEnable() { return bWindowEnable_; }
	virtual void SetWindowVisible(bool bVisible) { bWindowVisible_ = bVisible; }
	virtual bool IsWindowVisible() { return bWindowVisible_; }
	virtual bool IsWindowExists(int id);

	void SetAlpha(int alpha) { ColorAccess::SetColorA(color_, alpha); }
	int GetAlpha() { return ColorAccess::GetColorA(color_); }
	int GetAbsoluteAlpha();

	void SetFrameSprite(std::shared_ptr<Sprite2D> sprite) { spriteFrame_ = sprite; }

protected:
	DxWindowManager* manager_;
	int idWindow_;
	bool bWindowDelete_; //削除フラグ
	bool bWindowEnable_;
	bool bWindowVisible_;
	RECT rectWindow_; //ウィンドウ相対座標
	DxWindow* windowParent_; //親ウィンドウ
	std::list<std::shared_ptr<DxWindow>> listWindowChild_; //子ウィンドウ

	D3DCOLOR color_;
	std::shared_ptr<Sprite2D> spriteFrame_;
	int typeRenderFrame_;

	void _WorkChild();
	void _RenderChild();
	void _DispatchEventToChild(std::shared_ptr<DxWindowEvent> event);
	virtual void _RenderFrame();

private:
	static std::list<int> listWndId_;
};

/**********************************************************
//DxLabel
**********************************************************/
class DxLabel : public DxWindow {
public:
	DxLabel();
	virtual void Work();
	virtual void Render();
	void SetText(std::string str);
	void SetText(std::shared_ptr<DxText> text, bool bArrange = false);

protected:
	std::shared_ptr<DxText> text_;
};

/**********************************************************
//DxButton
**********************************************************/
class DxButton : public DxLabel {
public:
	DxButton();
	virtual void Work();
	virtual void Render();
	virtual void RenderIntersectedFrame();
	virtual void RenderSelectedFrame();

	bool IsIntersected() { return bIntersected_; }
	bool IsSelected() { return bSelected_; }
	void SetSelected(bool bSelected) { bSelected_ = bSelected; }

protected:
	bool bIntersected_;
	bool bSelected_;
};

/**********************************************************
//DxMessageBox
**********************************************************/
class DxMessageBox : public DxWindow {
public:
	enum {
		INDEX_NULL = -1,
	};

public:
	DxMessageBox();
	virtual void DispatchedEvent(std::shared_ptr<DxWindowEvent> event);
	void SetText(std::shared_ptr<DxText> text);
	void SetButton(std::vector<std::shared_ptr<DxButton>> listButton);
	int GetSelectedIndex() { return index_; }
	void UpdateWindowRect();

protected:
	int index_;
	std::shared_ptr<DxText> text_;
	std::vector<std::shared_ptr<DxButton>> listButton_;
};

} // namespace directx

#endif
