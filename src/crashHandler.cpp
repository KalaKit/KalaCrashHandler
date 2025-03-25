//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <filesystem>
#ifdef _WIN32
#include <Windows.h>
#include <dbghelp.h>
#include <shlwapi.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Shlwapi.lib")
#endif

#include "crashHandler.hpp"

using std::wstring;
using std::to_string;
using std::cout;
using std::replace;
using std::setfill;
using std::setw;
using std::time;
using std::tm;
using std::ofstream;
using std::filesystem::path;
#ifdef _WIN32
using std::wstring;
using std::hex;
using std::dec;
#endif

namespace KalaKit
{
	void KalaCrashHandler::Initialize()
	{
		//reserve emergency stack space (for stack overflow handling)
		ULONG stackSize = 32768; //32KB
		SetThreadStackGuarantee(&stackSize);

		SetUnhandledExceptionFilter(HandleCrash);
	}

#ifdef _WIN32
	LONG WINAPI KalaCrashHandler::HandleCrash(EXCEPTION_POINTERS* info)
	{
		DWORD code = info->ExceptionRecord->ExceptionCode;

		ostringstream oss;
		oss << "Crash detected!\n\n";

		oss << "Exception code: " << hex << code << "\n";
		oss << "Address: 0x" << hex << (uintptr_t)info->ExceptionRecord->ExceptionAddress << "\n\n";

		switch (code)
		{
			//
			// COMMON AND HIGH PRIORITY CRASH TYPES
			//

		case EXCEPTION_ACCESS_VIOLATION:
		{
			const ULONG_PTR* accessType = info->ExceptionRecord->ExceptionInformation;

			const char* accessStr = "unknown";
			switch (accessType[0])
			{
			case 0: accessStr = "read from"; break;
			case 1: accessStr = "write to"; break;
			case 8: accessStr = "execute"; break;
			}

			oss << "Reason: Access violation - attemted to " << accessStr
				<< " invalid memory at address 0x" << hex << accessType[1];

			if (accessType[0] == 8)
			{
				oss << "(possible code execution or exploit attempt)";
			}
			oss << "\n";
			break;
		}
		case EXCEPTION_STACK_OVERFLOW:
			oss << "Reason: Stack overflow (likely due to infinite recursion)\n";
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			oss << "Reason: Integer divide by zero\n";
			break;

			//
			// RARE BUT USEFUL CRASHES
			//

		case EXCEPTION_ILLEGAL_INSTRUCTION:
			oss << "Reason: Illegal CPU instruction executed\n";
			break;
		case EXCEPTION_BREAKPOINT:
			oss << "Reason: Breakpoint hit (INT 3 instruction executed)\n";
			break;
		case EXCEPTION_GUARD_PAGE:
			oss << "Reason: Guard page accessed (likely stack guard or memory protection violation)\n";
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			oss << "Reason: Privileged instruction executed in user mode\n";
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			oss << "Reason: Attempted to continue after a non-continuable exception (fatal logic error)\n";
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			oss << "Reason: Memory access failed (I/O or paging failure)\n";
			break;

		default:
			oss << "Reason: Unknown exception (code: 0x" << hex << code << ")\n";
			break;
		}

		AppendCallStackToStream(oss, info->ContextRecord);

		string exePath = GetExePath();
		string timeStamp = GetCurrentTimeStamp();

		if (createDump)
		{
			WriteMiniDump(info, exePath, timeStamp);
			oss << "A dump file '" << timeStamp << ".dmp" << "' was created at exe root folder.";
		}
		else cout << "[CRASH HANDLER] Dump file creation disabled by user.";

		WriteLog(oss.str(), exePath, timeStamp);
		CreateErrorPopup(oss.str());

		return EXCEPTION_EXECUTE_HANDLER;
	}

	void KalaCrashHandler::WriteLog(
		const string& message, 
		const string& exePath,
		const string& timeStamp)
	{
		string filePath = timeStamp + ".txt";
		string fullPath = (path(exePath) / filePath).string();
		ofstream logFile(fullPath);

		if (!logFile.is_open())
		{
			cout << "[CRASH HANDLER] Failed to open log file!\n";
			return;
		}

		logFile << message;

		logFile.close();
	}

	void KalaCrashHandler::WriteMiniDump(
		EXCEPTION_POINTERS* info, 
		const string& exePath, 
		const string& timeStamp)
	{
		string filePath = timeStamp + ".dmp";

		int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, exePath.c_str(), -1, nullptr, 0);
		wstring widePath(sizeNeeded - 1, 0); // -1 to exclude null terminator
		MultiByteToWideChar(CP_UTF8, 0, exePath.c_str(), -1, &widePath[0], sizeNeeded);

		//build full path to dump file
		widePath += L"\\" + wstring(filePath.begin(), filePath.end());

		HANDLE hFile = CreateFileW(
			widePath.c_str(),
			GENERIC_WRITE,
			0,
			nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION dumpInfo{};

			DWORD mdThreadID = GetThreadId(GetCurrentThread());
			dumpInfo.ThreadId = mdThreadID;
			cout << "[CRASH HANDLER] Minidump thread: " << mdThreadID << "\n";

			dumpInfo.ExceptionPointers = info;
			dumpInfo.ClientPointers = FALSE;

			MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				hFile,
				MiniDumpWithThreadInfo,
				&dumpInfo,
				nullptr,
				nullptr);

			CloseHandle(hFile);
		}
	}

	string KalaCrashHandler::GetCurrentTimeStamp()
	{
		time_t now = time(nullptr);
		tm localTime{};
		localtime_s(&localTime, &now);

		ostringstream oss;
		oss << setfill('0')
			<< localTime.tm_mday << "_"
			<< setw(2) << localTime.tm_hour << "_"
			<< setw(2) << localTime.tm_min << "_"
			<< setw(2) << localTime.tm_sec;

		string result = oss.str();

		string timeStamp = "crash_" + result;

		//replace characters not allowed in filenames
		replace(timeStamp.begin(), timeStamp.end(), ':', '-');
		timeStamp.erase(remove(timeStamp.begin(), timeStamp.end(), ']'), timeStamp.end());
		timeStamp.erase(remove(timeStamp.begin(), timeStamp.end(), '['), timeStamp.end());
		timeStamp.erase(remove(timeStamp.begin(), timeStamp.end(), ' '), timeStamp.end());

		return timeStamp;
	}

	string KalaCrashHandler::GetExePath()
	{
		//get executable directory
		WCHAR exePath[MAX_PATH];
		GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		PathRemoveFileSpecW(exePath);

		int sizeNeeded = WideCharToMultiByte(
			CP_UTF8,
			0,
			exePath,
			-1,
			nullptr,
			0,
			nullptr,
			nullptr);

		string exePathStr(sizeNeeded - 1, 0);

		WideCharToMultiByte(
			CP_UTF8,
			0,
			exePath,
			-1,
			&exePathStr[0],
			sizeNeeded,
			nullptr,
			nullptr);

		return exePathStr;
	}

	void KalaCrashHandler::AppendCallStackToStream(ostringstream& oss, CONTEXT* context)
	{
		HANDLE process = GetCurrentProcess();

		HANDLE thread = GetCurrentThread();
		DWORD mdThreadID = GetThreadId(thread);
		cout << "[CRASH HANDLER] Stackwalk thread: " << mdThreadID << "\n";

		SymSetOptions(
			SYMOPT_LOAD_LINES         //file/line info
			| SYMOPT_UNDNAME          //demangle c++ symbols
			| SYMOPT_DEFERRED_LOADS); //don't load all symbols immediately (faster)
		SymInitialize(process, nullptr, TRUE);

		STACKFRAME64 stack = {};
#if defined(_M_X64)
		DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
		stack.AddrPC.Offset = context->Rip;
		stack.AddrFrame.Offset = context->Rbp;
		stack.AddrStack.Offset = context->Rsp;
#else
		DWORD machineType = IMAGE_FILE_MACHINE_I386;
		stack.AddrPC.Offset = context->Eip;
		stack.AddrFrame.Offset = context->Ebp;
		stack.AddrStack.Offset = context->Esp;
#endif

		stack.AddrPC.Mode = AddrModeFlat;
		stack.AddrFrame.Mode = AddrModeFlat;
		stack.AddrStack.Mode = AddrModeFlat;

		oss << "\n========================================\n\n";
		oss << "Call stack:\n";

		for (int i = 0; i < 10; ++i)
		{
			if (!StackWalk64(
				machineType,
				process,
				thread,
				&stack,
				context,
				nullptr,
				SymFunctionTableAccess64,
				SymGetModuleBase64,
				nullptr))
			{
				break;
			}

			DWORD64 addr = stack.AddrPC.Offset;
			if (addr == 0) break;

			char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
			SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbol->MaxNameLen = MAX_SYM_NAME;

			oss << "  " << i << ": ";
			DWORD64 displacement = 0;
			if (SymFromAddr(
				process,
				addr,
				&displacement,
				symbol))
			{
				char demangled[1024];
				if (UnDecorateSymbolName(
					symbol->Name,
					demangled,
					sizeof(demangled),
					UNDNAME_COMPLETE))
				{
					oss << demangled;
				}
				else
				{
					oss << symbol->Name;
				}
			}
			else
			{
				oss << "(symbol not found)\n";
			}

			//file and line info
			IMAGEHLP_LINE64 lineInfo;
			DWORD lineDisplacement = 0;
			ZeroMemory(&lineInfo, sizeof(lineInfo));
			lineInfo.SizeOfStruct = sizeof(lineInfo);

			if (SymGetLineFromAddr64(
				process,
				addr,
				&lineDisplacement,
				&lineInfo))
			{
				string fullPath = lineInfo.FileName;
				size_t lastSlash = fullPath.find_last_of("\\/");
				size_t secondLastSlash = fullPath.find_last_of("\\/", lastSlash - 1);

				string shortPath = (secondLastSlash != string::npos)
					? fullPath.substr(secondLastSlash + 1)
					: fullPath;

				oss << "\n        script: " << shortPath;
				oss << "\n        line: " << dec << lineInfo.LineNumber;
			}

			oss << " [0x" << hex << addr << "]\n";
		}

		SymCleanup(process);

		oss << "\n========================================\n\n";
	}
#endif

	void KalaCrashHandler::CreateErrorPopup(const string& message)
	{
		string title = name + " has shut down";

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

		if (result == IDOK) Shutdown();
#elif __linux__
		string command = "zenity --error --text=\"" + (string)errorMessage + "\" --title=\"" + title + "\"";
		int result = system(command.c_str());
		(void)result;
		Shutdown();
#endif
	}
}
