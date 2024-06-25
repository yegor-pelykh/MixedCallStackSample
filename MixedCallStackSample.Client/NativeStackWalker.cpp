#include "pch.h"
#include "NativeStackWalker.h"
#include "ProcessInfo.hpp"

constexpr DWORD TraceMaxStackFrames = 50;
constexpr DWORD NameBufferLength = 512;

namespace MixedCallStackSampleClient
{
	void NativeStackWalker::Load()
	{
		_processHandle = ProcessInfo::GetProcessHandle();

		_isSymLoaded = SymInitialize(_processHandle, nullptr, TRUE);
	}

	void NativeStackWalker::Unload()
	{
		SymCleanup(_processHandle);
	}

	bool NativeStackWalker::IsCalledFromThisModule()
	{
		PVOID stack[TraceMaxStackFrames];
		const WORD numberOfFrames = CaptureStackBackTrace(0, TraceMaxStackFrames, stack, nullptr);
		if (numberOfFrames <= 2)
			return false;

		BOOL success = FALSE;

		HMODULE currentModuleHandle = nullptr;
		success = GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			static_cast<LPCTSTR>(stack[0]), &currentModuleHandle);
		if (!success || currentModuleHandle == nullptr)
			return false;

		HMODULE callerModuleHandle = nullptr;
		success = GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			static_cast<LPCTSTR>(stack[2]), &callerModuleHandle);
		if (!success || callerModuleHandle == nullptr)
			return false;

		return callerModuleHandle == currentModuleHandle;
	}

	HRESULT NativeStackWalker::AnnotateStack(const std::list<DWORD64>& ipStack, std::list<CString>& annotationStack)
	{
		if (!_isSymLoaded)
			return S_FALSE;

		annotationStack.clear();

		for (auto it = ipStack.begin(); it != ipStack.end(); ++it)
		{
			const DWORD64 ip = *it;

			CString annotation;
			AnnotateIP(ip, annotation);

			annotationStack.push_back(annotation);
		}

		return S_OK;
	}

	HRESULT NativeStackWalker::AnnotateIP(const DWORD64 ip, CString& annotation)
	{
		annotation.Empty();

		if (!_isSymLoaded)
			return S_FALSE;

		char symbolBytes[sizeof(IMAGEHLP_SYMBOL64) + (NameBufferLength + 1) * sizeof(char)];
		ZeroMemory(symbolBytes, sizeof(symbolBytes));

		const auto symbolInfo = reinterpret_cast<PIMAGEHLP_SYMBOL64>(symbolBytes);
		symbolInfo->MaxNameLength = NameBufferLength;
		symbolInfo->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);

		DWORD64 displacement = 0;

		DWORD64 moduleBaseAddress = SymGetModuleBase64(_processHandle, ip);
		if (moduleBaseAddress == 0)
			return S_FALSE;

		IMAGEHLP_MODULE64 moduleInfo = { 0 };
		ZeroMemory(&moduleInfo, sizeof(moduleInfo));
		moduleInfo.SizeOfStruct = sizeof(moduleInfo);

		const BOOL isModuleInfoSuccess = SymGetModuleInfo64(_processHandle, moduleBaseAddress, &moduleInfo);

		CString moduleName = isModuleInfoSuccess
			? CString(moduleInfo.ModuleName)
			: CString("UnknownModule");

		BOOL isSymFromAddrSuccess = SymGetSymFromAddr64(_processHandle, ip, &displacement, symbolInfo);
		if (!isSymFromAddrSuccess)
			return S_FALSE;

		IMAGEHLP_LINE64 line = { 0 };

		DWORD disp = static_cast<DWORD>(displacement);
		const BOOL isLineFromAddrSuccess = SymGetLineFromAddr64(_processHandle, ip, &disp, &line);
		if (isLineFromAddrSuccess)
		{
			CString buf;
			buf.AppendFormat(_T("%s!%hs(%i) : 0x%08I64X (%llu)"),
				moduleName.GetString(), symbolInfo->Name, line.LineNumber, ip, ip);
			annotation = buf;
		}
		else
		{
			CString buf;
			buf.AppendFormat(_T("%s!%hs(UnknownLine) : 0x%08I64X (%llu)"),
				moduleName.GetString(), symbolInfo->Name, ip, ip);
			annotation = buf;
		}

		return S_OK;
	}

	DWORD64 NativeStackWalker::GetNextEIP(
		const HANDLE threadHandle,
		const DWORD64 addrPC,
		const DWORD64 addrFrame,
		const DWORD64 addrStack,
		int skip
	)
	{
		if (!_isSymLoaded)
			return 0;

		if (threadHandle == INVALID_HANDLE_VALUE)
			return 0;

		DWORD image;
        STACKFRAME64 stackFrame;
        ZeroMemory(&stackFrame, sizeof(STACKFRAME64));

#ifdef _M_IX86
		image = IMAGE_FILE_MACHINE_I386;
		stackFrame.AddrPC.Offset = addrPC;
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = addrFrame;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = addrStack;
		stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
		image = IMAGE_FILE_MACHINE_AMD64;
		stackFrame.AddrPC.Offset = addrPC;
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = addrFrame;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = addrStack;
		stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
		image = IMAGE_FILE_MACHINE_IA64;
		stackFrame.AddrPC.Offset = addrPC;
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = addrFrame;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrBStore.Offset = addrStack;
		stackFrame.AddrBStore.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = addrFrame;
		stackFrame.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

		CONTEXT context = { 0 };

#ifdef _M_IX86
		context.Eip = static_cast<DWORD>(addrPC);
		context.Ebp = static_cast<DWORD>(addrFrame);
		context.Esp = static_cast<DWORD>(addrStack);
#elif _M_X64
		context.Rip = addrPC;
		context.Rbp = addrFrame;
		context.Rsp = addrStack;
#elif _M_IA64
		context.StIIP = addrPC;
		context.IntSp = addrFrame;
		context.RsBSP = addrStack;
#else
#error "Platform not supported!"
#endif

		while (StackWalk64(image, _processHandle, threadHandle, &stackFrame, &context,
			nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
		{
			if (skip > 0)
				--skip;
			else
				return stackFrame.AddrPC.Offset;
		}
		return 0;
	}
}
