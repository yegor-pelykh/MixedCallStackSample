#pragma once

namespace MixedCallStackSampleClient
{
	template <class MetaInterface>
	class COMPtrHolder final
	{
	public:
		COMPtrHolder()
		{
			_ptr = nullptr;
		}
		~COMPtrHolder()
		{
			if (_ptr != nullptr)
			{
				_ptr->Release();
				_ptr = nullptr;
			}
		}

	public:
		MetaInterface* operator->()
		{
			return _ptr;
		}
		MetaInterface** operator&()
		{
			return &_ptr;
		}
		explicit operator MetaInterface*()
		{
			return _ptr;
		}

	private:
		MetaInterface* _ptr;

	};
}
