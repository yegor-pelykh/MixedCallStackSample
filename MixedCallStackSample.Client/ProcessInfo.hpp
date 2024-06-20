#pragma once

namespace MixedCallStackSampleClient
{
	class ProcessInfo final
	{
	public:
		[[nodiscard]] static HANDLE GetProcessHandle() { return _processHandle; }
		[[nodiscard]] static DWORD GetProcessId() { return _processId; }

	private:
		inline static HANDLE _processHandle = GetCurrentProcess();
		inline static DWORD _processId = GetCurrentProcessId();

	};

}