#pragma once

namespace MixedCallStackSampleClient
{
	class CorProfiler;

	class InstanceManager final
	{
	public:
		static void SetCorProfiler(CorProfiler* profiler);
		static void ReleaseCorProfiler();
		static void IncrementComObjectsInUse() { ++_comObjectsInUse; }
		static void DecrementComObjectsInUse() { --_comObjectsInUse; }
		static bool IsAnyComObjectInUse() { return _comObjectsInUse.load() != 0; }

	public:
		[[nodiscard]]
		static CorProfiler* GetCorProfiler() { return _profiler; }
	
	private:
		inline static CorProfiler* _profiler = nullptr;
		inline static std::atomic<long> _comObjectsInUse = 0;

	};

}
