using System.Runtime.InteropServices;

namespace MixedCallStackSample.ClientTest.CLR
{
    internal class Program
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr LoadLibrary(string libFileName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool FreeLibrary(IntPtr hLibModule);

        // A function delegate that will be called from the loaded DLL
        // to test the managed-unmanaged code transition on the stack
        public delegate void ManagedToNativeTestFuncCallback();

        // A 32 bit function that will be called from the loaded DLL
        // to test the managed-unmanaged code transition on the stack
        [DllImport(
            @"Client.32\MixedCallStackSample.Client.32.dll",
            EntryPoint = "ManagedToNativeTestFunc",
            CallingConvention = CallingConvention.StdCall
        )]
        static extern void ManagedToNativeTestFunc32(ManagedToNativeTestFuncCallback callback);

        // A 64 bit function that will be called from the loaded DLL
        // to test the managed-unmanaged code transition on the stack
        [DllImport(
            @"Client.64\MixedCallStackSample.Client.64.dll",
            EntryPoint = "ManagedToNativeTestFunc",
            CallingConvention = CallingConvention.StdCall
            )]
        static extern void ManagedToNativeTestFunc64(ManagedToNativeTestFuncCallback callback);

        private static void Main()
        {
            var clientDllName = Environment.Is64BitProcess
                ? Client64LibPath
                : Client32LibPath;

            // Despite the fact that the profiler DLL will already be loaded before this call,
            // just in case we will try to load it again to check.
            // In case the DLL was not loaded by the CLR environment, we load it now.
            var clientModule = LoadLibrary(clientDllName);
            if (clientModule == IntPtr.Zero)
            {
                var err = Marshal.GetLastWin32Error();
                Console.WriteLine($"Injection of client library failed. Error: {err}");
                Console.ReadKey();
                return;
            }

            // Run tests
            TestProcedure();
        }

        // Main test procedure
        private static void TestProcedure()
        {
            if (Environment.Is64BitProcess)
                ManagedToNativeTestFunc64(TestProcedureCallback);
            else
                ManagedToNativeTestFunc32(TestProcedureCallback);
        }

        // A callback that will be called from native code
        private static void TestProcedureCallback()
        {
            TestInnerProc1();
        }

        // Nested function 1 to increase managed frames on the stack
        private static void TestInnerProc1()
        {
            TestInnerProc2();
        }

        // Nested function 2 to increase managed frames on the stack
        private static void TestInnerProc2()
        {
            TestInnerProc3();
        }

        // Nested function 3 to increase managed frames on the stack
        private static void TestInnerProc3()
        {
            for (var i = 0; i < IterationsCount; i++)
            {
                var moduleUser32 = LoadTestLibrary("user32.dll");
                var moduleKernel32 = LoadTestLibrary("kernel32.dll");

                if (moduleKernel32 != IntPtr.Zero)
                    FreeLibrary(moduleKernel32);
                if (moduleUser32 != IntPtr.Zero)
                    FreeLibrary(moduleUser32);

                Thread.Sleep(1000);
            }
        }

        // Just a wrapper for the function of loading a test library
        private static nint LoadTestLibrary(string libFileName)
        {
            return LoadLibrary(libFileName);
        }

        #region Constants
        private const int IterationsCount = 1000;
        private const string Client32LibPath = "Client.32/MixedCallStackSample.Client.32.dll";
        private const string Client64LibPath = "Client.64/MixedCallStackSample.Client.64.dll";
        #endregion

    }

}
