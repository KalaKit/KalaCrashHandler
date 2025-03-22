//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <filesystem>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

//crash handler
#include "errorPopup.hpp"
#include "crashHandler.hpp"

using std::filesystem::path;
using std::cout;

namespace ElypsoUtils
{
	void ErrorPopup::CreateErrorPopup(const string& message)
	{
		string title = CrashHandler::GetProgramName() + " has shut down";

		cout << "\n"
			<< "===================="
			<< "\n"
			<< "PROGRAM SHUTDOWN\n"
			<< "\n\n"
			<< message
			<< "\n"
			<< "===================="
			<< "\n";

#ifdef _WIN32
		int result = MessageBoxA(nullptr, message.c_str(), title.c_str(), MB_ICONERROR | MB_OK);

		if (result == IDOK) CrashHandler::Shutdown();
#elif __linux__
		string command = "zenity --error --text=\"" + (string)errorMessage + "\" --title=\"" + title + "\"";
		int result = system(command.c_str());
		(void)result;
		CrashHandler::Shutdown();
#endif
	}
}