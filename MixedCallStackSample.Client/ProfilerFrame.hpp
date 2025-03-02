#pragma once
#include "FrameRegisters.hpp"

namespace MixedCallStackSampleClient
{
	class ProfilerFrame final
	{
	public:
		ProfilerFrame(
			const bool isManaged,
			const FrameRegisters& registers
		)
		{
			IsManaged = isManaged;
			Registers = registers;
		}

	public:
		bool IsManaged;
		FrameRegisters Registers;

	};
}
