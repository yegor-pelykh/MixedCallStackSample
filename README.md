# MixedCallStackSample

## Overview

Welcome to the **MixedCallStackSample** project! This research initiative explores the fascinating concept of capturing a mixed call stack from within hooked process functions. By hooking into the `LoadLibrary` and `FreeLibrary` functions, we aim to gain insights into the call stack of managed processes.

## Project Structure

The solution comprises several projects, each serving a unique purpose:

- **`MixedCallStackSample.Client.32`**: A **client DLL library** designed for **32-bit processes**. This library can be loaded into a process using either the CLR environment (for managed applications) or through manual loading (for unmanaged applications).
  
- **`MixedCallStackSample.Client.64`**: Similar to the 32-bit version, this is a **client DLL library** for **64-bit processes**. It supports loading via the CLR environment or manual methods, catering to both managed and unmanaged scenarios.
  
- **`MixedCallStackSample.ClientTest.Native`**: A test application that operates in an **unmanaged environment**. It demonstrates how to load the client DLL using the `LoadLibrary` function.
  
- **`MixedCallStackSample.ClientTest.NetCore`**: This is a test managed application built on **.NET Core**. It loads the client DLL while utilizing the environment variables defined in the project settings.
  
- **`MixedCallStackSample.ClientTest.NetFX`**: A test managed application for the **.NET Framework** that also loads the client DLL using the specified environment variables.

## Important Note

Please be aware that this project is intended solely for research and educational purposes. It serves as a platform for experimentation and learning about mixed call stacks in managed processes.

We hope you find this project insightful and inspiring for your own development endeavors!
