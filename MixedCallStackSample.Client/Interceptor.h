#pragma once

namespace MixedCallStackSampleClient
{
	class Interceptor final
	{
	public:
		static BOOL IsHelperProcess();
		static void AttachDetours();
		static void DetachDetours();

	private:
		static void ProcessLoadLibrary(const CString& dependencyName);
		static void ProcessFreeLibrary(HMODULE moduleHandle);

	private:
		static HMODULE WINAPI OnLoadLibraryA(LPCSTR libFileName);
		static HMODULE WINAPI OnLoadLibraryW(LPCWSTR libFileName);
		static HMODULE WINAPI OnLoadLibraryExA(LPCSTR libFileName, HANDLE hFile, DWORD dwFlags);
		static HMODULE WINAPI OnLoadLibraryExW(LPCWSTR libFileName, HANDLE hFile, DWORD dwFlags);
		static BOOL WINAPI OnFreeLibrary(HMODULE hLibModule);

	private:
		inline static auto _procMutex = std::mutex();

	};

}
