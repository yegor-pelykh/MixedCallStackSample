#include "pch.h"
#include "InstanceManager.h"
#include "CorProfiler.h"

namespace MixedCallStackSampleClient
{
	void InstanceManager::SetCorProfiler(CorProfiler* profiler)
	{
		if (profiler != nullptr)
		{
			_profiler = profiler;
			profiler->AddRef();
		}
	}

	void InstanceManager::ReleaseCorProfiler()
	{
		if (_profiler != nullptr)
		{
			_profiler->Release();
			_profiler = nullptr;
		}
	}

}
