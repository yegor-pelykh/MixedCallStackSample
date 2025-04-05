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

## Features

- **Mixed Call Stack Capture**: The project captures both managed and unmanaged call stacks, providing a comprehensive view of the execution flow.
- **CLR Profiling API Integration**: Information about managed processes is obtained using the CLR Profiling API, allowing for detailed insights into the execution of managed code.
- **Support for 32-bit and 64-bit Processes**: Designed to work with both 32-bit and 64-bit processes, making it versatile for various applications.

## Getting Started

To get started with the project, follow these steps:

1. **Clone the Repository**:

   ```cmd
   git clone https://github.com/yegor-pelykh/MixedCallStackSample.git
   cd MixedCallStackSample

2. **Build the Solution**:

    Open the solution in Visual Studio and build the projects.

3. **Run Tests**:

    Execute the test applications to see the mixed call stack capture in action. To see the mixed call stack in the code, look at the result returned by functions `GetCallStack` and `GetCallStackEx` in debug mode.

## Important Note

Please be aware that this project is intended solely for research and educational purposes. It serves as a platform for experimentation and learning about mixed call stacks in managed processes.

## Contributing

We welcome contributions to the project! If you have suggestions or improvements, please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License - see the [**LICENSE**](/LICENSE) file for details.

## Acknowledgments

- Thanks to Microsoft Research for the Detours library.
- Special thanks to the .NET Runtime team developing the CLR Profiling API for their invaluable contributions.

We hope you find this project insightful and inspiring for your own development endeavors.