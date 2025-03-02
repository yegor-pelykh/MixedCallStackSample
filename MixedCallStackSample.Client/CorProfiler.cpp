#include "pch.h"
#include "CorProfiler.h"
#include "FrameRegisters.hpp"
#include "InstanceManager.h"
#include "NativeStackWalker.h"
#include "ProfilerFrame.hpp"

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
		, _runtimeType(COR_PRF_DESKTOP_CLR)
	{
		InstanceManager::IncrementComObjectsInUse();
	}

	CorProfiler::~CorProfiler()
	{
		InstanceManager::DecrementComObjectsInUse();
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
		const HRESULT queryResult = pICorProfilerInfoUnk->QueryInterface(
			__uuidof(ICorProfilerInfo8), reinterpret_cast<void**>(&_profilerInfo));

		if (FAILED(queryResult))
			return E_FAIL;

		// Saving a profiler instance for use in asynchronous functions
		InstanceManager::SetCorProfiler(this);

		return SetEventMask();
	}

	HRESULT CorProfiler::Shutdown()
	{
		InstanceManager::ReleaseCorProfiler();

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
		AssemblyID assemblyId = 0;
		HRESULT result = GetModuleInfo(moduleId, moduleName, baseAddress, assemblyId);
		if (SUCCEEDED(result))
		{
			_moduleBaseAddress[moduleId] = baseAddress;
			_modulePath[moduleId] = moduleName;
			_moduleAssemblyID[moduleId] = assemblyId;
		}

		auto mdi = CComPtr<IMetaDataImport>();
		result = _profilerInfo->GetModuleMetaData(moduleId, ofRead,
			IID_IMetaDataImport, reinterpret_cast<IUnknown**>(&mdi));
		if (SUCCEEDED(result))
			_moduleMDI[moduleId] = mdi;

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

		_moduleAssemblyID.erase(moduleId);
		_moduleBaseAddress.erase(moduleId);
		_modulePath.erase(moduleId);

		return S_OK;
	}

	HRESULT CorProfiler::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ClassLoadStarted(ClassID classID)
	{
		CacheClass(classID);

		return S_OK;
	}

	HRESULT CorProfiler::ClassLoadFinished(ClassID classID, HRESULT hrStatus)
	{
		CacheClass(classID);
		
		return S_OK;
	}

	HRESULT CorProfiler::ClassUnloadStarted(ClassID classID)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ClassUnloadFinished(ClassID classID, HRESULT hrStatus)
	{
		_className.erase(classID);

		return S_OK;
	}

	HRESULT CorProfiler::FunctionUnloadStarted(FunctionID functionID)
	{
		return S_OK;
	}

	HRESULT CorProfiler::JITCompilationStarted(FunctionID functionID, BOOL fIsSafeToBlock)
	{
		return S_OK;
	}

	HRESULT CorProfiler::JITCompilationFinished(FunctionID functionID, HRESULT hrStatus, BOOL fIsSafeToBlock)
	{
		const auto functionName = CacheFunction(functionID);
		if (functionName == "Main")
			_isAppExecutionStarted = true;

		return S_OK;
	}

	HRESULT CorProfiler::JITCachedFunctionSearchStarted(FunctionID functionID, BOOL* pbUseCachedFunction)
	{
		const auto functionName = CacheFunction(functionID);
		if (functionName == "Main")
			_isAppExecutionStarted = true;

		return S_OK;
	}

	HRESULT CorProfiler::JITCachedFunctionSearchFinished(FunctionID functionID, COR_PRF_JIT_CACHE result)
	{
		return S_OK;
	}

	HRESULT CorProfiler::JITFunctionPitched(FunctionID functionID)
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

	HRESULT CorProfiler::UnmanagedToManagedTransition(FunctionID functionID, COR_PRF_TRANSITION_REASON reason)
	{
		CacheFunction(functionID);

		return S_OK;
	}

	HRESULT CorProfiler::ManagedToUnmanagedTransition(FunctionID functionID, COR_PRF_TRANSITION_REASON reason)
	{
		CacheFunction(functionID);

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

	HRESULT CorProfiler::ObjectAllocated(ObjectID objectId, ClassID classID)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[])
	{
		return S_OK;
	}

	HRESULT CorProfiler::ObjectReferences(ObjectID objectId, ClassID classID, ULONG cObjectRefs,
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

	HRESULT CorProfiler::ExceptionSearchFunctionEnter(FunctionID functionID)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionSearchFunctionLeave()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionSearchFilterEnter(FunctionID functionID)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionSearchFilterLeave()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionSearchCatcherFound(FunctionID functionID)
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

	HRESULT CorProfiler::ExceptionUnwindFunctionEnter(FunctionID functionID)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionUnwindFunctionLeave()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionUnwindFinallyEnter(FunctionID functionID)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionUnwindFinallyLeave()
	{
		return S_OK;
	}

	HRESULT CorProfiler::ExceptionCatcherEnter(FunctionID functionID, ObjectID objectId)
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

	HRESULT CorProfiler::ReJITCompilationStarted(FunctionID functionID, ReJITID rejitId, BOOL fIsSafeToBlock)
	{
		return S_OK;
	}

	HRESULT CorProfiler::GetReJITParameters(ModuleID moduleId, mdMethodDef methodId,
		ICorProfilerFunctionControl* pFunctionControl)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ReJITCompilationFinished(FunctionID functionID, ReJITID rejitId, HRESULT hrStatus,
		BOOL fIsSafeToBlock)
	{
		return S_OK;
	}

	HRESULT CorProfiler::ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionID,
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

	HRESULT CorProfiler::DynamicMethodJITCompilationStarted(FunctionID functionID, BOOL fIsSafeToBlock,
		LPCBYTE pILHeader, ULONG cbILHeader)
	{
		return S_OK;
	}

	HRESULT CorProfiler::DynamicMethodJITCompilationFinished(FunctionID functionID, HRESULT hrStatus,
		BOOL fIsSafeToBlock)
	{
		return S_OK;
	}

	HRESULT CorProfiler::DynamicMethodUnloaded(FunctionID functionID)
	{
		return S_OK;
	}

	std::deque<PVOID> CorProfiler::GetMixedCallStack(const FrameRegisters& registers) const
	{
		auto callStack = std::deque<PVOID>();

		PVOID firstManagedIP = nullptr;
		FunctionID firstManagedFuncID = 0;
		const auto nativeBreakFunc = [this, &firstManagedIP, &firstManagedFuncID](const PVOID address)
		{
			FunctionID managedFunction = 0;
			if (IsFunctionManaged(static_cast<LPCBYTE>(address), managedFunction))
			{
				firstManagedIP = address;
				firstManagedFuncID = managedFunction;
				return true;
			}
			return false;
		};

		std::deque<PVOID> nativeCallStack = NativeStackWalker::GetNativeCallStack(
			registers,
			nativeBreakFunc
		);

		std::ranges::copy(nativeCallStack, std::back_inserter(callStack));

		const auto nativeThreadID = GetCurrentThreadId();
		const auto managedThreadID = GetManagedThreadIdFromNative(nativeThreadID);
		if (managedThreadID == 0)
			return callStack;

		CONTEXT context;
		registers.GetContext(context);

		auto profilerFrames = std::deque<ProfilerFrame>();
		const HRESULT result = _profilerInfo->DoStackSnapshot(
			managedThreadID,
			DoStackSnapshotCallback,
			COR_PRF_SNAPSHOT_REGISTER_CONTEXT,
			&profilerFrames,
			reinterpret_cast<PBYTE>(&context),
			sizeof(CONTEXT)
		);

		if (FAILED(result) || IS_ERROR(result))
		{
#ifdef DEBUG
			// It is very likely that we have not yet started executing the app.
			// Otherwise, inform about the error.
			if (_isAppExecutionStarted)
				OutputDebugString(_T("ERROR GETTING MANAGED CALL STACK!\n"));
#endif

			return callStack;
		}

		for (const ProfilerFrame& profilerFrame : profilerFrames)
		{
			if (profilerFrame.IsManaged)
			{
				callStack.push_back(profilerFrame.Registers.AddrPC);
			}
			else
			{
				firstManagedIP = nullptr;
				firstManagedFuncID = 0;
				nativeCallStack = NativeStackWalker::GetNativeCallStack(
					profilerFrame.Registers,
					nativeBreakFunc
				);

				std::ranges::copy(nativeCallStack, std::back_inserter(callStack));
			}
		}

		return callStack;
	}

	bool CorProfiler::IsFunctionManaged(const LPCBYTE address, FunctionID& managedFuncID) const
	{
		const HRESULT funcResult = _profilerInfo->GetFunctionFromIP(address, &managedFuncID);
		return SUCCEEDED(funcResult) && managedFuncID != 0;
	}

	HRESULT CorProfiler::GetFunctionInfo(
		const FunctionID funcID,
		ClassID& classID,
		ModuleID& moduleID,
		mdToken& token
	) const
	{
		return _profilerInfo->GetFunctionInfo(funcID, &classID, &moduleID, &token);
	}

	HRESULT CorProfiler::GetModulePath(const ModuleID moduleID, CString& modulePath) const
	{
		const auto itPath = _modulePath.find(moduleID);
		if (itPath == _modulePath.end())
		{
			modulePath = "";
			return E_FAIL;
		}

		modulePath = itPath->second;
		return S_OK;
	}

	HRESULT CorProfiler::GetModuleBaseAddress(const ModuleID moduleID, LPCBYTE& baseAddress) const
	{
		const auto itBaseAddress = _moduleBaseAddress.find(moduleID);
		if (itBaseAddress == _moduleBaseAddress.end())
		{
			baseAddress = nullptr;
			return E_FAIL;
		}

		baseAddress = itBaseAddress->second;
		return S_OK;
	}

	HRESULT CorProfiler::GetAnnotation(
		const CString& moduleName,
		const ClassID classID,
		const FunctionID functionID,
		CString& annotation
	)
	{
		annotation = CString();

		if (!moduleName.IsEmpty())
			annotation.AppendFormat(_T("%s!"), moduleName.GetString());

		const auto className = CacheClass(classID);
		if (!className.IsEmpty())
			annotation.AppendFormat(className);

		const auto funcName = CacheFunction(functionID);
		if (!funcName.IsEmpty())
		{
			if (!className.IsEmpty())
				annotation.AppendFormat(_T(".%s"), funcName.GetString());
			else
				annotation.Append(funcName);
		}

		return S_OK;
	}

	HRESULT CorProfiler::GetModuleAssemblyID(const ModuleID moduleID, AssemblyID& assemblyID) const
	{
		const auto itAssemblyID = _moduleAssemblyID.find(moduleID);
		if (itAssemblyID == _moduleAssemblyID.end())
		{
			assemblyID = 0;
			return E_FAIL;
		}

		assemblyID = itAssemblyID->second;
		return S_OK;
	}

	HRESULT CorProfiler::SetEventMask() const
	{
		constexpr DWORD eventMask =
			COR_PRF_MONITOR_JIT_COMPILATION |
			COR_PRF_MONITOR_MODULE_LOADS |
			COR_PRF_MONITOR_THREADS |
			COR_PRF_MONITOR_CLASS_LOADS |
			COR_PRF_ENABLE_STACK_SNAPSHOT;

		return _profilerInfo->SetEventMask(eventMask);
	}

	HRESULT CorProfiler::GetModuleInfo(
		const ModuleID moduleID,
		CString& moduleName,
		LPCBYTE& baseAddress,
		AssemblyID& assemblyID
	) const
	{
		ULONG nameLength = 0;
		WCHAR name[ModuleNameMaxLength];

		const HRESULT result = _profilerInfo->GetModuleInfo(
			moduleID, &baseAddress, ModuleNameMaxLength,
			&nameLength, name, &assemblyID);

		moduleName = CString(name);

		return result;
	}
	
	ThreadID CorProfiler::GetManagedThreadIdFromNative(const DWORD nativeThreadID) const
	{
		const auto managedThreadID = _threadsN2M.find(nativeThreadID);
		return managedThreadID != _threadsN2M.end()
			? managedThreadID->second
			: 0;
	}

	CString CorProfiler::GetFunctionName(const FunctionID funcId) const
	{
		// If the FunctionID is 0, we could be dealing with a native function. 
		if (funcId == 0)
			return CString();

		CString name;

		ClassID classID = 0;
		ModuleID moduleId = 0;
		mdToken token = 0;
		ULONG32 typeArgsLength = 0;
		ClassID typeArgs[TypeArgsMaxLength];
		COR_PRF_FRAME_INFO frameInfo = 0;

		HRESULT hr = S_OK;

		hr = _profilerInfo->GetFunctionInfo2(funcId, frameInfo, &classID, &moduleId, &token,
			TypeArgsMaxLength, &typeArgsLength, typeArgs);
		if (FAILED(hr))
			return CString();

		auto itModuleMDI = _moduleMDI.find(moduleId);
		if (itModuleMDI == _moduleMDI.end())
			return CString();

		auto& mdi = itModuleMDI->second;

		WCHAR methodName[MethodNameMaxLength];
		hr = mdi->GetMethodProps(
			token,
			nullptr,
			methodName,
			MethodNameMaxLength,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
		if (FAILED(hr))
			return CString();

		name.Append(methodName);

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

	CString CorProfiler::GetClassIDName(const ClassID classID) const
	{
		ModuleID modId;
		mdTypeDef classToken;
		ClassID parentClassID;
		ULONG32 typeArgsLength;
		ClassID typeArgs[TypeArgsMaxLength];

		if (classID == 0)
			return CString();

		HRESULT hr = _profilerInfo->GetClassIDInfo2(classID, &modId, &classToken, &parentClassID,
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
			return CString();

		const auto itModuleMDI = _moduleMDI.find(modId);
		if (itModuleMDI == _moduleMDI.end())
			return CString();

		auto& mdi = itModuleMDI->second;

		WCHAR wName[TypeDefMaxLength];
		DWORD dwTypeDefFlags = 0;

		hr = mdi->GetTypeDefProps(classToken, wName, TypeDefMaxLength,
			nullptr, &dwTypeDefFlags, nullptr);
		if (FAILED(hr))
			return CString();

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

	CString CorProfiler::CacheClass(const ClassID classID)
	{
		const auto itFound = _className.find(classID);

		CString className;
		if (itFound != _className.end())
		{
			className = itFound->second;
		}
		else
		{
			className = GetClassIDName(classID);
			if (!className.IsEmpty())
				_className[classID] = className;
		}

		return className;
	}

	CString CorProfiler::CacheFunction(const FunctionID functionID)
	{
		const auto itFound = _functionName.find(functionID);

		CString functionName;
		if (itFound != _functionName.end())
		{
			functionName = itFound->second;
		}
		else
		{
			functionName = GetFunctionName(functionID);
			if (!functionName.IsEmpty())
				_functionName[functionID] = functionName;
		}

		return functionName;
	}

	bool CorProfiler::SaveRuntimeInfo()
	{
		USHORT clrInstanceId;
		COR_PRF_RUNTIME_TYPE runtimeType;
		USHORT majorVersion;
		USHORT minorVersion;
		USHORT buildNumber;
		USHORT qfeVersion;
		ULONG versionStringLength = 0;
		WCHAR versionString[100];

		HRESULT hr = _profilerInfo->GetRuntimeInformation(
			&clrInstanceId,
			&runtimeType,
			&majorVersion,
			&minorVersion,
			&buildNumber,
			&qfeVersion,
			sizeof(versionString) / sizeof(WCHAR),
			&versionStringLength,
			versionString
		);

		if (SUCCEEDED(hr))
		{
			_runtimeType = runtimeType;
			_runtimeVersion = CString(versionString);
			return true;
		}

		_runtimeType = COR_PRF_DESKTOP_CLR;
		_runtimeVersion = CString();
		return false;
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
		const auto profilerFrames = static_cast<std::deque<ProfilerFrame>*>(clientData);
		auto contextData = reinterpret_cast<const CONTEXT*>(context);

		const auto registers = FrameRegisters(
#ifdef _M_IX86
			reinterpret_cast<PVOID>(contextData->Eip),
			reinterpret_cast<PVOID>(contextData->Ebp),
			reinterpret_cast<PVOID>(contextData->Esp)
#elif _M_X64
			reinterpret_cast<PVOID>(contextData->Rip),
			reinterpret_cast<PVOID>(contextData->Rbp),
			reinterpret_cast<PVOID>(contextData->Rsp)
#else
#error "Platform not supported!"
#endif
		);

		const bool isManaged = funcId != 0;

		profilerFrames->push_back(ProfilerFrame(isManaged, registers));

		return S_OK;
	}

}
