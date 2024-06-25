#include "pch.h"
#include "CorProfiler.h"
#include "COMPtrHolder.hpp"
#include "GlobalData.h"
#include "NativeStackWalker.h"

constexpr ULONG ModuleNameMaxLength = 2048;
constexpr ULONG32 TypeArgsMaxLength = 10;
constexpr ULONG MethodNameMaxLength = 256;
constexpr ULONG TypeDefMaxLength = 1024;

namespace MixedCallStackSampleClient
{
	CorProfiler::CorProfiler()
		: _refCount(0)
		, _profilerInfo(nullptr)
		, _isAppExecutionStarted(false)
	{
	}

	CorProfiler::~CorProfiler()
	{
		ReleaseProfilerInfo();
	}

	HRESULT CorProfiler::QueryInterface(const IID& riid, void** ppvObject)
	{
		if (riid == __uuidof(ICorProfilerCallback9) || 
			riid == __uuidof(ICorProfilerCallback8) ||
			riid == __uuidof(ICorProfilerCallback7) ||
			riid == __uuidof(ICorProfilerCallback6) ||
			riid == __uuidof(ICorProfilerCallback5) ||
			riid == __uuidof(ICorProfilerCallback4) ||
			riid == __uuidof(ICorProfilerCallback3) ||
			riid == __uuidof(ICorProfilerCallback2) ||
			riid == __uuidof(ICorProfilerCallback) ||
			riid == IID_IUnknown)
		{
			*ppvObject = this;
			this->AddRef();
			return S_OK;
		}

		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	ULONG CorProfiler::AddRef()
	{
		return ++_refCount;
	}

	ULONG CorProfiler::Release()
	{
		const long count = --_refCount;
		if (count <= 0)
			delete this;

		return count;
	}

	HRESULT CorProfiler::Initialize(IUnknown* pICorProfilerInfoUnk)
	{
		ReleaseProfilerInfo();

		const HRESULT queryResult = pICorProfilerInfoUnk->QueryInterface(
			__uuidof(ICorProfilerInfo8), reinterpret_cast<void**>(&_profilerInfo));

		if (FAILED(queryResult))
		{
			ReleaseProfilerInfo();
			return E_FAIL;
		}

		// Saving a profiler instance for use in asynchronous functions
		GlobalData::SetCorProfiler(this);

		return SetEventMask();
	}

	HRESULT CorProfiler::Shutdown()
	{
		ReleaseProfilerInfo();
		GlobalData::ReleaseCorProfiler();
		return S_OK;
	}

	HRESULT CorProfiler::AppDomainCreationStarted(AppDomainID appDomainId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus)
	{
		return S_OK;
	}

	HRESULT CorProfiler::AppDomainShutdownStarted(AppDomainID appDomainId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus)
	{
		return S_OK;
	}

	HRESULT CorProfiler::AssemblyLoadStarted(AssemblyID assemblyId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
	{
		return S_OK;
	}

	HRESULT CorProfiler::AssemblyUnloadStarted(AssemblyID assemblyId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ModuleLoadStarted(ModuleID moduleId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
	{
		if (FAILED(hrStatus))
			return S_OK;

		LPCBYTE baseAddress;
		CString moduleName;
		const HRESULT result = GetModuleInfo(moduleId, baseAddress, moduleName);
		if (SUCCEEDED(result))
		{
			_moduleBaseAddress[moduleId] = baseAddress;
			_moduleName[moduleId] = moduleName;
		}

		return S_OK;
	}

	HRESULT CorProfiler::ModuleUnloadStarted(ModuleID moduleId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus)
	{
		if (FAILED(hrStatus))
			return S_OK;

		_moduleBaseAddress.erase(moduleId);
		_moduleName.erase(moduleId);

		return S_OK;
	}

	HRESULT CorProfiler::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ClassLoadStarted(ClassID classId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ClassLoadFinished(ClassID classId, HRESULT hrStatus)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ClassUnloadStarted(ClassID classId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ClassUnloadFinished(ClassID classId, HRESULT hrStatus)
	{
		return S_OK;
	}

	HRESULT CorProfiler::FunctionUnloadStarted(FunctionID functionId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
	{
		return S_OK;
	}

	HRESULT CorProfiler::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
	{
		const auto funcName = GetFunctionName(functionId);
		if (funcName == "Main")
			_isAppExecutionStarted = true;

		return S_OK;
	}

	HRESULT CorProfiler::JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction)
	{
		const auto funcName = GetFunctionName(functionId);
		if (funcName == "Main")
			_isAppExecutionStarted = true;

		return S_OK;
	}

	HRESULT CorProfiler::JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result)
	{
		return S_OK;
	}

	HRESULT CorProfiler::JITFunctionPitched(FunctionID functionId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ThreadCreated(ThreadID threadId)
	{
		_threadsManaged.push_front(threadId);
		return S_OK;
	}

	STDMETHODIMP CorProfiler::ThreadDestroyed(ThreadID threadId)
	{
		_threadsManaged.remove(threadId);
		return S_OK;
	}

	HRESULT CorProfiler::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId)
	{
		_threadsM2N[managedThreadId] = osThreadId;
		_threadsN2M[osThreadId] = managedThreadId;
		return S_OK;
	}

	HRESULT CorProfiler::RemotingClientInvocationStarted()
	{
		return S_OK;
	}

	HRESULT CorProfiler::RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync)
	{
		return S_OK;
	}

	HRESULT CorProfiler::RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync)
	{
		return S_OK;
	}

	HRESULT CorProfiler::RemotingClientInvocationFinished()
	{
		return S_OK;
	}

	HRESULT CorProfiler::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
	{
		return S_OK;
	}

	HRESULT CorProfiler::RemotingServerInvocationStarted()
	{
		return S_OK;
	}

	HRESULT CorProfiler::RemotingServerInvocationReturned()
	{
		return S_OK;
	}

	HRESULT CorProfiler::RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync)
	{
		return S_OK;
	}

	HRESULT CorProfiler::UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
	{
		return S_OK;
	}

	HRESULT CorProfiler::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
	{
		return S_OK;
	}

	HRESULT CorProfiler::RuntimeSuspendFinished()
	{
		return S_OK;
	}

	HRESULT CorProfiler::RuntimeSuspendAborted()
	{
		return S_OK;
	}

	HRESULT CorProfiler::RuntimeResumeStarted()
	{
		return S_OK;
	}

	HRESULT CorProfiler::RuntimeResumeFinished()
	{
		return S_OK;
	}

	HRESULT CorProfiler::RuntimeThreadSuspended(ThreadID threadId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::RuntimeThreadResumed(ThreadID threadId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[],
		ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::ObjectAllocated(ObjectID objectId, ClassID classId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs,
		ObjectID objectRefIds[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::RootReferences(ULONG cRootRefs, ObjectID rootRefIds[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionThrown(ObjectID thrownObjectId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionSearchFunctionEnter(FunctionID functionId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionSearchFunctionLeave()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionSearchFilterEnter(FunctionID functionId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionSearchFilterLeave()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionSearchCatcherFound(FunctionID functionId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionOSHandlerEnter(UINT_PTR __unused)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionOSHandlerLeave(UINT_PTR __unused)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionUnwindFunctionEnter(FunctionID functionId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionUnwindFunctionLeave()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionUnwindFinallyEnter(FunctionID functionId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionUnwindFinallyLeave()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionCatcherLeave()
	{
		return S_OK;
	}

	HRESULT CorProfiler::COMClassicVTableCreated(ClassID wrappedClassId, const GUID& implementedIID, void* pVTable,
		ULONG cSlots)
	{
		return S_OK;
	}

	HRESULT CorProfiler::COMClassicVTableDestroyed(ClassID wrappedClassId, const GUID& implementedIID,
		void* pVTable)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionCLRCatcherFound()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionCLRCatcherExecute()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[],
		COR_PRF_GC_REASON reason)
	{
		return S_OK;
	}

	HRESULT CorProfiler::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[],
		ULONG cObjectIDRangeLength[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::GarbageCollectionFinished()
	{
		return S_OK;
	}

	HRESULT CorProfiler::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
	{
		return S_OK;
	}

	HRESULT CorProfiler::RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[],
		COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::HandleCreated(GCHandleID handleId, ObjectID initialObjectId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::HandleDestroyed(GCHandleID handleId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::InitializeForAttach(IUnknown* pCorProfilerInfoUnk, void* pvClientData, UINT cbClientData)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ProfilerAttachComplete()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ProfilerDetachSucceeded()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ReJITCompilationStarted(FunctionID functionId, ReJITID rejitId, BOOL fIsSafeToBlock)
	{
		return S_OK;
	}

	HRESULT CorProfiler::GetReJITParameters(ModuleID moduleId, mdMethodDef methodId,
		ICorProfilerFunctionControl* pFunctionControl)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ReJITCompilationFinished(FunctionID functionId, ReJITID rejitId, HRESULT hrStatus,
		BOOL fIsSafeToBlock)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionId,
		HRESULT hrStatus)
	{
		return S_OK;
	}

	HRESULT CorProfiler::MovedReferences2(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[],
		ObjectID newObjectIDRangeStart[], SIZE_T cObjectIDRangeLength[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::SurvivingReferences2(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[],
		SIZE_T cObjectIDRangeLength[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::ConditionalWeakTableElementReferences(ULONG cRootRefs, ObjectID keyRefIds[],
		ObjectID valueRefIds[], GCHandleID rootIds[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::GetAssemblyReferences(const WCHAR* wszAssemblyPath,
		ICorProfilerAssemblyReferenceProvider* pAsmRefProvider)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ModuleInMemorySymbolsUpdated(ModuleID moduleId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::DynamicMethodJITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock,
		LPCBYTE pILHeader, ULONG cbILHeader)
	{
		return S_OK;
	}

	HRESULT CorProfiler::DynamicMethodJITCompilationFinished(FunctionID functionId, HRESULT hrStatus,
		BOOL fIsSafeToBlock)
	{
		return S_OK;
	}

	HRESULT CorProfiler::DynamicMethodUnloaded(FunctionID functionId)
	{
		return S_OK;
	}

	void CorProfiler::GetMixedCallStack(const PCONTEXT context)
	{
		const auto nativeThreadHandle = GetCurrentThread();
		const auto nativeThreadID = GetCurrentThreadId();

		// This method gets the managed thrad corresponding to the native thread.
		const auto managedThreadID = GetManagedThreadIdFromNative(nativeThreadID);
		if (managedThreadID == 0)
		{
			// TODO:
			// Is it ok that sometimes we can't find the corresponding thread?
			return;
		}

		const auto ipStack = NativeStackWalker::GetIPStack(nativeThreadHandle, context);

		std::map<DWORD64, FunctionID> managedIPs;
		for (auto ip : ipStack)
		{
			FunctionID managedFunction = 0;
			HRESULT funcResult = _profilerInfo->GetFunctionFromIP(reinterpret_cast<BYTE*>(ip), &managedFunction);
			if (SUCCEEDED(funcResult) && managedFunction != 0)
				managedIPs[ip] = managedFunction;
		}

		if (managedIPs.size() == 0)
		{
			// TODO:
			// For some reason, many of the resulting mixed call stacks
			// do not contain managed functions...
			OutputDebugString(_T("managedIPs is empty!"));
		}

		// Let's try to create annotations for all IPs
		std::list<CString> annotationStack;
		NativeStackWalker::AnnotateStack(ipStack, annotationStack);

		// TODO:
		// As we usually see, only a portion of the frames were annotated.
		// Perhaps the rest is managed (or not?)

		// Okay, let's go through the frames provided by 'DoStackSnapshot'
		const auto stackFrames = new std::vector<StackFrame>();
		const HRESULT result = _profilerInfo->DoStackSnapshot(
			managedThreadID,
			DoStackSnapshotCallback,
			COR_PRF_SNAPSHOT_REGISTER_CONTEXT,
			stackFrames,
			reinterpret_cast<BYTE*>(context),
			sizeof(CONTEXT)
		);

		if (SUCCEEDED(result))
		{
			// We collected all managed frames and groups of unmanaged frames.

			// TODO:
			// What is the correct way to traverse the stack
			// and find managed frames and then combine them with unmanaged frames?

			ExpandNativeFrames(nativeThreadHandle, ipStack, *stackFrames);
		}
		else
		{
			// It is very likely that we have not yet started executing the app.
			// In this case, we can ignore it.

			if (!_isAppExecutionStarted)
				OutputDebugString(_T("App is not started yet"));
		}

		// Cleanup
		if (stackFrames != nullptr)
			delete stackFrames;
	}

	HRESULT CorProfiler::SetEventMask() const
	{
		constexpr DWORD eventMask =
			COR_PRF_MONITOR_JIT_COMPILATION |
			COR_PRF_MONITOR_MODULE_LOADS |
			COR_PRF_MONITOR_THREADS |
			COR_PRF_ENABLE_STACK_SNAPSHOT;

		return _profilerInfo->SetEventMask(eventMask);
	}

	HRESULT CorProfiler::GetModuleInfo(ModuleID moduleID, LPCBYTE& baseAddress, CString& moduleName) const
	{
		LPCBYTE address = nullptr;
		ULONG nameLength = 0;
		WCHAR name[ModuleNameMaxLength];

		const HRESULT result = _profilerInfo->GetModuleInfo(
			moduleID, &address, ModuleNameMaxLength,
			&nameLength, name, nullptr);

		if (SUCCEEDED(result))
		{
			baseAddress = address;
			moduleName = CString(name);
		}
		else
		{
			baseAddress = nullptr;
			moduleName = CString();
		}

		return result;
	}

	HRESULT CorProfiler::GetModuleInfoFromFunctionID(FunctionID funcId, LPCBYTE& baseAddress, CString& moduleName) const
	{
		ClassID classId;
		ModuleID moduleId;
		mdToken token;
		HRESULT result = _profilerInfo->GetFunctionInfo(funcId, &classId, &moduleId, &token);
		if (FAILED(result))
		{
			baseAddress = nullptr;
			moduleName = CString();
			return S_FALSE;
		}

		return GetModuleInfo(moduleId, baseAddress, moduleName);
	}

	ThreadID CorProfiler::GetManagedThreadIdFromNative(DWORD nativeThreadID)
	{
		const auto managedThreadID = _threadsN2M.find(nativeThreadID);
		return managedThreadID != _threadsN2M.end()
			? managedThreadID->second
			: 0;
	}

	CString CorProfiler::GetFunctionName(const FunctionID funcId)
	{
		// If the FunctionID is 0, we could be dealing with a native function. 
		if (funcId == 0)
			return CString();

		CString name;

		ClassID classId = 0;
		ModuleID moduleId = 0;
		mdToken token = 0;
		ULONG32 typeArgsLength = 0;
		ClassID typeArgs[TypeArgsMaxLength];
		COR_PRF_FRAME_INFO frameInfo = 0;

		HRESULT hr = S_OK;

		hr = _profilerInfo->GetFunctionInfo2(funcId, frameInfo, &classId, &moduleId, &token,
			TypeArgsMaxLength, &typeArgsLength, typeArgs);
		if (FAILED(hr))
		{
			return CString();
		}

		auto mdi = COMPtrHolder<IMetaDataImport>();
		hr = _profilerInfo->GetModuleMetaData(moduleId, ofRead,
			IID_IMetaDataImport, reinterpret_cast<IUnknown**>(&mdi));
		if (FAILED(hr))
		{
			return CString();
		}

		WCHAR funcName[MethodNameMaxLength];
		hr = mdi->GetMethodProps(token, nullptr, funcName, MethodNameMaxLength,
			nullptr, nullptr, nullptr, nullptr, 
			nullptr, nullptr);
		if (FAILED(hr))
		{
			return CString();
		}

		name.Append(funcName);

		// Fill in the type parameters of the generic method
		if (typeArgsLength > 0)
			name.Append(_T("<"));

		for (ULONG32 i = 0; i < typeArgsLength; i++)
		{
			const CString classIDName = GetClassIDName(typeArgs[i]);

			name.Append(classIDName);

			if ((i + 1) != typeArgsLength)
				name.Append(_T(", "));
		}

		if (typeArgsLength > 0)
			name.Append(_T(">"));

		return name;
	}

	CString CorProfiler::GetClassIDName(const ClassID classId)
	{
		ModuleID modId;
		mdTypeDef classToken;
		ClassID parentClassID;
		ULONG32 typeArgsLength;
		ClassID typeArgs[TypeArgsMaxLength];
		HRESULT hr = S_OK;

		if (classId == 0)
		{
			return CString();
		}

		hr = _profilerInfo->GetClassIDInfo2(classId, &modId, &classToken, &parentClassID,
			TypeArgsMaxLength, &typeArgsLength, typeArgs);
		if (CORPROF_E_CLASSID_IS_ARRAY == hr)
		{
			// We have a ClassID of an array.
			return CString("ArrayClass");
		}
		else if (CORPROF_E_CLASSID_IS_COMPOSITE == hr)
		{
			// We have a composite class
			return CString("CompositeClass");
		}
		else if (CORPROF_E_DATAINCOMPLETE == hr)
		{
			// type-loading is not yet complete. Cannot do anything about it.
			return CString("DataIncomplete");
		}
		else if (FAILED(hr))
		{
			return CString();
		}

		auto mdi = COMPtrHolder<IMetaDataImport>();
		hr = _profilerInfo->GetModuleMetaData(modId, (ofRead | ofWrite),
			IID_IMetaDataImport, reinterpret_cast<IUnknown**>(&mdi));
		if (FAILED(hr))
		{
			return CString();
		}

		WCHAR wName[TypeDefMaxLength];
		DWORD dwTypeDefFlags = 0;
		hr = mdi->GetTypeDefProps(classToken, wName, TypeDefMaxLength,
			nullptr, &dwTypeDefFlags,
		nullptr);
		if (FAILED(hr))
		{
			return CString();
		}

		CString name = wName;
		if (typeArgsLength > 0)
			name.Append(_T("<"));

		for (ULONG32 i = 0; i < typeArgsLength; i++)
		{
			const CString classIDName = GetClassIDName(typeArgs[i]);

			name.Append(classIDName);

			if ((i + 1) != typeArgsLength)
				name.Append(_T(", "));
		}

		if (typeArgsLength > 0)
			name.Append(_T(">"));

		return name;
	}

	void CorProfiler::ReleaseProfilerInfo()
	{
		if (_profilerInfo != nullptr)
		{
			_profilerInfo->Release();
			_profilerInfo = nullptr;
		}
	}

	HRESULT __stdcall CorProfiler::DoStackSnapshotCallback(
		FunctionID funcId,
		UINT_PTR ip,
		COR_PRF_FRAME_INFO frameInfo,
		ULONG32 contextSize,
		BYTE context[],
		void* clientData
	)
	{
		auto stackFrames = static_cast<std::vector<StackFrame>*>(clientData);
		auto contextData = reinterpret_cast<CONTEXT*>(context);

#ifdef _M_IX86
		DWORD64 addrPC = contextData->Eip;
		DWORD64 addrFrame = contextData->Ebp;
		DWORD64 addrStack = contextData->Esp;
#elif _M_X64
		DWORD64 addrPC = ip;
		DWORD64 addrFrame = contextData->Rbp;
		DWORD64 addrStack = contextData->Rsp;
#elif _M_IA64
		DWORD64 addrPC = contextData->StIIP;
		DWORD64 addrFrame = contextData->IntSp;
		DWORD64 addrStack = contextData->RsBSP;
#else
#error "Platform not supported!"
#endif

		if (funcId == 0)
		{
			// This tells us that this frame is unmanaged
			stackFrames->push_back(StackFrame(false, addrPC, addrFrame, addrStack));
			return S_OK;
		}

		// Here we process the managed frame and save information about
		// the module from which the function was called
		// (at the moment I only need information about the module).

		const auto corProfiler = GlobalData::GetCorProfiler();
		if (corProfiler == nullptr)
			return E_FAIL;

		LPCBYTE baseAddress;
		CString moduleName;
		const HRESULT result = corProfiler->GetModuleInfoFromFunctionID(funcId, baseAddress, moduleName);
		if (SUCCEEDED(result))
		{
			stackFrames->push_back(StackFrame(true, addrPC, addrFrame, addrStack,
				moduleName, baseAddress));
		}
		else
		{
			stackFrames->push_back(StackFrame(true, addrPC, addrFrame, addrStack,
				CString(), nullptr));
		}

		return result;
	}

	void CorProfiler::ExpandNativeFrames(
		HANDLE threadHandle,
		const std::list<DWORD64>& ipStack,
		const std::vector<StackFrame>& stackFrames
	)
	{
		for (const auto& frame : stackFrames)
		{
			if (frame.IsManaged)
			{
				OutputDebugString(_T("Managed Frame"));
			}
			else
			{
				OutputDebugString(_T("Unmanaged Frame Block"));
			}
		}
	}

}
