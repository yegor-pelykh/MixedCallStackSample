#include "pch.h"
#include "NativeStackWalker.h"
#include "ProcessInfo.hpp"

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

	PVOID NativeStackWalker::GetSymbolFromAddress(const CONTEXT& context)
	{
#ifdef _M_IX86
		const auto address = reinterpret_cast<PVOID>(context.Eip);
#elif _M_X64
		const auto address = reinterpret_cast<PVOID>(context.Rip);
#else
#error "Platform not supported!"
#endif

		return GetSymbolFromAddress(address);
	}

	PVOID NativeStackWalker::GetSymbolFromAddress(const PVOID address)
	{
		TCHAR buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		const auto symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		DWORD64 displacement = 0;
		const auto process = ProcessInfo::GetProcessHandle();
		return SymFromAddr(process, reinterpret_cast<DWORD64>(address), &displacement, symbol)
			? reinterpret_cast<PVOID>(symbol->Address)
			: nullptr;
	}

	std::deque<PVOID> NativeStackWalker::GetNativeCallStack(
		const FrameRegisters& registers,
		[[maybe_unused]] const std::function<bool(const PVOID)>& breakFunc
	)
	{
		std::deque<PVOID> frames;

#ifdef _M_IX86
		frames.push_back(registers.AddrPC);

		UINT_PTR frameAddress = reinterpret_cast<UINT_PTR>(registers.AddrFrame);
		const auto teb = reinterpret_cast<NT_TIB*>(::NtCurrentTeb());
		while (
			((frameAddress & 3) == 0) &&
			frameAddress + 2 * sizeof(VOID*) < reinterpret_cast<UINT_PTR>(teb->StackBase) &&
			frameAddress >= reinterpret_cast<UINT_PTR>(teb->StackLimit))
		{
			const auto returnAddress = reinterpret_cast<PVOID>(
				reinterpret_cast<UINT_PTR*>(frameAddress)[1]);
			if (returnAddress == nullptr || (breakFunc != nullptr && breakFunc(returnAddress)))
				break;

			frames.push_back(returnAddress);

			frameAddress = reinterpret_cast<UINT_PTR*>(frameAddress)[0];
		}
#elif  _M_X64
		CONTEXT context;
		registers.GetContext(context);

		DWORD64 ImageBase;
		PRUNTIME_FUNCTION runtimeFunction;
		while ((runtimeFunction = RtlLookupFunctionEntry(context.Rip, &ImageBase, nullptr)) != nullptr)
		{
			const auto returnAddress = reinterpret_cast<PVOID>(context.Rip);
			if (breakFunc != nullptr && breakFunc(returnAddress))
				break;

			frames.push_back(returnAddress);

			PVOID handlerData;
			ULONG64 establisherFrame;
			RtlVirtualUnwind(UNW_FLAG_NHANDLER, ImageBase, context.Rip,
				runtimeFunction, &context, &handlerData, &establisherFrame, nullptr);
		}
#endif

		return frames;
	}

	HRESULT NativeStackWalker::GetModuleBaseAddress(
		const PVOID address,
		DWORD64& moduleBaseAddress
	)
	{
		moduleBaseAddress = 0;

		if (!_isSymLoaded)
			return E_FAIL;

		moduleBaseAddress = SymGetModuleBase64(_processHandle, reinterpret_cast<DWORD64>(address));
		return moduleBaseAddress != 0 ? S_OK : E_FAIL;
	}

	HRESULT NativeStackWalker::GetModuleInfo(
		const DWORD64 moduleBaseAddress,
		IMAGEHLP_MODULE64& moduleInfo
	)
	{
		moduleInfo = { 0 };
		ZeroMemory(&moduleInfo, sizeof(moduleInfo));
		moduleInfo.SizeOfStruct = sizeof(moduleInfo);

		if (!_isSymLoaded)
			return E_FAIL;

		const BOOL hasModuleInfo = SymGetModuleInfo64(_processHandle, moduleBaseAddress, &moduleInfo);
		if (hasModuleInfo == FALSE)
			return E_FAIL;

		return S_OK;
	}

	HRESULT NativeStackWalker::GetAnnotation(
		const DWORD64 address,
		const CString& moduleName,
		CString& annotation
	)
	{
		annotation.Empty();

		if (!_isSymLoaded)
			return E_FAIL;

		char symbolBytes[sizeof(IMAGEHLP_SYMBOL64) + (NameBufferLength + 1) * sizeof(char)];
		ZeroMemory(symbolBytes, sizeof(symbolBytes));

		const auto symbolInfo = reinterpret_cast<PIMAGEHLP_SYMBOL64>(symbolBytes);
		symbolInfo->MaxNameLength = NameBufferLength;
		symbolInfo->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);

		DWORD64 displacement = 0;

		const BOOL hasSymbols = SymGetSymFromAddr64(_processHandle, address, &displacement, symbolInfo);
		if (!hasSymbols)
			return E_FAIL;

		if (moduleName.IsEmpty())
			annotation.Format(_T("%hs"), symbolInfo->Name);
		else
			annotation.Format(_T("%s!%hs"), moduleName.GetString(), symbolInfo->Name);

		return S_OK;
	}

}
