#ifndef __DIRECTX_DIRECTINPUT__
#define __DIRECTX_DIRECTINPUT__

#include "DxConstant.hpp"

namespace directx {

enum {
	KEY_FREE = 0, // キーが押されていない状態
	KEY_PUSH = 1, // キーを押した瞬間
	KEY_PULL = 2, // キーが離された瞬間
	KEY_HOLD = 3, // キーが押されている状態

	CONTROLLER_AXIS_OFFSET = 100,

};

enum {
	MOUSE_KEY_LEFT, // SDL_BUTTON_LEFT - 1,
	MOUSE_KEY_MIDDLE, // SDL_BUTTON_MIDDLE - 1
	MOUSE_KEY_RIGHT, // SDL_BUTTON_RIGHT - 1
	MOUSE_KEY_X1, // SDL_BUTTON_X1 - 1
	MOUSE_KEY_X2, // SDL_BUTTON_X2 - 1
	MOUSE_KEY_MAX,
};

struct MouseState
{
	Sint32 lX, lY, lZ;
};

struct GameController
{
	SDL_GameController* ptr;
	Sint16 axis[SDL_CONTROLLER_AXIS_MAX];
	int button[SDL_CONTROLLER_BUTTON_MAX];
};

/**********************************************************
//DirectInput
**********************************************************/
class DirectInput {
	static DirectInput* thisBase_;

public:
public:
	DirectInput();
	virtual ~DirectInput();
	static DirectInput* GetBase() { return thisBase_; }

	virtual bool Initialize();

	virtual void Update(); //キーをセットする
	virtual void EventUpdate(SDL_Event* evt);
	int GetKeyState(int key);
	int GetMouseState(int button);
	int GetPadState(int padNo, int button);

	int GetMouseMoveX() { return stateMouse_.lX; } //マウスの移動量を取得X
	int GetMouseMoveY() { return stateMouse_.lY; } //マウスの移動量を取得Y
	int GetMouseMoveZ() { return stateMouse_.lZ; } //マウスの移動量を取得Z
	POINT GetMousePosition();

	void ResetInputState();
	void ResetMouseState();
	void ResetKeyState();
	void ResetPadState();

protected:
	std::vector<SDL_Joystick*> pJoypad_; // パッドデバイスオブジェクト
	std::vector<GameController> pGamecontroller_;
	MouseState stateMouse_;

	int lastMouseX_, lastMouseY_;

	int bufKey_[SDL_NUM_SCANCODES]; //現フレームのキーの状態
	int bufMouse_[MOUSE_KEY_MAX]; //現フレームのマウスの状態
	
	bool _InitializeJoypad();

	int _GetStateSub(bool flag, int state);
};

/**********************************************************
//VirtualKeyManager
**********************************************************/
class VirtualKeyManager;
class VirtualKey {
	friend VirtualKeyManager;

public:
	VirtualKey(int keyboard, int padIndex, int padButton);
	virtual ~VirtualKey();
	int GetKeyState() { return state_; }
	void SetKeyState(int state) { state_ = state; }

	int GetKeyCode() { return keyboard_; }
	void SetKeyCode(int code) { keyboard_ = code; }
	int GetPadIndex() { return padIndex_; }
	void SetPadIndex(int index) { padIndex_ = index; }
	int GetPadButton() { return padButton_; }
	void SetPadButton(int button) { padButton_ = button; }

private:
	int keyboard_; //キーボードのキー
	int padIndex_; //ジョイパッドの番号
	int padButton_; //ジョイパッドのボタン
	int state_; //現在の状態
};

class VirtualKeyManager : public DirectInput {
public:
	VirtualKeyManager();
	~VirtualKeyManager();
	virtual void Update(); //キーをセットする
	virtual void EventUpdate(SDL_Event* evt);
	void ClearKeyState();

	void AddKeyMap(int id, gstd::ref_count_ptr<VirtualKey> key) { mapKey_[id] = key; }
	void DeleteKeyMap(int id) { mapKey_.erase(id); };
	void ClearKeyMap() { mapKey_.clear(); };
	int GetVirtualKeyState(int id);
	gstd::ref_count_ptr<VirtualKey> GetVirtualKey(int id);
	bool IsTargetKeyCode(int key);

protected:
	std::map<int, gstd::ref_count_ptr<VirtualKey>> mapKey_;
	int _GetVirtualKeyState(int id);
};

/**********************************************************
//KeyReplayManager
**********************************************************/
class KeyReplayManager {
public:
	enum {
		STATE_RECORD,
		STATE_REPLAY,
	};

public:
	KeyReplayManager(VirtualKeyManager* input);
	virtual ~KeyReplayManager();
	void SetManageState(int state) { state_ = state; }
	void AddTarget(int key);
	bool IsTargetKeyCode(int key);

	void Update();
	void ReadRecord(gstd::RecordBuffer& record);
	void WriteRecord(gstd::RecordBuffer& record);

protected:
	struct ReplayData {
		int id_;
		int frame_;
		int state_;
	};

	int state_;
	int frame_;
	std::map<int, int> mapLastKeyState_;
	std::list<int> listTarget_;
	std::list<ReplayData> listReplayData_;
	VirtualKeyManager* input_;
};

} // namespace directx

#endif
