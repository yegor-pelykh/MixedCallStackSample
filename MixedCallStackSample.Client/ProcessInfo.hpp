#pragma once
#include "FrameRegisters.hpp"

namespace MixedCallStackSampleClient
{
	class ProcessInfo final
	{
	public:
		[[nodiscard]]
		static HANDLE GetProcessHandle()
		{
			return _processHandle;
		}
		[[nodiscard]]
		static DWORD GetProcessId()
		{
			return _processId;
		}
		static void GetFrameRegisters(FrameRegisters& registers)
		{
			CONTEXT context = { .ContextFlags = CONTEXT_CONTROL };
			RtlCaptureContext(&context);

#ifdef _M_IX86
			registers = FrameRegisters(
				reinterpret_cast<PVOID>(context.Eip),
				reinterpret_cast<PVOID>(context.Ebp),
				reinterpret_cast<PVOID>(context.Esp)
			);
#elif _M_X64
			registers = FrameRegisters(
				reinterpret_cast<PVOID>(context.Rip),
				reinterpret_cast<PVOID>(context.Rbp),
				reinterpret_cast<PVOID>(context.Rsp)
			);
#else
#error "Platform not supported!"
#endif
		}
		static bool IsNETServiceThread()
		{
			CString threadName = GetCurrentThreadName();
			threadName.MakeLower();
			return threadName.Find(_T(".net")) >= 0;
		}

	private:
		static CString GetCurrentThreadName()
		{
			const DWORD threadId = GetCurrentThreadId();
			const HANDLE thread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, threadId);
			if (thread == nullptr) {
				return CString();
			}

			PWSTR threadName = nullptr;
			HRESULT hr = GetThreadDescription(thread, &threadName);
			if (FAILED(hr))
			{
				CloseHandle(thread);
				return CString();
			}

			CString name(threadName);

			LocalFree(threadName);
			CloseHandle(thread);

			return name;
		}

	private:
		inline static HANDLE _processHandle = GetCurrentProcess();
		inline static DWORD _processId = GetCurrentProcessId();

	};
}
