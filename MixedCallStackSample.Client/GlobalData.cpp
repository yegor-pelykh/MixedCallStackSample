#include "pch.h"
#include "GlobalData.h"
#include "CorProfiler.h"

namespace MixedCallStackSampleClient
{
	void GlobalData::SetCorProfiler(CorProfiler* profiler)
	{
		if (profiler != nullptr)
		{
			_profiler = profiler;
			profiler->AddRef();
		}
	}

	void GlobalData::ReleaseCorProfiler()
	{
		if (_profiler != nullptr)
		{
			_profiler->Release();
			_profiler = nullptr;
		}
	}

}
