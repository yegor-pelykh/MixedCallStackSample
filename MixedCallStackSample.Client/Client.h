#pragma once

namespace MixedCallStackSampleClient
{
	class Client final
	{
	public:
		static void Load();
		static void Unload();

	private:
		static void SetRuntimeLocale();

	};

}
