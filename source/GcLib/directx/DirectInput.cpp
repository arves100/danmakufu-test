#include "DirectInput.hpp"

using namespace gstd;
using namespace directx;

#include <SDL_syswm.h>

/**********************************************************
// SDL2 Input (ex.: DirectInput)
**********************************************************/
DirectInput* DirectInput::thisBase_ = NULL;
DirectInput::DirectInput()
{
	memset(bufKey_, KEY_FREE, sizeof(bufKey_));
	memset(bufMouse_, KEY_FREE, sizeof(bufMouse_));
	lastMouseX_ = lastMouseY_ = 0;
	stateMouse_.lX = stateMouse_.lY = stateMouse_.lZ = 0;
}
DirectInput::~DirectInput()
{
	Logger::WriteTop(u8"DirectInput：終了開始");

	for (auto& j : pJoypad_)
		SDL_JoystickClose(j);

	for (auto& gc : pGamecontroller_)
		SDL_GameControllerClose(gc.ptr);

	thisBase_ = NULL;
	Logger::WriteTop(u8"DirectInput：終了完了");
}

bool DirectInput::Initialize()
{
	_InitializeJoypad();

	thisBase_ = this;
	Logger::WriteTop(u8"DirectInput：初期化完了");
	return true;
}

bool DirectInput::_InitializeJoypad()
{
	Logger::WriteTop(u8"DirectInput：ジョイパッド初期化");
	int count = SDL_NumJoysticks();

	if (count == 0) {
		Logger::WriteTop(u8"DirectInput：ジョイパッドは見つかりませんでした");
		return false; // ジョイパッドが見付からない
	}
	
	for (int i = 0; i < count; i++)
	{
		if (SDL_IsGameController(i))
		{
			SDL_GameController* gc = SDL_GameControllerOpen(i);
			if (!gc)
			{
				Logger::WriteTop(std::string("DirectInput: Cannot open game controller ") + SDL_GetError());
				continue;
			}

			GameController c;
			memset(c.axis, 0, sizeof(c.axis));
			memset(c.button, KEY_FREE, sizeof(c.button));
			c.ptr = gc;
			pGamecontroller_.push_back(c);
		}
		else
		{
#if 0 // § TODO ?
			SDL_Joystick* j = SDL_JoystickOpen(i);
			if (!j)
			{
				Logger::WriteTop(u8"DirectInput: Cannot open game controller " + StringUtility::ConvertMultiToWide(SDL_GetError()));
				continue;
			}

			pJoypad_.push_back(j);
#endif
		}
	}

	Logger::WriteTop(u8"DirectInput：ジョイパッド初期化完了");

	return true;
}

int DirectInput::_GetStateSub(bool flag, int state)
{
	int res = KEY_FREE;
	if (flag) {
		if (state == KEY_FREE)
			res = KEY_PUSH;
		else
			res = KEY_HOLD;
	}
	else {
		if (state == KEY_PUSH || state == KEY_HOLD)
			res = KEY_PULL;
		else
			res = KEY_FREE;
	}
	return res;
}

void DirectInput::EventUpdate(SDL_Event* evt)
{
	switch (evt->type)
	{
	case SDL_MOUSEWHEEL:
		stateMouse_.lZ = evt->wheel.y;
		break;

	case SDL_CONTROLLERDEVICEREMOVED:
	{
		std::vector<GameController>::iterator it = pGamecontroller_.begin();
		std::advance(it, evt->cdevice.which + 1);

		SDL_GameControllerClose((*it).ptr);
		pGamecontroller_.erase(it);
		break;
	}
	case SDL_CONTROLLERDEVICEADDED:
		if (pGamecontroller_.size() < (evt->cdevice.which + 1)) // Controller which starts from 0
		{
			SDL_GameController* gc = SDL_GameControllerOpen(evt->cdevice.which);
			if (gc)
			{
				GameController c;
				memset(c.axis, 0, sizeof(c.axis));
				memset(c.button, KEY_FREE, sizeof(c.button));
				c.ptr = gc;
				pGamecontroller_.push_back(c);
			}
		}
		break;
	default:
		break;
	}
}

void DirectInput::Update()
{
	auto currentKeys = SDL_GetKeyboardState(nullptr);

	for (int iKey = 0; iKey < SDL_NUM_SCANCODES; iKey++)
		bufKey_[iKey] = _GetStateSub(currentKeys[iKey], bufKey_[iKey]);

	int mouseState = SDL_GetMouseState(&lastMouseX_, &lastMouseY_);

	for (int iButton = 0; iButton < MOUSE_KEY_MAX; iButton++)
		// We use +1 here because MOUSE_KEY_LEFT in Danmakufu enum is 0, while SDL_BUTTON_LEFT is 1
		bufMouse_[iButton] = _GetStateSub(mouseState & SDL_BUTTON(iButton + 1), bufMouse_[iButton]);

	for (auto& pad : pGamecontroller_)
	{
		for (int axis = 0; axis < SDL_CONTROLLER_AXIS_MAX; axis++)
			pad.axis[axis] = SDL_GameControllerGetAxis(pad.ptr, (SDL_GameControllerAxis)axis);

		for (int iButton = 0; iButton < SDL_CONTROLLER_BUTTON_MAX; iButton++)
			pad.button[iButton] = _GetStateSub(SDL_GameControllerGetButton(pad.ptr, (SDL_GameControllerButton)iButton), pad.button[iButton]);
	}
}
int DirectInput::GetKeyState(int key)
{
	SDL_Scancode sc = SDL_GetScancodeFromKey(key);

	if (sc >= SDL_NUM_SCANCODES || sc < 0)
		return KEY_FREE;

	return bufKey_[sc];
}
int DirectInput::GetMouseState(int button)
{
	if (button < 0 || button >= MOUSE_KEY_MAX)
		return KEY_FREE;
	return bufMouse_[button];
}

int DirectInput::GetPadState(int padNo, int button)
{
	int res = KEY_FREE;
	if (padNo < pGamecontroller_.size())
		res = pGamecontroller_[padNo].button[button];
	return res;
}

POINT DirectInput::GetMousePosition()
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	return { x, y };
}

void DirectInput::ResetInputState()
{
	ResetMouseState();
	ResetKeyState();
	ResetPadState();
}
void DirectInput::ResetMouseState()
{
	for (int iButton = 0; iButton < MOUSE_KEY_MAX; iButton++)
		bufMouse_[iButton] = KEY_FREE;
}
void DirectInput::ResetKeyState()
{
	for (int iButton = 0; iButton < SDL_NUM_SCANCODES; iButton++)
		bufKey_[iButton] = KEY_FREE;
}
void DirectInput::ResetPadState()
{
	for (int iPad = 0; iPad < pGamecontroller_.size(); iPad++)
	{
		for (int iKey = 0; iKey < SDL_CONTROLLER_BUTTON_MAX; iKey++)
			pGamecontroller_[iPad].button[iKey] = KEY_FREE;
		for (int iAxis = 0; iAxis < SDL_CONTROLLER_AXIS_MAX; iAxis++)
			pGamecontroller_[iPad].axis[iAxis] = 0;
	}
}

/**********************************************************
//VirtualKeyManager
**********************************************************/
//VirtualKeyManager
VirtualKeyManager::VirtualKeyManager()
{
}
VirtualKeyManager::~VirtualKeyManager()
{
}

void VirtualKeyManager::Update()
{
	DirectInput::Update();

	auto itr = mapKey_.begin(), end = mapKey_.end();
	for (; itr != end; itr++) {
		int id = itr->first;
		auto& vk = itr->second;

		SDL_Scancode sc = SDL_GetScancodeFromKey(vk.button);

		if (sc >= 0 && sc < SDL_NUM_SCANCODES)
			vk.state = bufKey_[sc];
	}

	if (pGamecontroller_.size() > 0) // Do not iterate controller loop if there is no controller
	{
		auto itr2 = mapKeyG_.begin(), end2 = mapKeyG_.end();
		for (; itr2 != end2; itr2++)
		{
			int id = itr->first;
			auto& vk = itr2->second;

			if (vk.padId > -1 && vk.padId < pGamecontroller_.size())
			{
				if (!vk.bAxis &&
					vk.button >= 0 && vk.button < SDL_CONTROLLER_BUTTON_MAX)
					vk.state = pGamecontroller_[vk.padId].button[vk.button];
				else if (vk.bAxis &&
					vk.button > CONTROLLER_AXIS_OFFSET)
						vk.state = pGamecontroller_[vk.padId].axis[vk.button - CONTROLLER_AXIS_OFFSET];
			}
		}
	}
}
void VirtualKeyManager::ClearKeyState()
{
	DirectInput::ResetInputState();
	auto itr = mapKey_.begin(), end = mapKey_.end();
	for (; itr != end; itr++) {
		itr->second.state = KEY_FREE;
	}
	auto itr2 = mapKeyG_.begin(), end2 = mapKeyG_.end();
	for (; itr2 != end2; itr2++) {
		itr->second.state = KEY_FREE;
	}
}

void VirtualKeyManager::EventUpdate(SDL_Event* evt)
{
	DirectInput::EventUpdate(evt);
}

int VirtualKeyManager::GetVirtualKeyState(int id)
{
	if (mapKey_.find(id) == mapKey_.end())
		return KEY_FREE;

	int state = mapKey_[id].state;

	if (state == KEY_FREE)
		state = mapKeyG_[id].state;

	return state;
}

bool VirtualKeyManager::IsTargetKeyCode(int key)
{
	bool res = false;
	auto it = mapKey_.begin(), end = mapKey_.end();
	for (; it != end; it++)
	{
		auto vk = it->second;
		if (key == vk.button)
		{
			res = true;
			break;
		}
	}
	return res;
}

/**********************************************************
//KeyReplayManager
**********************************************************/
KeyReplayManager::KeyReplayManager(VirtualKeyManager* input)
{
	frame_ = 0;
	input_ = input;
	state_ = STATE_RECORD;
}
KeyReplayManager::~KeyReplayManager()
{
}
void KeyReplayManager::AddTarget(int key)
{
	listTarget_.push_back(key);
	mapLastKeyState_[key] = KEY_FREE;
}
void KeyReplayManager::Update()
{
	if (state_ == STATE_RECORD) {
		std::list<int>::iterator itrTarget = listTarget_.begin();
		for (; itrTarget != listTarget_.end(); itrTarget++) {
			int idKey = *itrTarget;
			int keyState = input_->GetVirtualKeyState(idKey);
			bool bInsert = (frame_ == 0 || mapLastKeyState_[idKey] != keyState);
			if (bInsert) {
				ReplayData data;
				data.id_ = idKey;
				data.frame_ = frame_;
				data.state_ = keyState;
				listReplayData_.push_back(data);
			}
			mapLastKeyState_[idKey] = keyState;
		}
	} else if (state_ == STATE_REPLAY) {
		std::list<int>::iterator itrTarget = listTarget_.begin();
		for (; itrTarget != listTarget_.end(); itrTarget++) {
			int idKey = *itrTarget;
			std::list<ReplayData>::iterator itrData = listReplayData_.begin();
			for (; itrData != listReplayData_.end();) {
				ReplayData data = *itrData;
				if (data.frame_ > frame_)
					break;

				if (idKey == data.id_ && data.frame_ == frame_) {
					mapLastKeyState_[idKey] = data.state_;
					itrData = listReplayData_.erase(itrData);
				} else {
					itrData++;
				}
			}

			int lastKeyState = mapLastKeyState_[idKey];
			input_->ForceSetKeyMap(idKey, lastKeyState);
		}
	}
	frame_++;
}
bool KeyReplayManager::IsTargetKeyCode(int key)
{
	return input_->IsTargetKeyCode(key);
}
void KeyReplayManager::ReadRecord(gstd::RecordBuffer& record)
{
	int countReplayData = record.GetRecordAsInteger("count");

	ByteBuffer buffer;
	buffer.SetSize(sizeof(ReplayData) * countReplayData);
	record.GetRecord("data", buffer.GetPointer(), buffer.GetSize());

	for (int iRec = 0; iRec < countReplayData; iRec++) {
		ReplayData data;
		buffer.Read(&data, sizeof(ReplayData));
		listReplayData_.push_back(data);
	}
}
void KeyReplayManager::WriteRecord(gstd::RecordBuffer& record)
{
	int countReplayData = listReplayData_.size();
	record.SetRecordAsInteger("count", countReplayData);

	ByteBuffer buffer;
	std::list<ReplayData>::iterator itrData = listReplayData_.begin();
	for (; itrData != listReplayData_.end(); itrData++) {
		ReplayData data = *itrData;
		buffer.Write(&data, sizeof(ReplayData));
	}

	record.SetRecord("data", buffer.GetPointer(), buffer.GetSize());
}
