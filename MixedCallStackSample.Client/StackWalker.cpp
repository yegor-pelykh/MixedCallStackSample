#include "pch.h"
#include "StackWalker.h"
#include "CorProfiler.h"
#include "InstanceManager.h"
#include "NativeStackWalker.h"
#include "ProcessInfo.hpp"

namespace MixedCallStackSampleClient
{
    std::deque<PVOID> StackWalker::GetCallStack(const PVOID trimAddress /* = nullptr */)
    {
        FrameRegisters registers;
        ProcessInfo::GetFrameRegisters(registers);

        const auto corProfiler = InstanceManager::GetCorProfiler();
        auto callStack = corProfiler != nullptr
    		? corProfiler->GetMixedCallStack(registers)
    		: NativeStackWalker::GetNativeCallStack(registers);

        callStack = trimAddress != nullptr
            ? TrimToAddress(callStack, trimAddress)
            : TrimToEntryPoint(callStack);

        return callStack;
    }

    std::deque<StackFrame> StackWalker::GetCallStackEx(const PVOID trimAddress /* = nullptr */)
    {
        const auto callStack = GetCallStack(trimAddress);

        auto frames = std::deque<StackFrame>();
        for (const auto& address : callStack)
        {
            StackFrame frame;
            GetStackFrameByAddress(address, frame);
            frames.push_back(frame);
        }
        return frames;
    }

    bool StackWalker::IsCalledFromThisModule(const std::deque<StackFrame>& callStack)
    {
        if (callStack.size() < 2)
            return false;

        HMODULE currentModuleHandle = nullptr;
        BOOL success = GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            static_cast<LPCTSTR>(callStack[0].ReturnAddress), &currentModuleHandle);
        if (!success || currentModuleHandle == nullptr)
            return false;

        HMODULE callerModuleHandle = nullptr;
        success = GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            static_cast<LPCTSTR>(callStack[1].ReturnAddress), &callerModuleHandle);
        if (!success || callerModuleHandle == nullptr)
            return false;

        return callerModuleHandle == currentModuleHandle;
    }

    void StackWalker::GetStackFrameByAddress(const PVOID address, StackFrame& stackFrame)
    {
        stackFrame = StackFrame();
        stackFrame.ReturnAddress = address;

        const auto corProfiler = InstanceManager::GetCorProfiler();

        FunctionID managedFunctionID = 0;
        stackFrame.Type = corProfiler != nullptr && corProfiler->IsFunctionManaged(
            static_cast<LPCBYTE>(address),
            managedFunctionID
        ) && managedFunctionID != 0
            ? StackFrameType::Managed
            : StackFrameType::Native;

        if (stackFrame.Type == StackFrameType::Managed)
        {
            ClassID classID;
            ModuleID moduleID;
            mdToken token;
            HRESULT gfiResult = corProfiler->GetFunctionInfo(
                managedFunctionID, classID, moduleID, token);
            if (SUCCEEDED(gfiResult))
            {
                LPCBYTE moduleBaseAddress;
                HRESULT result = corProfiler->GetModuleBaseAddress(moduleID, moduleBaseAddress);
                if (SUCCEEDED(result))
                    stackFrame.ModuleBaseAddress = const_cast<PVOID>(reinterpret_cast<const void*>(moduleBaseAddress));

                CString modulePath;
                result = corProfiler->GetModulePath(moduleID, modulePath);
                if (SUCCEEDED(result))
                {
                    stackFrame.ModulePath = modulePath;
                    stackFrame.ModuleName = modulePath.Mid(modulePath.ReverseFind('\\') + 1);
                }

                CString annotation;
                result = corProfiler->GetAnnotation(
                    stackFrame.ModuleName, classID, managedFunctionID, annotation);
                if (SUCCEEDED(result))
                    stackFrame.Annotation = annotation;
            }
        }
        else
        {
            DWORD64 moduleBaseAddress = 0;
            if (SUCCEEDED(NativeStackWalker::GetModuleBaseAddress(address, moduleBaseAddress)))
            {
                stackFrame.ModuleBaseAddress = reinterpret_cast<PVOID>(moduleBaseAddress);

                IMAGEHLP_MODULE64 moduleInfo;
                if (SUCCEEDED(NativeStackWalker::GetModuleInfo(moduleBaseAddress, moduleInfo)))
                {
                    stackFrame.ModuleName = CString(moduleInfo.ModuleName);
                    stackFrame.ModulePath = CString(moduleInfo.ImageName);

                    CString annotation;
                    if (SUCCEEDED(NativeStackWalker::GetAnnotation(
                        reinterpret_cast<DWORD64>(address), stackFrame.ModuleName, annotation)))
                    {
                        stackFrame.Annotation = annotation;
                    }
                }
            }
            else
            {
#ifdef DEBUG
                CString outputMessage;
                outputMessage.Format(_T("Unable to retrieve information for address %p\n"), address);
                OutputDebugString(outputMessage);
#endif
            }
        }
    }

    std::deque<PVOID> StackWalker::TrimToEntryPoint(const std::deque<PVOID>& callStack)
    {
        std::deque<PVOID> trimmed;

        DWORD64 currentModuleHandle = 0;
        GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCTSTR>(&StackWalker::TrimToEntryPoint),
            reinterpret_cast<HMODULE*>(&currentModuleHandle));

        const auto corProfiler = InstanceManager::GetCorProfiler();

        PVOID lastInThisModule = nullptr;
        for (auto it = callStack.begin(); it != callStack.end(); ++it)
        {
            bool isThisModule;
            FunctionID managedFunctionID = 0;
            if (corProfiler == nullptr ||
                !corProfiler->IsFunctionManaged(static_cast<LPCBYTE>(*it), managedFunctionID))
            {
                DWORD64 moduleBaseAddress = 0;
                if (SUCCEEDED(NativeStackWalker::GetModuleBaseAddress(*it, moduleBaseAddress)))
                    isThisModule = moduleBaseAddress == currentModuleHandle;
                else
                    isThisModule = false;
            }
            else
                isThisModule = false;

            if (isThisModule)
            {
                lastInThisModule = *it;
            }
            else
            {
                trimmed.push_back(lastInThisModule);
                std::copy(it, callStack.end(), std::back_inserter(trimmed));
                break;
            }
        }

        return trimmed;
    }

    std::deque<PVOID> StackWalker::TrimToAddress(const std::deque<PVOID>& callStack, PVOID address)
    {
        std::deque<PVOID> trimmed;

        for (auto it = callStack.begin(); it != callStack.end(); ++it)
        {
	        const PVOID symProcAddress = NativeStackWalker::GetSymbolFromAddress(*it);
            if (address == symProcAddress)
            {
                std::copy(it, callStack.end(), std::back_inserter(trimmed));
                break;
            }
        }

        return trimmed;
    }
}
