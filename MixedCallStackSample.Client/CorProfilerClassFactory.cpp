#include "pch.h"
#include "CorProfilerClassFactory.h"
#include "GlobalData.h"
#include "CorProfiler.h"

namespace MixedCallStackSampleClient
{
	CorProfilerClassFactory::CorProfilerClassFactory()
		: _refCount(0)
	{
		GlobalData::IncrementComObjectsInUse();
	}

	CorProfilerClassFactory::~CorProfilerClassFactory()
	{
		GlobalData::DecrementComObjectsInUse();
	}

	HRESULT CorProfilerClassFactory::QueryInterface(REFIID riid, void** ppvObject)
	{
		if (riid == IID_IUnknown || riid == IID_IClassFactory)
		{
			*ppvObject = this;
			this->AddRef();
			return S_OK;
		}

		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	ULONG CorProfilerClassFactory::AddRef()
	{
		return ++_refCount;
	}

	ULONG CorProfilerClassFactory::Release()
	{
		const long count = --_refCount;
		if (count <= 0)
			delete this;

		return count;
	}

	HRESULT CorProfilerClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
	{
		if (pUnkOuter != nullptr)
		{
			*ppvObject = nullptr;
			return CLASS_E_NOAGGREGATION;
		}

		const auto profiler = new CorProfiler();
		if (profiler == nullptr)
			return E_FAIL;

		return profiler->QueryInterface(riid, ppvObject);
	}

	HRESULT CorProfilerClassFactory::LockServer(BOOL fLock)
	{
		return S_OK;
	}

}
