#pragma once

namespace MixedCallStackSampleClient
{
	class RecursionGuard final
	{
	public:
		RecursionGuard()
		{
			std::lock_guard guard(_mutex);
			const DWORD threadId = GetCurrentThreadId();
			if (_enteredThreads.contains(threadId))
			{
				_isInitiator = false;
			}
			else
			{
				_enteredThreads.insert(threadId);
				_isInitiator = true;
			}
		}
		~RecursionGuard()
		{
			std::lock_guard guard(_mutex);
			const DWORD threadId = GetCurrentThreadId();
			_enteredThreads.erase(threadId);
		}

	public:
		[[nodiscard]] bool IsInitiator() const
		{
			return _isInitiator;
		}

	private:
		bool _isInitiator;

	private:
		inline static auto _enteredThreads = std::set<DWORD>();
		inline static auto _mutex = std::mutex();

	};
}