#pragma once

namespace MixedCallStackSampleClient
{
	enum class StackFrameType
	{
		Native,
		Managed
	};

	struct StackFrame final
	{
	public:
		StackFrame()
			: Type(StackFrameType::Native)
			, ReturnAddress(nullptr)
			, ModuleBaseAddress(nullptr)
			, ModuleName(_T(""))
			, ModulePath(_T(""))
			, Annotation(_T(""))
		{
		}
		StackFrame(const StackFrame& other)
		{
			Type = other.Type;
			ReturnAddress = other.ReturnAddress;
			ModuleBaseAddress = other.ModuleBaseAddress;
			ModuleName = other.ModuleName;
			ModulePath = other.ModulePath;
			Annotation = other.Annotation;
		}

	public:
		StackFrame& operator=(const StackFrame& other)
		{
			if (this == &other)
				return *this;

			Type = other.Type;
			ReturnAddress = other.ReturnAddress;
			ModuleBaseAddress = other.ModuleBaseAddress;
			ModuleName = other.ModuleName;
			ModulePath = other.ModulePath;
			Annotation = other.Annotation;

			return *this;
		}

	public:
		StackFrameType Type;
		PVOID ReturnAddress;
		PVOID ModuleBaseAddress;
		CString ModuleName;
		CString ModulePath;
		CString Annotation;

	};
}
