#include "pch.h"
#include "Client.h"
#include "Interceptor.h"
#include "CorProfilerClassFactory.h"
#include "InstanceManager.h"

using namespace MixedCallStackSampleClient;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    if (Interceptor::IsHelperProcess())
        return TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        Client::Load();
    	break;
    case DLL_PROCESS_DETACH:
        Client::Unload();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

// A function that will be called to test the managed-unmanaged
// code transition on the stack and run tests
extern "C" void ManagedToNativeTestFunc(void (*callback)())
{
    if (callback != nullptr)
        callback();
}

extern "C" HRESULT CALLBACK DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    // {420DB72F-7A39-43C0-9E74-988670E44AB8}
    constexpr GUID clsidCorProfiler = {
    	0x420DB72F, 0x7A39, 0x43C0, { 0x9E, 0x74, 0x98, 0x86, 0x70, 0xE4, 0x4A, 0xB8 }
    };

    if (ppv == nullptr || rclsid != clsidCorProfiler)
        return CLASS_E_CLASSNOTAVAILABLE;

    const auto factory = new CorProfilerClassFactory();
    if (factory == nullptr)
        return E_OUTOFMEMORY;

    return factory->QueryInterface(riid, ppv);
}

extern "C" HRESULT CALLBACK DllCanUnloadNow()
{
    return InstanceManager::IsAnyComObjectInUse() ? S_FALSE : S_OK;
}