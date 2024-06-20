#include "pch.h"
#include "Interceptor.h"
#include "CorProfiler.h"
#include "GlobalData.h"
#include "NativeStackWalker.h"
#include "OriginalFunctions.hpp"

namespace MixedCallStackSampleClient
{
	BOOL Interceptor::IsHelperProcess()
	{
        return DetourIsHelperProcess();
	}

    void Interceptor::AttachDetours()
    {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealLoadLibraryA), OnLoadLibraryA);
        DetourAttach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealLoadLibraryW), OnLoadLibraryW);
        DetourAttach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealLoadLibraryExA), OnLoadLibraryExA);
        DetourAttach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealLoadLibraryExW), OnLoadLibraryExW);
        DetourAttach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealFreeLibrary), OnFreeLibrary);
        DetourTransactionCommit();
    }

    void Interceptor::DetachDetours()
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealLoadLibraryA), OnLoadLibraryA);
        DetourDetach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealLoadLibraryW), OnLoadLibraryW);
        DetourDetach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealLoadLibraryExA), OnLoadLibraryExA);
        DetourDetach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealLoadLibraryExW), OnLoadLibraryExW);
        DetourDetach(&reinterpret_cast<PVOID&>(OriginalFunctions::RealFreeLibrary), OnFreeLibrary);
        DetourTransactionCommit();
    }

    void Interceptor::ProcessLoadLibrary(const CString& libFileName)
    {
        const auto corProfiler = GlobalData::GetCorProfiler();
        if (corProfiler == nullptr)
            return;

        CONTEXT context = { 0 };
        context.ContextFlags = CONTEXT_FULL;
        RtlCaptureContext(&context);

        corProfiler->GetMixedCallStack(&context);
    }

    void Interceptor::ProcessFreeLibrary(HMODULE moduleHandle)
	{
        const auto corProfiler = GlobalData::GetCorProfiler();
        if (corProfiler == nullptr)
            return;

        CONTEXT context = { 0 };
        context.ContextFlags = CONTEXT_FULL;
        RtlCaptureContext(&context);

        corProfiler->GetMixedCallStack(&context);
	}
    
    HMODULE WINAPI Interceptor::OnLoadLibraryA(LPCSTR libFileName)
    {
        if (NativeStackWalker::IsCalledFromThisModule())
            return OriginalFunctions::RealLoadLibraryA(libFileName);

        const CString libName = libFileName;
        ProcessLoadLibrary(libName);

        return OriginalFunctions::RealLoadLibraryA(libFileName);
    }

    HMODULE WINAPI Interceptor::OnLoadLibraryW(LPCWSTR libFileName)
    {
        if (NativeStackWalker::IsCalledFromThisModule())
            return OriginalFunctions::RealLoadLibraryW(libFileName);

        const CString libName = libFileName;
        ProcessLoadLibrary(libName);

        return OriginalFunctions::RealLoadLibraryW(libFileName);
    }

    HMODULE WINAPI Interceptor::OnLoadLibraryExA(LPCSTR libFileName, HANDLE hFile, DWORD dwFlags)
    {
        if (NativeStackWalker::IsCalledFromThisModule())
            return OriginalFunctions::RealLoadLibraryExA(libFileName, hFile, dwFlags);

        const CString libName = libFileName;
        ProcessLoadLibrary(libName);

        return OriginalFunctions::RealLoadLibraryExA(libFileName, hFile, dwFlags);
    }

    HMODULE WINAPI Interceptor::OnLoadLibraryExW(LPCWSTR libFileName, HANDLE hFile, DWORD dwFlags)
    {
        if (NativeStackWalker::IsCalledFromThisModule())
            return OriginalFunctions::RealLoadLibraryExW(libFileName, hFile, dwFlags);

        const CString libName = libFileName;
        ProcessLoadLibrary(libName);

        return OriginalFunctions::RealLoadLibraryExW(libFileName, hFile, dwFlags);
    }

    BOOL WINAPI Interceptor::OnFreeLibrary(HMODULE hLibModule)
    {
        if (NativeStackWalker::IsCalledFromThisModule())
            return OriginalFunctions::RealFreeLibrary(hLibModule);

        ProcessFreeLibrary(hLibModule);

        return OriginalFunctions::RealFreeLibrary(hLibModule);
    }

}
