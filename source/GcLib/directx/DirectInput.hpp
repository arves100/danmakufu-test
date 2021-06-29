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
class DirectInput
{
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

private:
	static DirectInput* thisBase_;

};

/**********************************************************
//VirtualKeyManager
**********************************************************/
struct VirtualKey {
	VirtualKey() : button(0), state(KEY_FREE) {}
	VirtualKey(int b) : button(b), state(KEY_FREE) {}

	int state;
	int button;
};

struct GVirtualKey {
	GVirtualKey() : padId(0), button(0), bAxis(0), state(KEY_FREE) {}
	GVirtualKey(int id, int b, bool ax) : padId(id), button(b), bAxis(ax), state(KEY_FREE) {}

	int state;
	int button;
	bool bAxis;
	int padId;
};

class VirtualKeyManager : public DirectInput {
public:
	VirtualKeyManager();
	~VirtualKeyManager();
	virtual void Update(); //キーをセットする
	virtual void EventUpdate(SDL_Event* evt);
	void ClearKeyState();

	void AddKeyMap(int id, int button) { mapKey_.insert(std::make_pair(id, VirtualKey(button))); }
	void AddGKeyMap(int id, int button, int padId, bool bAxis) { mapKeyG_.insert(std::make_pair(id, GVirtualKey(padId, button, bAxis))); }
	void DeleteKeyMap(int id) { mapKey_.erase(id); mapKeyG_.erase(id); };
	void ClearKeyMap() { mapKey_.clear(); mapKeyG_.clear(); };
	int GetVirtualKeyState(int id);
	bool IsTargetKeyCode(int key);
	bool HaveKeyMap(int id) { return mapKey_.count(id) > 0;  }

	// Only used for replay mode!
	void ForceSetKeyMap(int id, int state) { mapKey_[id].state = state; }

protected:
	std::map<int, VirtualKey> mapKey_;
	std::map<int, GVirtualKey> mapKeyG_; // GameController
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
