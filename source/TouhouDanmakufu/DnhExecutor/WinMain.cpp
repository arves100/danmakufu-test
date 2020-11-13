#include "GcLibImpl.hpp"

/**********************************************************
WinMain
**********************************************************/
int main(int argc, char* argv[])
{
	gstd::DebugUtility::DumpMemoryLeaksOnExit();
	try {
		DnhConfiguration* config = DnhConfiguration::CreateInstance();
		ELogger* logger = ELogger::CreateInstance();
		logger->Initialize(config->IsLogFile(), config->IsLogWindow());
		EPathProperty::CreateInstance();

		EApplication* app = EApplication::CreateInstance();
		app->Initialize();
		app->Run();
	} catch (std::exception& e) {
		std::wstring log = StringUtility::ConvertMultiToWide(e.what());
		Logger::WriteTop(log);
	} catch (gstd::wexception& e) {
		Logger::WriteTop(e.what());
	}
	// catch(...) {
	// 	Logger::WriteTop("不明なエラー");
	// }

	EApplication::DeleteInstance();
	EPathProperty::DeleteInstance();
	ELogger::DeleteInstance();
	DnhConfiguration::DeleteInstance();

	return 0;
}
