#pragma once

namespace MixedCallStackSampleClient
{
	class CorProfilerClassFactory final : public IClassFactory
	{
	public:
		CorProfilerClassFactory();
		virtual ~CorProfilerClassFactory();

	public:
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
		ULONG STDMETHODCALLTYPE AddRef() override;
		ULONG STDMETHODCALLTYPE Release() override;
		HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
		HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) override;

	private:
		std::atomic<long> _refCount;

	};
}

