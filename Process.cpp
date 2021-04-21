#include "ProcessesManager.h"
#include <string>
#include <iostream>
#include <NTSecAPI.h>

typedef NTSTATUS(NTAPI* pfnNtQueryInformationProcess)(
	IN  HANDLE ProcessHandle,
	IN  int ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN  ULONG ProcessInformationLength,
	OUT PULONG ReturnLength    OPTIONAL
	);


typedef LONG(NTAPI* NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI* NtResumeProcess)(IN HANDLE ProcessHandle);

void Process::staticInitProcessState() {
	this->setProcessSecurityState();
	this->setProcessUserName();
	this->setProcessImageState();
}

Process::Process(PID processId,ProcessesManager* processesManager) {
	this->thisProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE | PROCESS_SUSPEND_RESUME
		, FALSE, processId);

	if (this->thisProcess == INVALID_HANDLE_VALUE) {
		//fails to get process handle
	}

	if (!OpenProcessToken(this->thisProcess, TOKEN_QUERY, &this->processToken)) {
		//fails to get process token handle
	}

	//this->userName = NULL;
	//this->processName = new TCHAR[MAX_PROCESS_NAME_LENGTH];
	//this->staticInitProcessState();

	this->processId = processId;
	this->processesManager = processesManager;
}

Process::Process(PID processId) {
	this->thisProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE | PROCESS_SUSPEND_RESUME
		, FALSE, processId);
	if (this->thisProcess == 0) {
		//fails to get process handle
		this->thisProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	}

	if (!OpenProcessToken(this->thisProcess, TOKEN_QUERY, &this->processToken)) {
		//fails to get process token handle
	}

	this->processId = processId;
	this->processName = processName;
	HMODULE hNtDll = LoadLibrary(_T("ntdll.dll"));
	

	
	//this->userName = NULL;
	//this->processName = new TCHAR[MAX_PROCESS_NAME_LENGTH];
	//this->staticInitProcessState();

	//this->processesManager = processesManager;
}

Process::~Process() {
	CloseHandle(this->thisProcess);
	
	if (memoryState != nullptr) 
		delete this->memoryState;
	
	if (ioState != nullptr) 
		delete this->ioState;

	if (cpuState != nullptr)
		delete this->cpuState;

	if (securityState != nullptr)
		delete this->securityState;

	if (imageState != nullptr)
		delete this->imageState;

	if (handleState != nullptr)
		delete handleState;


}


DWORD Process::killProcess() {
	if (!TerminateProcess(this->thisProcess, 0)) {
		return GetLastError();
	}
	return 0;
}

DWORD Process::updateInfo() {
	this->setProcessMemoryState();
	this->setProcessCPUState();
	return 0;
}

BOOL Process::getAllThreads() {
	this->threads = this->processesManager->getThreadsByPID(this->processId);
	return TRUE;
}

LSA_HANDLE GetPolicyHandle() {
	LSA_OBJECT_ATTRIBUTES ObjectAttributes;
	TCHAR* SystemName = NULL;
	USHORT SystemNameLength;
	LSA_UNICODE_STRING lusSystemName;
	NTSTATUS ntsResult;
	LSA_HANDLE lsahPolicyHandle;

	ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

	ntsResult = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_LOOKUP_NAMES, &lsahPolicyHandle);
	
	if (ntsResult != CMC_STATUS_SUCCESS) {
		_tprintf(_T("OpenPolicy returned %lu"), LsaNtStatusToWinError(ntsResult));
		return NULL;
	}

	return lsahPolicyHandle;
}

DWORD Process::suspendProcess() {
	NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(
		GetModuleHandle(L"ntdll"), "NtSuspendProcess");

	pfnNtSuspendProcess(this->thisProcess);
	return 0;
}

DWORD Process::resumeProcess() {
	NtSuspendProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(
		GetModuleHandle(L"ntdll"), "NtResumeProcess");
	pfnNtResumeProcess(this->thisProcess);
	return 0;
}
DWORD Process::setProcessUserName() {
	DWORD res = 0;
	LSA_HANDLE policyHandle;
	policyHandle = GetPolicyHandle();
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, 12052);
	HANDLE processToken;
	OpenProcessToken(hProcess, TOKEN_QUERY, &processToken);
	TOKEN_USER tokenUser1;
	DWORD _;
	GetTokenInformation(processToken, TokenUser, &tokenUser1, 0, &_);
	GetTokenInformation(processToken, TokenUser, &tokenUser1, _, &_);
	if (policyHandle == NULL) {
		//FAILS TO GET POLICY HANDLE
	}

	if (processToken == NULL) {
		return NULL;
	}
	GetTokenInformation(processToken, TokenUser, &tokenUser1, sizeof(tokenUser1), &_);

	GetTokenInformation(processToken, TokenUser, &tokenUser1, _, &_);

	PSID sids[1];
	sids[0] = tokenUser1.User.Sid;
	PLSA_TRANSLATED_NAME names = NULL;
	PLSA_REFERENCED_DOMAIN_LIST referencedNames = NULL;
	LPTSTR Name;
	LPTSTR userName = NULL;
	if (LsaLookupSids(policyHandle, 1, &tokenUser1.User.Sid, &referencedNames, &names) >= 0) {
		if (names[0].Use != SidTypeInvalid && names[0].Use != SidTypeUnknown) {
			if (userName == NULL) {
				this->userName = names[0].Name.Buffer;
			}
		}
	}
	else {
		res = GetLastError();
	}


	if (names) {
		LsaFreeMemory(names);
	}

	if (referencedNames) {
		LsaFreeMemory(referencedNames);
	}

	return 1;
}

DWORD Process::setProcessSecurityState() {
	if (this->securityState == nullptr) {
		//if fail to create security State
	}
	DWORD dwTokenGroup;

	if (!GetTokenInformation(processToken, TokenGroups,&this->securityState->Groups , sizeof(TOKEN_GROUPS), &dwTokenGroup)) {
		
	}

	DWORD dwTokenSessionId;
	if (!GetTokenInformation(processToken, TokenSessionId, &this->securityState->Session, sizeof(DWORD), &dwTokenSessionId)) {

	}

	DWORD dwImpersonationLevel;
	if (!GetTokenInformation(processToken, TokenImpersonationLevel, &this->securityState->impersonationLevel,
		sizeof(SECURITY_IMPERSONATION_LEVEL), &dwImpersonationLevel)) {

	}

	return 0;
}

void Process::setPriority(Priority priority) {
	if (!SetPriorityClass(this->thisProcess, priority)) {

	}
}

Priority Process::getPriority() {
	return GetPriorityClass(this->thisProcess);
}

Affinity Process::getAffinity() {
	DWORD_PTR affinity;
	if (!GetProcessAffinityMask(this->thisProcess,&affinity,NULL)) {

	}

	return affinity;
}

void Process::setAffinity(Affinity affinity) {
	//if (!SetProcessAffinityMask(this->thisProcess, (DWORD)&this->affinity)) {
		//when can't set process affinity mask
	//}
}


DWORD Process::setProcessCPUState() {
	DWORD res = 0;
	if (this->cpuState == NULL) {
		//error when alloc CPUState struct
	}

	if (!GetProcessTimes(this->thisProcess, &cpuState->createTime, &cpuState->exitTime, &cpuState->kernelTime, &cpuState->userTime)) {
		//error when get Processes times
		res = GetLastError();
	}

	return res;
}

DWORD Process::setProcessImageState() {
	DWORD res = 0;
	if (imageState == nullptr) {
		//error when alloc ImageState struct
	}
	//Get cmdline of process

	//Get path
	if (!GetProcessImageFileName(thisProcess, this->imageState->imageFileName, MAX_FILE_LENGTH)) {
		res = GetLastError();
	}
	
	return res;

}

DWORD Process::setProcessMemoryState() {
	ZeroMemory(&this->pMemoryCounters, sizeof(this->pMemoryCounters));
	if (GetProcessMemoryInfo(this->thisProcess, (PPROCESS_MEMORY_COUNTERS)&this->pMemoryCounters, sizeof(this->pMemoryCounters))) {
		memoryState->workingSet = this->pMemoryCounters.WorkingSetSize;
		memoryState->privateBytes = this->pMemoryCounters.PrivateUsage;
		memoryState->pageFile = this->pMemoryCounters.PagefileUsage;
		memoryState->peakWorkingSet = this->pMemoryCounters.PeakWorkingSetSize;
	}
	else {
		return GetLastError();
	}

	return 0;
}

DWORD Process::setProcessIOState() {
	ZeroMemory(this->ioState, sizeof(IOState));
	if(!GetProcessIoCounters(this->thisProcess, &this->ioCounters)) {
		return GetLastError();
	}

	ioState->reads = ioCounters.ReadTransferCount;
	ioState->writes = ioCounters.WriteTransferCount;
	ioState->other = ioCounters.OtherTransferCount;
}

DWORD Process::setProcessHandleState() {
	return 0;
}

BOOL Process::createDump(LPTSTR filename,MINIDUMP_TYPE dumpType) {
	HANDLE dumpFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dumpFile == INVALID_HANDLE_VALUE) {
		//fails to create a file handle
		return FALSE;
	}

	if (!MiniDumpWriteDump(this->thisProcess, this->processId, dumpFile, dumpType, NULL, NULL, NULL)) {
		//fails to dump
		return FALSE;
	}

	CloseHandle(dumpFile);
	return TRUE;
}

PID Process::getPID() {
	return processId;
}

MPEString Process::getUserName() {
	this->setProcessUserName();
	return this->userName;
}
SecurityState* Process::getSecurityState() {
	this->setProcessSecurityState();
	return this->securityState;
}
ImageState* Process::getImageState() {
	this->setProcessImageState();
	return this->imageState;
}
HandleState* Process::getHandleState() {
	this->setProcessHandleState();
	return this->handleState;
}
IOState* Process::getIOState() {
	this->setProcessIOState();
	return this->ioState;
}
MemoryState* Process::getMemoryState() {
	this->setProcessMemoryState();
	return this->memoryState;
}
CPUState* Process::getCPUState() {
	this->setProcessCPUState();
	return this->cpuState;
}