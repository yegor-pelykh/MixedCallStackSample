# MixedCallStackSample

## Description

This research project is a test ground for the idea of ​​obtaining mixed call stack from within hooked process functions.

In this case, the `LoadLibrary` (of all variations) and `FreeLibrary` functions are hooked.

When calling the hooked `LoadLibrary`/`FreeLibrary` functions, I'm trying to get a mixed call stack (for managed process).

## Project structure

Solution includes the following projects:

- **`MixedCallStackSample.Client.32`**: is a **client DLL library for 32-bit processes** that must be loaded into the process, either using the CLR environment (for managed processes) or manual loading (for unmanaged processes)
- **`MixedCallStackSample.Client.64`**: is a **client DLL library for 64-bit processes** that must be loaded into the process, either using the CLR environment (for managed processes) or manual loading (for unmanaged processes)
- **`MixedCallStackSample.ClientTest.CLR`**: is a test **managed application** that loads the client DLL using the environment variables specified in the project properties
- **`MixedCallStackSample.ClientTest.Native`**: is a test **unmanaged application** that loads the client DLL using the `LoadLibrary` call.


## Warning

The project is intended for research and educational purposes.
