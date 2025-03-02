#pragma once

namespace MixedCallStackSampleClient
{
	struct StackFrame;

	class Interceptor final
	{
	public:
		static BOOL IsHelperProcess();
		static void AttachDetours();
		static void DetachDetours();

	private:
		static void ProcessLoadLibrary(const CString& libFileName, const std::deque<StackFrame>& callStack);
		static void ProcessFreeLibrary(HMODULE moduleHandle, const std::deque<StackFrame>& callStack);

	private:
		static HMODULE WINAPI OnLoadLibraryA(LPCSTR libFileName);
		static HMODULE WINAPI OnLoadLibraryW(LPCWSTR libFileName);
		static HMODULE WINAPI OnLoadLibraryExA(LPCSTR libFileName, HANDLE hFile, DWORD dwFlags);
		static HMODULE WINAPI OnLoadLibraryExW(LPCWSTR libFileName, HANDLE hFile, DWORD dwFlags);
		static BOOL WINAPI OnFreeLibrary(HMODULE hLibModule);

	private:
		inline static auto _procMutex = std::mutex();

	public:
		inline static HMODULE(WINAPI* RealLoadLibraryA)(LPCSTR) = LoadLibraryA;
		inline static HMODULE(WINAPI* RealLoadLibraryW)(LPCWSTR) = LoadLibraryW;
		inline static HMODULE(WINAPI* RealLoadLibraryExA)(LPCSTR, HANDLE, DWORD) = LoadLibraryExA;
		inline static HMODULE(WINAPI* RealLoadLibraryExW)(LPCWSTR, HANDLE, DWORD) = LoadLibraryExW;
		inline static BOOL(WINAPI* RealFreeLibrary)(HMODULE) = FreeLibrary;


	};

}
