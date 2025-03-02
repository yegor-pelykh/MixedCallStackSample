#pragma once
#include "StackFrame.hpp"

namespace MixedCallStackSampleClient
{
	class StackWalker
	{
	public:
		static std::deque<PVOID> GetCallStack(const PVOID trimAddress = nullptr);
		static std::deque<StackFrame> GetCallStackEx(const PVOID trimAddress = nullptr);
		static bool IsCalledFromThisModule(const std::deque<StackFrame>& callStack);

	private:
		static void GetStackFrameByAddress(const PVOID address, StackFrame& stackFrame);

	private:
		static std::deque<PVOID> TrimToEntryPoint(const std::deque<PVOID>& callStack);
		static std::deque<PVOID> TrimToAddress(const std::deque<PVOID>& callStack, PVOID address);

	};
}