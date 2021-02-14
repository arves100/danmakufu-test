#include "Application.hpp"
#include "Logger.hpp"

using namespace gstd;

/**********************************************************
//Application
**********************************************************/
Application* Application::thisBase_ = nullptr;
Application::Application() : bAppActive_(false), bAppRun_(false)
{
#ifdef _WIN32
	::InitCommonControls();
#endif
}
Application::~Application()
{
	SDL_Quit();

	thisBase_ = nullptr;
}
bool Application::Initialize()
{
	if (thisBase_ != nullptr)
		return false;
	thisBase_ = this;
	bAppRun_ = true;
	bAppActive_ = true;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
		return false;

	return true;
}
bool Application::Run()
{
	if (bAppRun_ == false) {
		return false;
	}

	try {
		bool res = _Initialize();
		if (res == false)
			throw std::runtime_error(u8"初期化中に例外が発生しました。");
	} catch (std::exception& e) {
		Logger::WriteTop(e.what());
		Logger::WriteTop(u8"初期化中に例外が発生しました。強制終了します。");
		bAppRun_ = false;
	} catch (...) {
		Logger::WriteTop(u8"初期化中に例外が発生しました。強制終了します。");
		bAppRun_ = false;
	}

	SDL_Event evt;
	while (true) {
		if (bAppRun_ == false)
			break;
		if (SDL_PollEvent(&evt)) {
			OnEvent(&evt);
		} else {
			if (bAppActive_ == false) {
#ifdef _WIN32
				Sleep(10);
#else
				sleep(10);
#endif
				continue;
			}
			try {
				if (_Loop() == false)
					break;
			}
			catch (std::exception& e) {
				Logger::WriteTop(e.what());
				Logger::WriteTop(u8"実行中に例外が発生しました。終了します。");
				break;
			}
			// catch(...)
			// {
			// 	Logger::WriteTop(u8"実行中に例外が発生しました。終了します。");
			// 	break;
			// }
		}
	}

	bAppRun_ = false;

	try {
		bool res = _Finalize();
		if (res == false)
			throw std::runtime_error(u8"終了中に例外が発生しました。");
	} catch (std::exception& e) {
		Logger::WriteTop(e.what());
		Logger::WriteTop(u8"正常に終了できませんでした。");
	} catch (...) {
		Logger::WriteTop(u8"正常に終了できませんでした。");
	}
	return true;
}
