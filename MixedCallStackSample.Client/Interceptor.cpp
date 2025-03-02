#include "pch.h"
#include "Interceptor.h"
#include "NativeStackWalker.h"
#include "ProcessInfo.hpp"
#include "RecursionGuard.hpp"
#include "StackWalker.h"

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
        DetourAttach(&reinterpret_cast<PVOID&>(RealLoadLibraryA), OnLoadLibraryA);
        DetourAttach(&reinterpret_cast<PVOID&>(RealLoadLibraryW), OnLoadLibraryW);
        DetourAttach(&reinterpret_cast<PVOID&>(RealLoadLibraryExA), OnLoadLibraryExA);
        DetourAttach(&reinterpret_cast<PVOID&>(RealLoadLibraryExW), OnLoadLibraryExW);
        DetourAttach(&reinterpret_cast<PVOID&>(RealFreeLibrary), OnFreeLibrary);
        DetourTransactionCommit();
    }

    void Interceptor::DetachDetours()
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&reinterpret_cast<PVOID&>(RealLoadLibraryA), OnLoadLibraryA);
        DetourDetach(&reinterpret_cast<PVOID&>(RealLoadLibraryW), OnLoadLibraryW);
        DetourDetach(&reinterpret_cast<PVOID&>(RealLoadLibraryExA), OnLoadLibraryExA);
        DetourDetach(&reinterpret_cast<PVOID&>(RealLoadLibraryExW), OnLoadLibraryExW);
        DetourDetach(&reinterpret_cast<PVOID&>(RealFreeLibrary), OnFreeLibrary);
        DetourTransactionCommit();
    }

    void Interceptor::ProcessLoadLibrary(const CString& libFileName, const std::deque<StackFrame>& callStack)
    {
        std::lock_guard procLock(_procMutex);

        // ...
    }

    void Interceptor::ProcessFreeLibrary(HMODULE moduleHandle, const std::deque<StackFrame>& callStack)
	{
        std::lock_guard procLock(_procMutex);

        // ...
	}
    
    HMODULE WINAPI Interceptor::OnLoadLibraryA(const LPCSTR libFileName)
    {
        if (const RecursionGuard recursionGuard; recursionGuard.IsInitiator())
        {
            if (!ProcessInfo::IsNETServiceThread())
            {
                CONTEXT context = { .ContextFlags = CONTEXT_CONTROL };
                RtlCaptureContext(&context);

                const auto addressToTrim = NativeStackWalker::GetSymbolFromAddress(context);
                const auto callStack = StackWalker::GetCallStackEx(addressToTrim);

                if (!StackWalker::IsCalledFromThisModule(callStack))
                {
                    const CString libName = libFileName;
                    ProcessLoadLibrary(libName, callStack);
				}
			}
        }

        return RealLoadLibraryA(libFileName);
    }

    HMODULE WINAPI Interceptor::OnLoadLibraryW(const LPCWSTR libFileName)
    {
        if (const RecursionGuard recursionGuard; recursionGuard.IsInitiator())
        {
            if (!ProcessInfo::IsNETServiceThread())
            {
                CONTEXT context = { .ContextFlags = CONTEXT_CONTROL };
                RtlCaptureContext(&context);

                const auto addressToTrim = NativeStackWalker::GetSymbolFromAddress(context);
                const auto callStack = StackWalker::GetCallStackEx(addressToTrim);

                if (!StackWalker::IsCalledFromThisModule(callStack))
                {
                    const CString libName = libFileName;
                    ProcessLoadLibrary(libName, callStack);
				}
			}
        }

        return RealLoadLibraryW(libFileName);
    }

    HMODULE WINAPI Interceptor::OnLoadLibraryExA(const LPCSTR libFileName, const HANDLE hFile, const DWORD dwFlags)
    {
        if (const RecursionGuard recursionGuard; recursionGuard.IsInitiator())
        {
            if (!ProcessInfo::IsNETServiceThread())
            {
                CONTEXT context = { .ContextFlags = CONTEXT_CONTROL };
                RtlCaptureContext(&context);

                const auto addressToTrim = NativeStackWalker::GetSymbolFromAddress(context);
                const auto callStack = StackWalker::GetCallStackEx(addressToTrim);

                if (!StackWalker::IsCalledFromThisModule(callStack))
                {
                	const CString libName = libFileName;
                	ProcessLoadLibrary(libName, callStack);
                }
            }
        }

        return RealLoadLibraryExA(libFileName, hFile, dwFlags);
    }

    HMODULE WINAPI Interceptor::OnLoadLibraryExW(const LPCWSTR libFileName, const HANDLE hFile, const DWORD dwFlags)
    {
        if (const RecursionGuard recursionGuard; recursionGuard.IsInitiator())
        {
            if (!ProcessInfo::IsNETServiceThread())
            {
                CONTEXT context = { .ContextFlags = CONTEXT_CONTROL };
                RtlCaptureContext(&context);

                const auto addressToTrim = NativeStackWalker::GetSymbolFromAddress(context);
                const auto callStack = StackWalker::GetCallStackEx(addressToTrim);

	            if (!StackWalker::IsCalledFromThisModule(callStack))
	            {
	            	const CString libName = libFileName;
	            	ProcessLoadLibrary(libName, callStack);
	            }
            }
        }

        return RealLoadLibraryExW(libFileName, hFile, dwFlags);
    }

    BOOL WINAPI Interceptor::OnFreeLibrary(const HMODULE hLibModule)
    {
        if (const RecursionGuard recursionGuard; recursionGuard.IsInitiator())
        {
            if (!ProcessInfo::IsNETServiceThread())
            {
                CONTEXT context = { .ContextFlags = CONTEXT_CONTROL };
                RtlCaptureContext(&context);

                const auto addressToTrim = NativeStackWalker::GetSymbolFromAddress(context);
                const auto callStack = StackWalker::GetCallStackEx(addressToTrim);

	            if (!StackWalker::IsCalledFromThisModule(callStack))
	            {
            		ProcessFreeLibrary(hLibModule, callStack);
	            }
            }
        }

        return RealFreeLibrary(hLibModule);
    }

}
