#include <afx.h>
#include <afxwin.h>
#include "MixedCallStackSample.ClientTest.Native.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static CWinApp theApp;

constexpr int iterationsCount = 1000;
constexpr const TCHAR* client32LibPath = _T("Client.32/MixedCallStackSample.Client.32.dll");
constexpr const TCHAR* client64LibPath = _T("Client.64/MixedCallStackSample.Client.64.dll");

static bool Is64BitProcess()
{
    return CHAR_BIT * sizeof(void*) == 64;
}

static void TestProcedure()
{
    for (int i = 0; i < iterationsCount; i++)
    {
        const HMODULE moduleUser32 = LoadLibrary(_T("user32.dll"));
        const HMODULE moduleKernel32 = LoadLibrary(_T("kernel32.dll"));

        if (moduleKernel32 != nullptr)
            FreeLibrary(moduleKernel32);
        if (moduleUser32 != nullptr)
            FreeLibrary(moduleUser32);

        Sleep(20);
    }
}

int main()
{
	const HMODULE hModule = GetModuleHandle(nullptr);
    if (hModule == nullptr)
    {
	    wprintf(L"Fatal Error: GetModuleHandle failed.\n");
	    return 1;
    }

    if (!AfxWinInit(hModule, nullptr, GetCommandLine(), 0))
    {
	    wprintf(L"Fatal Error: MFC initialization failed\n");
	    return 1;
    }

    const CString clientDllName = Is64BitProcess()
        ? client64LibPath
        : client32LibPath;

    const HMODULE clientModule = LoadLibrary(clientDllName);
    if (clientModule == nullptr)
    {
        const auto err = GetLastError();
        wprintf(L"Injection of client library failed. Error: %i.\n", err);
        return 1;
    }

    TestProcedure();

    return 0;
}
