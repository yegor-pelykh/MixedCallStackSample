#pragma once
#include "FrameRegisters.hpp"

namespace MixedCallStackSampleClient
{
	class CorProfiler final : public ICorProfilerCallback9
	{
	public:
		CorProfiler();
		virtual ~CorProfiler();

	public:
		HRESULT STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) override;
		ULONG STDMETHODCALLTYPE AddRef() override;
		ULONG STDMETHODCALLTYPE Release() override;
		HRESULT STDMETHODCALLTYPE Initialize(IUnknown* pICorProfilerInfoUnk) override;
		HRESULT STDMETHODCALLTYPE Shutdown() override;
		HRESULT STDMETHODCALLTYPE AppDomainCreationStarted(AppDomainID appDomainId) override;
		HRESULT STDMETHODCALLTYPE AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus) override;
		HRESULT STDMETHODCALLTYPE AppDomainShutdownStarted(AppDomainID appDomainId) override;
		HRESULT STDMETHODCALLTYPE AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus) override;
		HRESULT STDMETHODCALLTYPE AssemblyLoadStarted(AssemblyID assemblyId) override;
		HRESULT STDMETHODCALLTYPE AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;
		HRESULT STDMETHODCALLTYPE AssemblyUnloadStarted(AssemblyID assemblyId) override;
		HRESULT STDMETHODCALLTYPE AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;
		HRESULT STDMETHODCALLTYPE ModuleLoadStarted(ModuleID moduleId) override;
		HRESULT STDMETHODCALLTYPE ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus) override;
		HRESULT STDMETHODCALLTYPE ModuleUnloadStarted(ModuleID moduleId) override;
		HRESULT STDMETHODCALLTYPE ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus) override;
		HRESULT STDMETHODCALLTYPE ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId) override;
		HRESULT STDMETHODCALLTYPE ClassLoadStarted(ClassID classID) override;
		HRESULT STDMETHODCALLTYPE ClassLoadFinished(ClassID classID, HRESULT hrStatus) override;
		HRESULT STDMETHODCALLTYPE ClassUnloadStarted(ClassID classID) override;
		HRESULT STDMETHODCALLTYPE ClassUnloadFinished(ClassID classID, HRESULT hrStatus) override;
		HRESULT STDMETHODCALLTYPE FunctionUnloadStarted(FunctionID functionID) override;
		HRESULT STDMETHODCALLTYPE JITCompilationStarted(FunctionID functionID, BOOL fIsSafeToBlock) override;
		HRESULT STDMETHODCALLTYPE JITCompilationFinished(FunctionID functionID, HRESULT hrStatus, BOOL fIsSafeToBlock) override;
		HRESULT STDMETHODCALLTYPE JITCachedFunctionSearchStarted(FunctionID functionID, BOOL* pbUseCachedFunction) override;
		HRESULT STDMETHODCALLTYPE JITCachedFunctionSearchFinished(FunctionID functionID, COR_PRF_JIT_CACHE result) override;
		HRESULT STDMETHODCALLTYPE JITFunctionPitched(FunctionID functionID) override;
		HRESULT STDMETHODCALLTYPE JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline) override;
		HRESULT STDMETHODCALLTYPE ThreadCreated(ThreadID threadId) override;
		HRESULT STDMETHODCALLTYPE ThreadDestroyed(ThreadID threadId) override;
		HRESULT STDMETHODCALLTYPE ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId) override;
		HRESULT STDMETHODCALLTYPE RemotingClientInvocationStarted() override;
		HRESULT STDMETHODCALLTYPE RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync) override;
		HRESULT STDMETHODCALLTYPE RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync) override;
		HRESULT STDMETHODCALLTYPE RemotingClientInvocationFinished() override;
		HRESULT STDMETHODCALLTYPE RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync) override;
		HRESULT STDMETHODCALLTYPE RemotingServerInvocationStarted() override;
		HRESULT STDMETHODCALLTYPE RemotingServerInvocationReturned() override;
		HRESULT STDMETHODCALLTYPE RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync) override;
		HRESULT STDMETHODCALLTYPE UnmanagedToManagedTransition(FunctionID functionID, COR_PRF_TRANSITION_REASON reason) override;
		HRESULT STDMETHODCALLTYPE ManagedToUnmanagedTransition(FunctionID functionID, COR_PRF_TRANSITION_REASON reason) override;
		HRESULT STDMETHODCALLTYPE RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason) override;
		HRESULT STDMETHODCALLTYPE RuntimeSuspendFinished() override;
		HRESULT STDMETHODCALLTYPE RuntimeSuspendAborted() override;
		HRESULT STDMETHODCALLTYPE RuntimeResumeStarted() override;
		HRESULT STDMETHODCALLTYPE RuntimeResumeFinished() override;
		HRESULT STDMETHODCALLTYPE RuntimeThreadSuspended(ThreadID threadId) override;
		HRESULT STDMETHODCALLTYPE RuntimeThreadResumed(ThreadID threadId) override;
		HRESULT STDMETHODCALLTYPE MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[],
			ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[]) override;
		HRESULT STDMETHODCALLTYPE ObjectAllocated(ObjectID objectId, ClassID classID) override;
		HRESULT STDMETHODCALLTYPE ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[]) override;
		HRESULT STDMETHODCALLTYPE ObjectReferences(ObjectID objectId, ClassID classID, ULONG cObjectRefs,
			ObjectID objectRefIds[]) override;
		HRESULT STDMETHODCALLTYPE RootReferences(ULONG cRootRefs, ObjectID rootRefIds[]) override;
		HRESULT STDMETHODCALLTYPE ExceptionThrown(ObjectID thrownObjectId) override;
		HRESULT STDMETHODCALLTYPE ExceptionSearchFunctionEnter(FunctionID functionID) override;
		HRESULT STDMETHODCALLTYPE ExceptionSearchFunctionLeave() override;
		HRESULT STDMETHODCALLTYPE ExceptionSearchFilterEnter(FunctionID functionID) override;
		HRESULT STDMETHODCALLTYPE ExceptionSearchFilterLeave() override;
		HRESULT STDMETHODCALLTYPE ExceptionSearchCatcherFound(FunctionID functionID) override;
		HRESULT STDMETHODCALLTYPE ExceptionOSHandlerEnter(UINT_PTR __unused) override;
		HRESULT STDMETHODCALLTYPE ExceptionOSHandlerLeave(UINT_PTR __unused) override;
		HRESULT STDMETHODCALLTYPE ExceptionUnwindFunctionEnter(FunctionID functionID) override;
		HRESULT STDMETHODCALLTYPE ExceptionUnwindFunctionLeave() override;
		HRESULT STDMETHODCALLTYPE ExceptionUnwindFinallyEnter(FunctionID functionID) override;
		HRESULT STDMETHODCALLTYPE ExceptionUnwindFinallyLeave() override;
		HRESULT STDMETHODCALLTYPE ExceptionCatcherEnter(FunctionID functionID, ObjectID objectId) override;
		HRESULT STDMETHODCALLTYPE ExceptionCatcherLeave() override;
		HRESULT STDMETHODCALLTYPE COMClassicVTableCreated(ClassID wrappedClassId, const GUID& implementedIID, void* pVTable,
			ULONG cSlots) override;
		HRESULT STDMETHODCALLTYPE COMClassicVTableDestroyed(ClassID wrappedClassId, const GUID& implementedIID, void* pVTable) override;
		HRESULT STDMETHODCALLTYPE ExceptionCLRCatcherFound() override;
		HRESULT STDMETHODCALLTYPE ExceptionCLRCatcherExecute() override;
		HRESULT STDMETHODCALLTYPE ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[]) override;
		HRESULT STDMETHODCALLTYPE GarbageCollectionStarted(int cGenerations, BOOL generationCollected[],
			COR_PRF_GC_REASON reason) override;
		HRESULT STDMETHODCALLTYPE SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[],
			ULONG cObjectIDRangeLength[]) override;
		HRESULT STDMETHODCALLTYPE GarbageCollectionFinished() override;
		HRESULT STDMETHODCALLTYPE FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID) override;
		HRESULT STDMETHODCALLTYPE RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[],
			COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[]) override;
		HRESULT STDMETHODCALLTYPE HandleCreated(GCHandleID handleId, ObjectID initialObjectId) override;
		HRESULT STDMETHODCALLTYPE HandleDestroyed(GCHandleID handleId) override;
		HRESULT STDMETHODCALLTYPE InitializeForAttach(IUnknown* pCorProfilerInfoUnk, void* pvClientData, UINT cbClientData) override;
		HRESULT STDMETHODCALLTYPE ProfilerAttachComplete() override;
		HRESULT STDMETHODCALLTYPE ProfilerDetachSucceeded() override;
		HRESULT STDMETHODCALLTYPE ReJITCompilationStarted(FunctionID functionID, ReJITID rejitId, BOOL fIsSafeToBlock) override;
		HRESULT STDMETHODCALLTYPE GetReJITParameters(ModuleID moduleId, mdMethodDef methodId,
			ICorProfilerFunctionControl* pFunctionControl) override;
		HRESULT STDMETHODCALLTYPE ReJITCompilationFinished(FunctionID functionID, ReJITID rejitId, HRESULT hrStatus,
			BOOL fIsSafeToBlock) override;
		HRESULT STDMETHODCALLTYPE ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionID,
			HRESULT hrStatus) override;
		HRESULT STDMETHODCALLTYPE MovedReferences2(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[],
			ObjectID newObjectIDRangeStart[], SIZE_T cObjectIDRangeLength[]) override;
		HRESULT STDMETHODCALLTYPE SurvivingReferences2(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[],
			SIZE_T cObjectIDRangeLength[]) override;
		HRESULT STDMETHODCALLTYPE ConditionalWeakTableElementReferences(ULONG cRootRefs, ObjectID keyRefIds[],
			ObjectID valueRefIds[], GCHandleID rootIds[]) override;
		HRESULT STDMETHODCALLTYPE GetAssemblyReferences(const WCHAR* wszAssemblyPath,
			ICorProfilerAssemblyReferenceProvider* pAsmRefProvider) override;
		HRESULT STDMETHODCALLTYPE ModuleInMemorySymbolsUpdated(ModuleID moduleId) override;
		HRESULT STDMETHODCALLTYPE DynamicMethodJITCompilationStarted(FunctionID functionID, BOOL fIsSafeToBlock,
			LPCBYTE pILHeader, ULONG cbILHeader) override;
		HRESULT STDMETHODCALLTYPE DynamicMethodJITCompilationFinished(FunctionID functionID, HRESULT hrStatus,
			BOOL fIsSafeToBlock) override;
		HRESULT STDMETHODCALLTYPE DynamicMethodUnloaded(FunctionID functionID) override;

	public:
		std::deque<PVOID> GetMixedCallStack(const FrameRegisters& registers) const;
		bool IsFunctionManaged(const LPCBYTE address, FunctionID& managedFuncID) const;
		HRESULT GetFunctionInfo(
			const FunctionID funcID,
			ClassID& classID,
			ModuleID& moduleID,
			mdToken& token
		) const;
		HRESULT GetModulePath(const ModuleID moduleID, CString& modulePath) const;
		HRESULT GetModuleBaseAddress(const ModuleID moduleID, LPCBYTE& baseAddress) const;
		HRESULT GetModuleAssemblyID(const ModuleID moduleID, AssemblyID& assemblyID) const;
		HRESULT GetAnnotation(
			const CString& moduleName,
			const ClassID classID,
			const FunctionID functionID,
			CString& annotation
		);

	private:
		HRESULT SetEventMask() const;
		HRESULT GetModuleInfo(
			const ModuleID moduleID,
			CString& moduleName,
			LPCBYTE& baseAddress,
			AssemblyID& assemblyID
		) const;
		ThreadID GetManagedThreadIdFromNative(DWORD nativeThreadId) const;
		CString GetFunctionName(FunctionID funcId) const;
		CString GetClassIDName(ClassID classID) const;
		CString CacheClass(ClassID classID);
		CString CacheFunction(FunctionID functionID);
		bool SaveRuntimeInfo();
		/* bool LoadFrameworkLibraries(); */

	private:
		static HRESULT __stdcall DoStackSnapshotCallback(FunctionID funcId,
			UINT_PTR ip, COR_PRF_FRAME_INFO frameInfo, ULONG32 contextSize,
			BYTE context[], void* clientData);

	private:
		std::atomic<long> _refCount;
		CComPtr<ICorProfilerInfo8> _profilerInfo;
		bool _isAppExecutionStarted;
		std::map<ModuleID, CString> _modulePath;
		std::map<ModuleID, LPCBYTE> _moduleBaseAddress;
		std::map<ModuleID, AssemblyID> _moduleAssemblyID;
		std::map<ModuleID, CComPtr<IMetaDataImport>> _moduleMDI;
		std::map<ClassID, CString> _className;
		std::map<FunctionID, CString> _functionName;
		std::list<ThreadID> _threadsManaged;
		std::map<ThreadID, DWORD> _threadsM2N;
		std::map<DWORD, ThreadID> _threadsN2M;
		COR_PRF_RUNTIME_TYPE _runtimeType;
		CString _runtimeVersion;

	};

}
