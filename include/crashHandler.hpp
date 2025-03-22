//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#ifdef _WIN32
	#ifdef CRASHHANDLER_DLL_EXPORT
		#define CRASHHANDLER_API __declspec(dllexport)
	#else
		#define CRASHHANDLER_API __declspec(dllimport)
	#endif
#else
	#define CRASHHANDLER_API
#endif

#include <string>
#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace ElypsoUtils
{
	using std::string;
	using std::ostringstream;

	class CRASHHANDLER_API CrashHandler
	{
	public:
#ifdef _WIN32
		/// <summary>
		/// Windows crash handler that calls the minidump creator function and sends error info to error popup.
		/// </summary>
		static LONG WINAPI HandleCrash(EXCEPTION_POINTERS* info);
#endif

		static string GetCurrentTimeStamp();
	private:
#ifdef _WIN32
		/// <summary>
		/// Creates a windows crash .dmp file to exe location.
		/// </summary>
		static string WriteMiniDump(EXCEPTION_POINTERS* info);

		/// <summary>
		/// Appends up to to last 10 frames of the call stack upon crash.
		/// </summary>
		static void AppendCallStackToStream(ostringstream& oss, CONTEXT* context);
#endif
	};
}