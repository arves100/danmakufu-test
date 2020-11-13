#ifndef __GSTD_WINDOW__
#define __GSTD_WINDOW__

#include "GstdConstant.hpp"
#include "GstdUtility.hpp"
#include "Thread.hpp"

namespace gstd {

/**********************************************************
//WindowBase
**********************************************************/
class WindowBase {
public:
	WindowBase();
	virtual ~WindowBase();
	SDL_Window* GetWindowHandle() { return hWnd_; }

	Uint32 GetWindowId() { return SDL_GetWindowID(hWnd_); }

	virtual void SetBounds(int x, int y, int width, int height)
	{
		SDL_SetWindowSize(hWnd_, width, height);
		SDL_SetWindowPosition(hWnd_, x, y);
	}

	int GetClientX()
	{
		int x, y;
		SDL_GetWindowPosition(hWnd_, &x, &y);
		return x;
	}
	int GetClientY()
	{
		int x, y;
		SDL_GetWindowPosition(hWnd_, &x, &y);
		return y;
	}
	int GetClientWidth()
	{
		int w, h;
		SDL_GetWindowSize(hWnd_, &w, &h);
		return w;
	}
	int GetClientHeight()
	{
		int w, h;
		SDL_GetWindowSize(hWnd_, &w, &h);
		return h;
	}

	void SetWindowVisible(bool bVisible)
	{
		if (bVisible)
			SDL_ShowWindow(hWnd_);
		else
			SDL_HideWindow(hWnd_);
	}

	bool IsWindowVisible() { return (SDL_GetWindowFlags(hWnd_) & SDL_WINDOW_HIDDEN) ? false : true; }

	Uint32 GetWindowFlags() { return SDL_GetWindowFlags(hWnd_); }

	virtual void LocateParts() {} //画面部品配置
	void MoveWindowCenter();

protected:
	SDL_Window* hWnd_;
	virtual void EventProcedure(SDL_Event* evt) {} //オーバーライド用プロシージャ
};

#if 0 // UI is not supported in SDL2
class WindowBase::Style {
public:
	Style()
	{
		style_ = 0;
		styleEx_ = 0;
	}
	virtual ~Style() {}
	DWORD GetStyle() { return style_; }
	DWORD SetStyle(DWORD style)
	{
		style_ |= style;
		return style_;
	}
	DWORD RemoveStyle(DWORD style)
	{
		style_ &= ~style;
		return style_;
	}

	DWORD GetStyleEx() { return styleEx_; }
	DWORD SetStyleEx(DWORD style)
	{
		styleEx_ |= style;
		return styleEx_;
	}
	DWORD RemoveStyleEx(DWORD style)
	{
		styleEx_ &= ~style;
		return styleEx_;
	}

protected:
	DWORD style_;
	DWORD styleEx_;
};

/**********************************************************
//ModalDialog
**********************************************************/
class ModalDialog : public WindowBase {
public:
	ModalDialog() { bEndDialog_ = false; }
	void Create(HWND hParent, LPCTSTR resource);

protected:
	HWND hParent_;
	bool bEndDialog_;
	void _RunMessageLoop();
	void _FinishMessageLoop();
};

/**********************************************************
//WPanel
**********************************************************/
class WPanel : public WindowBase {
public:
	void Create(HWND hWndParent);

protected:
	virtual LRESULT _WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //オーバーライド用プロシージャ
};

/**********************************************************
//WLabel
**********************************************************/
class WLabel : public WindowBase {
public:
	WLabel();
	void Create(HWND hWndParent);
	void SetText(std::wstring text);
	void SetTextColor(int color);
	void SetBackColor(int color);

	std::wstring GetText();
	int GetTextLength();

private:
	int colorText_;
	int colorBack_;
	virtual LRESULT _WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

/**********************************************************
//WButton
**********************************************************/
class WButton : public WindowBase {
public:
	class Style;

public:
	void Create(HWND hWndParent);
	void Create(HWND hWndParent, WButton::Style& style);
	void SetText(std::wstring text);
	bool IsChecked();
};
class WButton::Style : public WindowBase::Style {
};

/**********************************************************
//WGroupBox
**********************************************************/
class WGroupBox : public WindowBase {
public:
	void Create(HWND hWndParent);
	void SetText(std::wstring text);
};

/**********************************************************
//WEditBox
**********************************************************/
class WEditBox : public WindowBase {
public:
	class Style;

public:
	void Create(HWND hWndParent, WEditBox::Style& style);
	void SetText(std::wstring text);
	std::wstring GetText();
	int GetTextLength();
	int GetMaxTextLength() { return ::SendMessage(hWnd_, EM_GETLIMITTEXT, 0, 0); }
	void SetMaxTextLength(int length) { ::SendMessage(hWnd_, EM_SETLIMITTEXT, (WPARAM)length, 0); }
};
class WEditBox::Style : public WindowBase::Style {
};

/**********************************************************
//WListBox
**********************************************************/
class WListBox : public WindowBase {
public:
	class Style;

public:
	void Create(HWND hWndParent, WListBox::Style& style);

	void Clear();
	int GetCount();
	void SetSelectedIndex(int index);
	int GetSelectedIndex();
	void AddString(std::wstring str);
	void InsertString(int index, std::wstring str);
	void DeleteString(int index);
	std::wstring GetText(int index);
};
class WListBox::Style : public WindowBase::Style {
};

/**********************************************************
//WComboBox
**********************************************************/
class WComboBox : public WindowBase {
public:
	class Style;

public:
	void Create(HWND hWndParent, WComboBox::Style& style);

	void SetItemHeight(int height);
	void SetDropDownListWidth(int width);

	void Clear();
	int GetCount();
	void SetSelectedIndex(int index);
	int GetSelectedIndex();
	std::wstring GetSelectedText();
	void AddString(std::wstring str);
	void InsertString(int index, std::wstring str);
};
class WComboBox::Style : public WindowBase::Style {
};

/**********************************************************
//WListView
**********************************************************/
class WListView : public WindowBase {
public:
	class Style;

public:
	void Create(HWND hWndParent, Style& style);
	void Clear();
	void AddColumn(int cx, int sub, DWORD fmt, std::wstring text);
	void AddColumn(int cx, int sub, std::wstring text) { AddColumn(cx, sub, LVCFMT_LEFT, text); }
	void SetColumnText(int cx, std::wstring text);
	void AddRow(std::wstring text);
	void SetText(int row, int column, std::wstring text);
	void DeleteRow(int row);
	int GetRowCount();
	std::wstring GetText(int row, int column);
	bool IsExistsInColumn(std::wstring value, int column);
	int GetIndexInColumn(std::wstring value, int column);
	void SetSelectedRow(int index);
	int GetSelectedRow();
	void ClearSelection();
};
class WListView::Style : public WindowBase::Style {
public:
	Style() { styleListViewEx_ = 0; }
	DWORD GetListViewStyleEx() { return styleListViewEx_; }
	DWORD SetListViewStyleEx(DWORD style)
	{
		styleListViewEx_ |= style;
		return styleListViewEx_;
	}
	DWORD RemoveListViewStyleEx(DWORD style)
	{
		styleListViewEx_ &= ~style;
		return styleListViewEx_;
	}

protected:
	DWORD styleListViewEx_;
};

/**********************************************************
//WTreeView
**********************************************************/
class WTreeView : public WindowBase {
public:
	class Item;
	class ItemStyle;
	class Style;

	ref_count_ptr<Item> itemRoot_;

public:
	WTreeView();
	~WTreeView();
	void Create(HWND hWndParent, Style& style);
	void Clear()
	{
		itemRoot_ = NULL;
		TreeView_DeleteAllItems(hWnd_);
	}
	void CreateRootItem(ItemStyle& style);
	ref_count_ptr<Item> GetRootItem() { return itemRoot_; }
	ref_count_ptr<Item> GetSelectedItem();
};
class WTreeView::Item {
public:
	Item();
	virtual ~Item();
	ref_count_ptr<Item> CreateChild(WTreeView::ItemStyle& style);
	void Delete();
	void SetText(std::wstring text);
	std::wstring GetText();
	void SetParam(LPARAM param);
	LPARAM GetParam();
	std::list<ref_count_ptr<Item>> GetChildList();

private:
	friend WTreeView;
	HWND hTree_;
	HTREEITEM hItem_;
};
class WTreeView::ItemStyle {
public:
	ItemStyle()
	{
		ZeroMemory(&tvis_, sizeof(TVINSERTSTRUCT));
		tvis_.hInsertAfter = TVI_LAST;
	}
	TVINSERTSTRUCT& GetInsertStruct() { return tvis_; }
	void SetMask(UINT mask) { tvis_.item.mask |= mask; }

private:
	friend WTreeView;
	TVINSERTSTRUCT tvis_;
};
class WTreeView::Style : public WindowBase::Style {
};

/**********************************************************
//WTabControll
**********************************************************/
class WTabControll : public WindowBase {
public:
	~WTabControll();
	void Create(HWND hWndParent);
	void AddTab(std::wstring text);
	void AddTab(std::wstring text, ref_count_ptr<WPanel> panel);
	void ShowPage();
	int GetCurrentPage() { return TabCtrl_GetCurSel(hWnd_); }
	void SetCurrentPage(int page)
	{
		TabCtrl_SetCurSel(hWnd_, page);
		ShowPage();
	}
	ref_count_ptr<WPanel> GetPanel(int page) { return vectPanel_[page]; }
	int GetPageCount() { return vectPanel_.size(); }
	virtual void LocateParts();

protected:
	virtual LRESULT _WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	std::vector<ref_count_ptr<WPanel>> vectPanel_;
};

/**********************************************************
//WStatusBar
**********************************************************/
class WStatusBar : public WindowBase {
public:
	void Create(HWND hWndParent);
	void SetPartsSize(std::vector<int> parts);
	void SetText(int pos, std::wstring text);
	void SetBounds(WPARAM wParam, LPARAM lParam);
};

/**********************************************************
//WSplitter
**********************************************************/
class WSplitter : public WindowBase {
public:
	enum SplitType {
		TYPE_VERTICAL,
		TYPE_HORIZONTAL,
	};

public:
	WSplitter();
	~WSplitter();
	void Create(HWND hWndParent, SplitType type);
	void SetRatioX(float ratio) { ratioX_ = ratio; }
	float GetRatioX() { return ratioX_; }
	void SetRatioY(float ratio) { ratioY_ = ratio; }
	float GetRatioY() { return ratioY_; }

protected:
	SplitType type_;
	bool bCapture_;
	float ratioX_;
	float ratioY_;
	virtual LRESULT _WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif
/**********************************************************
//WindowUtility
**********************************************************/
class WindowUtility {
public:
	static void SetMouseVisible(bool bVisible);
};

} // namespace gstd

#endif
