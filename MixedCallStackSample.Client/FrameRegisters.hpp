#pragma once

namespace MixedCallStackSampleClient
{
	struct FrameRegisters final
	{
	public:
		FrameRegisters()
			: FrameRegisters(
				nullptr,
				nullptr,
				nullptr
			)
		{
		}

		explicit FrameRegisters(
			const PVOID addrPC,
			const PVOID addrFrame,
			const PVOID addrStack
		)
		{
			AddrPC = addrPC;
			AddrFrame = addrFrame;
			AddrStack = addrStack;
		}

		FrameRegisters(const FrameRegisters& other)
		{
			AddrPC = other.AddrPC;
			AddrFrame = other.AddrFrame;
			AddrStack = other.AddrStack;
		}

	public:
		FrameRegisters& operator=(const FrameRegisters& other)
		{
			if (this == &other)
				return *this;

			AddrPC = other.AddrPC;
			AddrFrame = other.AddrFrame;
			AddrStack = other.AddrStack;

			return *this;
		}

	public:
		void GetContext(CONTEXT& context) const
		{
			context = { .ContextFlags = CONTEXT_CONTROL };

#ifdef _M_IX86
			context.Eip = reinterpret_cast<DWORD>(AddrPC);
			context.Ebp = reinterpret_cast<DWORD>(AddrFrame);
			context.Esp = reinterpret_cast<DWORD>(AddrStack);
#elif _M_X64
			context.Rip = reinterpret_cast<DWORD64>(AddrPC);
			context.Rbp = reinterpret_cast<DWORD64>(AddrFrame);
			context.Rsp = reinterpret_cast<DWORD64>(AddrStack);
#else
#error "Platform not supported!"
#endif
		}

	public:
		PVOID AddrPC;
		PVOID AddrFrame;
		PVOID AddrStack;

	};
}