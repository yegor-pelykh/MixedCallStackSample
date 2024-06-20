#pragma once

namespace MixedCallStackSampleClient
{
	class StackFrame final
	{
	public:
		StackFrame(
			const bool isManaged,
			const DWORD64 addrPC,
			const DWORD64 addrFrame,
			const DWORD64 addrStack,
			const CString& moduleName,
			const LPCBYTE& moduleBaseAddress
		)
		{
			IsManaged = isManaged;
			AddrPC = addrPC;
			AddrFrame = addrFrame;
			AddrStack = addrStack;
			ModuleName = moduleName;
			ModuleBaseAddress = moduleBaseAddress;
		}

		StackFrame(
			const bool isManaged,
			const DWORD64 addrPC,
			const DWORD64 addrFrame,
			const DWORD64 addrStack
		) : StackFrame(
			isManaged,
			addrPC,
			addrFrame,
			addrStack,
			CString(),
			nullptr
		)
		{
		}

	public:
		bool IsManaged;
		DWORD64 AddrPC;
		DWORD64 AddrFrame;
		DWORD64 AddrStack;
		CString ModuleName;
		LPCBYTE ModuleBaseAddress;

	};
}