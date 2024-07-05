#pragma once

namespace MixedCallStackSampleClient
{
	class NativeStackWalker final
	{
	public:
		static void Load();
		static void Unload();
		static bool IsCalledFromThisModule();
		static HRESULT AnnotateStack(const std::list<PVOID>& ipStack, std::list<CString>& annotationStack);
		static HRESULT AnnotateIP(const PVOID ip, CString& annotation);
		static DWORD64 GetNextEIP(HANDLE threadHandle, DWORD64 addrPC, DWORD64 addrFrame, DWORD64 addrStack, int skip);

	private:
		inline static HANDLE _processHandle = nullptr;
		inline static bool _isSymLoaded = false;

	};
}
