#pragma once
#include "FrameRegisters.hpp"

namespace MixedCallStackSampleClient
{
	class NativeStackWalker final
	{
	public:
		static void Load();
		static void Unload();
		static PVOID GetSymbolFromAddress(const CONTEXT& context);
		static PVOID GetSymbolFromAddress(const PVOID address);
		static std::deque<PVOID> GetNativeCallStack(
			const FrameRegisters& registers,
			[[maybe_unused]] const std::function<bool(const PVOID)>& breakFunc = nullptr
		);
		static HRESULT GetModuleBaseAddress(
			const PVOID address,
			DWORD64& moduleBaseAddress
		);
		static HRESULT GetModuleInfo(
			const DWORD64 moduleBaseAddress,
			IMAGEHLP_MODULE64& moduleInfo
		);
		static HRESULT GetAnnotation(
			const DWORD64 address,
			const CString& moduleName,
			CString& annotation
		);

	private:
		inline static HANDLE _processHandle = nullptr;
		inline static bool _isSymLoaded = false;

	};
}
