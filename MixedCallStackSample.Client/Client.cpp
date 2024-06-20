#include "pch.h"
#include "Client.h"
#include "GlobalData.h"
#include "Interceptor.h"
#include "NativeStackWalker.h"

namespace MixedCallStackSampleClient
{
	void Client::Load()
	{
		SetRuntimeLocale();
		NativeStackWalker::Load();
		Interceptor::AttachDetours();
	}

	void Client::Unload()
	{
		Interceptor::DetachDetours();
		GlobalData::ReleaseCorProfiler();
		NativeStackWalker::Unload();
	}

	void Client::SetRuntimeLocale()
	{
		TCHAR systemLocaleName[LOCALE_NAME_MAX_LENGTH];
		GetSystemDefaultLocaleName(systemLocaleName, LOCALE_NAME_MAX_LENGTH);
		_wsetlocale(LC_ALL, systemLocaleName);
	}
	
}
