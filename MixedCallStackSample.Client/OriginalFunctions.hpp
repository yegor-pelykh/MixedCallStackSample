#pragma once

namespace MixedCallStackSampleClient
{
	class OriginalFunctions final
	{
	public:
		inline static HMODULE(WINAPI* RealLoadLibraryA)(LPCSTR)						= LoadLibraryA;
		inline static HMODULE(WINAPI* RealLoadLibraryW)(LPCWSTR)					= LoadLibraryW;
		inline static HMODULE(WINAPI* RealLoadLibraryExA)(LPCSTR, HANDLE, DWORD)	= LoadLibraryExA;
		inline static HMODULE(WINAPI* RealLoadLibraryExW)(LPCWSTR, HANDLE, DWORD)	= LoadLibraryExW;
		inline static BOOL(WINAPI* RealFreeLibrary)(HMODULE)						= FreeLibrary;

	};

}